/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "multitransactioneditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractItemView>
#include <QCompleter>
#include <QDebug>
#include <QGlobalStatic>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringList>
#include <QTableView>
#include <QTimer>
#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountcreator.h"
#include "accountsmodel.h"
#include "costcentermodel.h"
#include "icons.h"
#include "idfilter.h"
#include "journalmodel.h"
#include "kcurrencycalculator.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneysettings.h"
#include "knewaccountdlg.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include "payeecreator.h"
#include "payeesmodel.h"
#include "securitiesmodel.h"
#include "splitdialog.h"
#include "statusmodel.h"
#include "tagsmodel.h"
#include "widgethintframe.h"

#include "ui_newtransactioneditor.h"

using namespace Icons;

class MultiTransactionEditor::Private
{
public:
    enum TaxValueChange {
        ValueUnchanged,
        ValueChanged,
    };
    Private(MultiTransactionEditor* parent)
        : q(parent)
        , ui(new Ui_NewTransactionEditor)
        , tabOrderUi(nullptr)
        , accountsModel(new AccountNamesFilterProxyModel(parent))
        , categoriesModel(new AccountNamesFilterProxyModel(parent))
        , costCenterModel(new QSortFilterProxyModel(parent))
        , payeesModel(new QSortFilterProxyModel(parent))
        , accepted(false)
        , frameCollection(nullptr)
    {
        accountsModel->setObjectName(QLatin1String("MultiTransactionEditor::accountsModel"));
        categoriesModel->setObjectName(QLatin1String("MultiTransactionEditor::categoriesModel"));
        costCenterModel->setObjectName(QLatin1String("SortedCostCenterModel"));
        payeesModel->setObjectName(QLatin1String("SortedPayeesModel"));

        costCenterModel->setSortLocaleAware(true);
        costCenterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        payeesModel->setSortLocaleAware(true);
        payeesModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    }

    ~Private()
    {
        delete ui;
    }

    void setupTabOrder();
    bool anyChanges() const;
    bool isDatePostOpeningDate(const QDate& date, const QString& accountId);
    bool postdateChanged(const QDate& date);
    void costCenterChanged(int costCenterIndex);
    void payeeChanged(int payeeIndex);
    void accountChanged(const QString& id);
    bool categoryChanged(const QString& id);
    void numberChanged(const QString& newNumber);
    void amountChanged();
    bool isIncomeExpense(const QModelIndex& idx) const;
    bool isIncomeExpense(const QString& categoryId) const;
    void tagsChanged(const QStringList& tagIds);
    void createCategory();
    void createPayee();

    MultiTransactionEditor* q;
    Ui_NewTransactionEditor* ui;
    Ui_NewTransactionEditor* tabOrderUi;
    AccountNamesFilterProxyModel* accountsModel;
    AccountNamesFilterProxyModel* categoriesModel;
    QSortFilterProxyModel* costCenterModel;
    QSortFilterProxyModel* payeesModel;
    bool accepted;
    QUndoStack undoStack;
    WidgetHintFrameCollection* frameCollection;
    QString errorMessage;
    StatusModel statusModel;
    QStringList selectedJournalEntryIds;
};

bool MultiTransactionEditor::Private::anyChanges() const
{
    bool rc = false;
    rc |= !ui->dateEdit->isNull();
    rc |= ui->creditDebitEdit->haveValue();
    rc |= !ui->payeeEdit->lineEdit()->text().isEmpty();
    rc |= !ui->categoryCombo->getSelected().isEmpty();
    rc |= !ui->costCenterCombo->lineEdit()->text().isEmpty();
    rc |= !ui->tagContainer->selectedTags().isEmpty();
    rc |= !ui->statusCombo->currentText().isEmpty();
    rc |= !ui->memoEdit->toPlainText().isEmpty();
    return rc;
}

bool MultiTransactionEditor::Private::isDatePostOpeningDate(const QDate& date, const QString& accountId)
{
    bool rc = true;

    try {
        MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
        const bool isIncomeExpense = account.isIncomeExpense();

        // we don't check for categories
        if (!isIncomeExpense) {
            if (date < account.openingDate())
                rc = false;
        }
    } catch (MyMoneyException&) {
        qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
    }
    return rc;
}

bool MultiTransactionEditor::Private::postdateChanged(const QDate& date)
{
    WidgetHintFrame::hide(ui->dateEdit, i18n("The posting date of the transaction."));

    // if the date field is empty, we have a valid date
    if (ui->dateEdit->isNull()) {
        return true;
    }

    if (!date.isValid()) {
        WidgetHintFrame::show(ui->dateEdit, i18n("The posting date is invalid."));
        return false;
    }

    // collect all account ids of all selected transactions
    QStringList accountIds;
    if (!ui->categoryCombo->getSelected().isEmpty()) {
        accountIds << ui->categoryCombo->getSelected();
    }
    const auto journalModel = MyMoneyFile::instance()->journalModel();
    for (const auto& journalEntryId : selectedJournalEntryIds) {
        const auto journalEntry = journalModel->itemById(journalEntryId);
        const auto splits = journalEntry.transaction().splits();
        for (const auto& sp : splits) {
            accountIds << sp.accountId();
        }
    }

    bool rc = true;
    for (const auto& accountId : accountIds) {
        if (!isDatePostOpeningDate(date, accountId)) {
            MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
            WidgetHintFrame::show(ui->dateEdit, i18n("The posting date is prior to the opening date of account <b>%1</b>.", account.name()));
            rc = false;
            break;
        }
    }
    return rc;
}

void MultiTransactionEditor::Private::costCenterChanged(int costCenterIndex)
{
    Q_UNUSED(costCenterIndex)
}

bool MultiTransactionEditor::Private::isIncomeExpense(const QString& categoryId) const
{
    if (!categoryId.isEmpty()) {
        MyMoneyAccount category = MyMoneyFile::instance()->account(categoryId);
        return category.isIncomeExpense();
    }
    return false;
}

bool MultiTransactionEditor::Private::categoryChanged(const QString& accountId)
{
    WidgetHintFrame::hide(ui->categoryCombo, i18n("Select category or account."));
    bool rc = true;
    if (!accountId.isEmpty()) {
        try {
            MyMoneyAccount category = MyMoneyFile::instance()->account(accountId);
            if (category.isAssetLiability()) {
                const auto journalModel = MyMoneyFile::instance()->journalModel();
                for (const auto& journalEntryId : selectedJournalEntryIds) {
                    const auto journalEntry = journalModel->itemById(journalEntryId);
                    const auto postDate = journalEntry.transaction().postDate();
                    if (postDate < category.openingDate()) {
                        QString msg = i18nc("@info Error when selecting account",
                                            "A selected transaction has a post date (%1) before the opening date of the selected account (%2).",
                                            MyMoneyUtils::formatDate(postDate),
                                            MyMoneyUtils::formatDate(category.openingDate()));
                        WidgetHintFrame::show(ui->categoryCombo, msg);
                        rc = false;
                    }
                }
            }
        } catch (MyMoneyException&) {
            qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
        }
    }
    return rc;
}

void MultiTransactionEditor::Private::numberChanged(const QString& newNumber)
{
    Q_UNUSED(newNumber)
}

void MultiTransactionEditor::Private::amountChanged()
{
}

void MultiTransactionEditor::Private::payeeChanged(int payeeIndex)
{
    Q_UNUSED(payeeIndex)
}

void MultiTransactionEditor::Private::tagsChanged(const QStringList& tagIds)
{
    Q_UNUSED(tagIds)
}

void MultiTransactionEditor::Private::setupTabOrder()
{
    const auto defaultTabOrder = QStringList{
        QLatin1String("accountCombo"),
        QLatin1String("dateEdit"),
        QLatin1String("creditDebitEdit"),
        QLatin1String("payeeEdit"),
        QLatin1String("numberEdit"),
        QLatin1String("categoryCombo"),
        QLatin1String("costCenterCombo"),
        QLatin1String("tagContainer"),
        QLatin1String("statusCombo"),
        QLatin1String("memoEdit"),
        QLatin1String("enterButton"),
        QLatin1String("cancelButton"),
    };
    q->setProperty("kmm_defaulttaborder", defaultTabOrder);
    q->setProperty("kmm_currenttaborder", q->tabOrder(QLatin1String("multiTransactionEditor"), defaultTabOrder));

    q->setupTabOrder(q->property("kmm_currenttaborder").toStringList());
}

void MultiTransactionEditor::Private::createCategory()
{
    auto creator = new AccountCreator(q);
    creator->setComboBox(ui->categoryCombo);
    creator->addButton(ui->cancelButton);
    creator->addButton(ui->enterButton);
    creator->setAccountType(eMyMoney::Account::Type::Expense);
    if (ui->creditDebitEdit->haveValue() && ui->creditDebitEdit->value().isPositive()) {
        creator->setAccountType(eMyMoney::Account::Type::Income);
    }
    creator->createAccount();
}

void MultiTransactionEditor::Private::createPayee()
{
    auto creator = new PayeeCreator(q);
    creator->setComboBox(ui->payeeEdit);
    creator->addButton(ui->cancelButton);
    creator->addButton(ui->enterButton);
    creator->createPayee();
}

MultiTransactionEditor::MultiTransactionEditor(QWidget* parent, const QString& accountId)
    : TransactionEditorBase(parent, accountId)
    , d(new Private(this))
{
    auto const file = MyMoneyFile::instance();
    auto const model = file->accountsModel();

    d->ui->setupUi(this);

    // default is to hide the account selection combobox
    // and the number widget
    setShowAccountCombo(false);
    setShowNumberWidget(false);

    d->setupTabOrder();

    // determine order of credit and debit edit widgets
    // based on their visual order in the ledger
    int creditColumn = JournalModel::Column::Payment;
    int debitColumn = JournalModel::Column::Deposit;

    QWidget* w(this);
    do {
        w = w->parentWidget();
        const auto view = qobject_cast<const QTableView*>(w);
        if (view) {
            creditColumn = view->horizontalHeader()->visualIndex(creditColumn);
            debitColumn = view->horizontalHeader()->visualIndex(debitColumn);
            break;
        }
    } while (w);

    // in case they are in the opposite order, we swap the edit widgets
    if (debitColumn < creditColumn) {
        d->ui->creditDebitEdit->swapCreditDebit();
    }

    d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type>{
        eMyMoney::Account::Type::Asset,
        eMyMoney::Account::Type::Liability,
        eMyMoney::Account::Type::Equity,
    });
    d->accountsModel->setHideEquityAccounts(false);
    d->accountsModel->setHideZeroBalancedEquityAccounts(false);
    d->accountsModel->setSourceModel(model);
    d->accountsModel->sort(AccountsModel::Column::AccountName);
    d->ui->accountCombo->setModel(d->accountsModel);

    d->categoriesModel->addAccountGroup(QVector<eMyMoney::Account::Type>{
        eMyMoney::Account::Type::Asset,
        eMyMoney::Account::Type::Liability,
        eMyMoney::Account::Type::Income,
        eMyMoney::Account::Type::Expense,
        eMyMoney::Account::Type::Equity,
    });
    d->categoriesModel->setHideEquityAccounts(false);
    d->categoriesModel->setSourceModel(model);
    d->categoriesModel->sort(AccountsModel::Column::AccountName);
    d->ui->categoryCombo->setModel(d->categoriesModel);

    d->ui->tagContainer->setModel(file->tagsModel()->modelWithEmptyItem());

    d->costCenterModel->setSortRole(Qt::DisplayRole);
    d->costCenterModel->setSourceModel(file->costCenterModel()->modelWithEmptyItem());
    d->costCenterModel->setSortLocaleAware(true);
    d->costCenterModel->sort(0);

    d->ui->costCenterCombo->setEditable(true);
    d->ui->costCenterCombo->setModel(d->costCenterModel);
    d->ui->costCenterCombo->setModelColumn(0);
    d->ui->costCenterCombo->completer()->setFilterMode(Qt::MatchContains);

    d->payeesModel->setSortRole(Qt::DisplayRole);
    d->payeesModel->setSourceModel(file->payeesModel()->modelWithEmptyItem());
    d->payeesModel->setSortLocaleAware(true);
    d->payeesModel->sort(0);

    d->ui->payeeEdit->setEditable(true);
    d->ui->payeeEdit->lineEdit()->setClearButtonEnabled(true);
    d->ui->payeeEdit->setModel(d->payeesModel);
    d->ui->payeeEdit->setModelColumn(0);
    d->ui->payeeEdit->completer()->setCompletionMode(QCompleter::PopupCompletion);
    d->ui->payeeEdit->completer()->setFilterMode(Qt::MatchContains);

    // make sure that there is no selection left in the background
    // in case there is no text in the edit field
    connect(d->ui->payeeEdit->lineEdit(), &QLineEdit::textEdited, [&](const QString& txt) {
        if (txt.isEmpty()) {
            d->ui->payeeEdit->setCurrentIndex(-1);
        }
    });

    connect(d->ui->categoryCombo->lineEdit(), &QLineEdit::textEdited, [&](const QString& txt) {
        if (txt.isEmpty()) {
            d->ui->categoryCombo->setSelected(QString());
        }
    });
    d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
    d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

    // construct a special status model that supports the unchanged (unknown) entry
    QMap<QString, StatusEntry> states = {
        {QStringLiteral("ST00"), StatusEntry(QString(), eMyMoney::Split::State::Unknown, QString(), QString())},
    };
    const auto rows = d->statusModel.rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = d->statusModel.index(row, 0);
        const auto statusEntry = d->statusModel.itemByIndex(idx);
        states.insert(statusEntry.id(), statusEntry);
    }
    d->statusModel.load(states);

    d->ui->statusCombo->setModel(&d->statusModel);

    d->ui->creditDebitEdit->setAllowEmpty(true);

    d->frameCollection = new WidgetHintFrameCollection(this);
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->dateEdit));
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->categoryCombo));
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
    d->frameCollection->addWidget(d->ui->enterButton);

    connect(d->ui->numberEdit, &QLineEdit::textChanged, this, [&](const QString& newNumber) {
        d->numberChanged(newNumber);
    });

    connect(d->ui->costCenterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int costCenterIndex) {
        d->costCenterChanged(costCenterIndex);
    });

    connect(d->ui->categoryCombo, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& id) {
        d->categoryChanged(id);
    });

    connect(d->ui->dateEdit, &KMyMoneyDateEdit::dateValidityChanged, this, [&](const QDate& date) {
        d->postdateChanged(date);
    });

    connect(d->ui->dateEdit, &KMyMoneyDateEdit::dateEntered, this, [&](const QDate& date) {
        d->postdateChanged(date);
        Q_EMIT postDateChanged(date);
    });

    connect(d->ui->creditDebitEdit, &CreditDebitEdit::amountChanged, this, [&]() {
        d->amountChanged();
    });

    connect(d->ui->payeeEdit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int payeeIndex) {
        d->payeeChanged(payeeIndex);
    });

    connect(d->ui->tagContainer, &KTagContainer::tagsChanged, this, [&](const QStringList& tagIds) {
        d->tagsChanged(tagIds);
    });

    connect(d->ui->cancelButton, &QToolButton::clicked, this, &MultiTransactionEditor::reject);
    connect(d->ui->enterButton, &QToolButton::clicked, this, &MultiTransactionEditor::acceptEdit);

    // handle some events in certain conditions different from default
    d->ui->payeeEdit->installEventFilter(this);
    d->ui->costCenterCombo->installEventFilter(this);
    d->ui->tagContainer->tagCombo()->installEventFilter(this);
    d->ui->categoryCombo->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);

    setCancelButton(d->ui->cancelButton);
    setEnterButton(d->ui->enterButton);

    // force setup of filters
    slotSettingsChanged();

    // setup widget content
    d->ui->dateEdit->setAllowEmptyDate(true);
    d->ui->dateEdit->clearEditText();

    d->ui->creditDebitEdit->setAllowEmpty(true);

    d->ui->categoryCombo->setSplitActionVisible(false);
    d->ui->categoryCombo->clearSelection();
}

MultiTransactionEditor::~MultiTransactionEditor()
{
}

void MultiTransactionEditor::setAmountPlaceHolderText(const QAbstractItemModel* model)
{
    d->ui->creditDebitEdit->setPlaceholderText(model->headerData(JournalModel::Column::Payment, Qt::Horizontal).toString(),
                                               model->headerData(JournalModel::Column::Deposit, Qt::Horizontal).toString());
}

bool MultiTransactionEditor::accepted() const
{
    return d->accepted;
}

void MultiTransactionEditor::acceptEdit()
{
    d->accepted = true;
    Q_EMIT done();
}

void MultiTransactionEditor::loadTransaction(const QModelIndex& index)
{
    Q_UNUSED(index)
}

QStringList MultiTransactionEditor::saveTransaction(const QStringList& selectedJournalEntryIds)
{
    auto selection(selectedJournalEntryIds);
    connect(MyMoneyFile::instance()->journalModel(), &JournalModel::idChanged, this, [&](const QString& currentId, const QString& previousId) {
        selection.replaceInStrings(previousId, currentId);
    });

    if (d->anyChanges()) {
        const auto journalModel = MyMoneyFile::instance()->journalModel();
        MyMoneyFileTransaction ft;
        try {
            const auto file = MyMoneyFile::instance();
            for (const auto& journalEntryId : selectedJournalEntryIds) {
                const auto journalIdx = journalModel->indexById(journalEntryId);
                const auto journalEntry = journalModel->itemByIndex(journalIdx);
                auto t = journalEntry.transaction();
                auto sp = journalEntry.split();
                MyMoneySplit csp;

                if (t.splitCount() == 2) {
                    for (auto split : t.splits()) {
                        if (split.id() != sp.id()) {
                            csp = split;
                            break;
                        }
                    }
                }

                if (!d->ui->dateEdit->isNull()) {
                    t.setPostDate(d->ui->dateEdit->date());
                }
                if (!d->ui->statusCombo->currentText().isEmpty()) {
                    const auto idx = d->statusModel.index(d->ui->statusCombo->currentIndex(), 0);
                    sp.setReconcileFlag(idx.data(eMyMoney::Model::SplitReconcileStateRole).value<eMyMoney::Split::State>());
                    t.modifySplit(sp);
                }
                if (!d->ui->payeeEdit->lineEdit()->text().isEmpty()) {
                    const auto payeeRow = d->ui->payeeEdit->currentIndex();
                    const auto payeeIdx = d->payeesModel->index(payeeRow, 0);
                    const auto payeeId = payeeIdx.data(eMyMoney::Model::IdRole).toString();
                    sp.setPayeeId(payeeId);
                    t.modifySplit(sp);
                    if (!csp.id().isEmpty()) {
                        csp.setPayeeId(payeeId);
                        t.modifySplit(csp);
                    }
                }
                if (!d->ui->categoryCombo->getSelected().isEmpty()) {
                    if (!csp.id().isEmpty()) {
                        csp.setAccountId(d->ui->categoryCombo->getSelected());
                        t.modifySplit(csp);
                    }
                }

                file->modifyTransaction(t);
            }
            ft.commit();
        } catch (const MyMoneyException& e) {
            qDebug() << Q_FUNC_INFO << "something went wrong" << e.what();
            selection = selectedJournalEntryIds;
        }
    }
    return selection;
}

bool MultiTransactionEditor::eventFilter(QObject* o, QEvent* e)
{
    auto cb = qobject_cast<QComboBox*>(o);
    if (o) {
        // filter out wheel events for combo boxes if the popup view is not visible
        if ((e->type() == QEvent::Wheel) && !cb->view()->isVisible()) {
            return true;
        }

        if (e->type() == QEvent::FocusOut) {
            if (o == d->ui->categoryCombo) {
                if (!d->ui->categoryCombo->popup()->isVisible() && !cb->currentText().isEmpty() && !d->ui->categoryCombo->lineEdit()->isReadOnly()) {
                    const auto accountId = d->ui->categoryCombo->getSelected();
                    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    if (!accountIdx.isValid() || accountIdx.data(eMyMoney::Model::AccountFullNameRole).toString().compare(cb->currentText())) {
                        d->createCategory();
                    }
                }

            } else if (o == d->ui->payeeEdit) {
                if (!cb->currentText().isEmpty()) {
                    const auto index(cb->findText(cb->currentText()));
                    if (index != -1) {
                        cb->setCurrentIndex(index);
                    } else {
                        d->createPayee();
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

QDate MultiTransactionEditor::postDate() const
{
    return d->ui->dateEdit->date();
}

void MultiTransactionEditor::setShowAccountCombo(bool show) const
{
    d->ui->accountLabel->setVisible(show);
    d->ui->accountCombo->setVisible(show);
    d->ui->topMarginWidget->setVisible(show);
    d->ui->accountCombo->setSplitActionVisible(false);
}

void MultiTransactionEditor::setShowButtons(bool show) const
{
    d->ui->enterButton->setVisible(show);
    d->ui->cancelButton->setVisible(show);
}

void MultiTransactionEditor::setShowNumberWidget(bool show) const
{
    d->ui->numberLabel->setVisible(show);
    d->ui->numberEdit->setVisible(show);
}

void MultiTransactionEditor::setAccountId(const QString& accountId)
{
    d->ui->accountCombo->setSelected(accountId);
}

void MultiTransactionEditor::setReadOnly(bool readOnly)
{
    if (isReadOnly() != readOnly) {
        TransactionEditorBase::setReadOnly(readOnly);
        if (readOnly) {
            d->frameCollection->removeWidget(d->ui->enterButton);
            d->ui->enterButton->setDisabled(true);
        } else {
            // no need to enable the enter button here as the
            // frameCollection will take care of it anyway
            d->frameCollection->addWidget(d->ui->enterButton);
        }
    }
}

void MultiTransactionEditor::setupUi(QWidget* parent)
{
    if (d->tabOrderUi == nullptr) {
        d->tabOrderUi = new Ui::NewTransactionEditor;
    }
    d->tabOrderUi->setupUi(parent);
    d->tabOrderUi->accountLabel->setVisible(false);
    d->tabOrderUi->accountCombo->setVisible(false);
}

void MultiTransactionEditor::storeTabOrder(const QStringList& tabOrder)
{
    TransactionEditorBase::storeTabOrder(QLatin1String("multiTransactionEditor"), tabOrder);
}

void MultiTransactionEditor::slotSettingsChanged()
{
    d->categoriesModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
    d->accountsModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
}

bool MultiTransactionEditor::setSelectedJournalEntryIds(const QStringList& selectedJournalEntryIds)
{
    d->selectedJournalEntryIds = selectedJournalEntryIds;
    const auto journalModel = MyMoneyFile::instance()->journalModel();
    bool payeeEnabled(true);
    bool categoryEnabled(true);
    bool amountEnabled(true);
    bool costCenterEnabled(true);
    bool tagEnabled(true);
    bool memoEnabled(true);
    bool statusEnabled(true);

    for (const auto& journalEntryId : selectedJournalEntryIds) {
        const auto idx = journalModel->indexById(journalEntryId);
        if (idx.data(eMyMoney::Model::BaseModelRole) == eMyMoney::Model::SchedulesJournalEntryRole) {
            d->errorMessage = i18nc("@info Error selecting multiple transactions for edit", "Cannot edit multiple schedules at once.");
            return false;
        }
        if (idx.data(eMyMoney::Model::JournalEntryIsFrozenRole).toBool() == true) {
            statusEnabled = false;
        }
        if (idx.data(eMyMoney::Model::TransactionSplitCountRole).toInt() > 2) {
            WidgetHintFrame::hide(d->ui->categoryCombo,
                                  i18n("Selection of category or account is not possible when split transactions are part of the selection."));
            categoryEnabled = false;
        }
    }

    // now disable the widgets which the user cannot use due to selected transactions
    d->ui->dateEdit->setEnabled(true);
    d->ui->creditDebitEdit->setEnabled(amountEnabled);
    d->ui->payeeEdit->setEnabled(payeeEnabled);
    d->ui->numberEdit->setEnabled(false);
    d->ui->categoryCombo->setEnabled(categoryEnabled);
    d->ui->costCenterCombo->setEnabled(costCenterEnabled);
    d->ui->tagContainer->setEnabled(tagEnabled);
    d->ui->statusCombo->setEnabled(statusEnabled);
    d->ui->memoEdit->setEnabled(memoEnabled);
    d->ui->enterButton->setEnabled(true);
    return true;
}

QString MultiTransactionEditor::errorMessage() const
{
    return d->errorMessage;
}
