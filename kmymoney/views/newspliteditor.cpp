/*
    SPDX-FileCopyrightText: 2016-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newspliteditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QCompleter>
#include <QDate>
#include <QDebug>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConcatenateRowsProxyModel>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountcreator.h"
#include "accountsmodel.h"
#include "costcentermodel.h"
#include "creditdebitedit.h"
#include "icons.h"
#include "journalmodel.h"
#include "kcurrencycalculator.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneyutils.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "payeecreator.h"
#include "payeesmodel.h"
#include "securitiesmodel.h"
#include "splitmodel.h"
#include "splitview.h"
#include "widgethintframe.h"

#include "ui_newspliteditor.h"

using namespace Icons;

struct NewSplitEditor::Private
{
    Private(NewSplitEditor* parent)
        : q(parent)
        , ui(new Ui_NewSplitEditor)
        , tabOrderUi(nullptr)
        , accountsModel(new AccountNamesFilterProxyModel(parent))
        , payeesModel(new QSortFilterProxyModel(parent))
        , costCenterModel(new QSortFilterProxyModel(parent))
        , splitModel(nullptr)
        , accepted(false)
        , costCenterRequired(false)
        , showValuesInverted(false)
        , loadingSplit(false)
        , isIncomeExpense(false)
        , readOnly(false)
        , postDate(QDate::currentDate())
    {
        accountsModel->setObjectName("AccountNamesFilterProxyModel");
        costCenterModel->setObjectName("SortedCostCenterModel");
        payeesModel->setObjectName("SortedPayeesModel");

        costCenterModel->setSortLocaleAware(true);
        costCenterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        payeesModel->setSortLocaleAware(true);
        payeesModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    }

    ~Private()
    {
        delete ui;
    }

    bool checkForValidSplit(bool doUserInteraction = true);

    bool costCenterChanged(int costCenterIndex);
    bool categoryChanged(const QString& accountId);
    bool numberChanged(const QString& newNumber);
    bool amountChanged();
    void setupTabOrder();
    void createCategory();
    void createPayee();

    NewSplitEditor* q;
    Ui_NewSplitEditor* ui;
    Ui_NewSplitEditor* tabOrderUi;
    AccountNamesFilterProxyModel* accountsModel;
    QSortFilterProxyModel* payeesModel;
    QSortFilterProxyModel* costCenterModel;
    SplitModel* splitModel;
    bool accepted;
    bool costCenterRequired;
    bool showValuesInverted;
    bool loadingSplit;
    bool isIncomeExpense;
    bool readOnly;
    MyMoneyAccount counterAccount;
    MyMoneyAccount category;
    MyMoneySecurity commodity;
    MyMoneyMoney value;
    MyMoneyMoney shares;
    QDate postDate;
    WidgetHintFrameCollection* frameCollection;
};

bool NewSplitEditor::Private::checkForValidSplit(bool doUserInteraction)
{
    QStringList infos;
    bool rc = true;
    if(!costCenterChanged(ui->costCenterCombo->currentIndex())) {
        infos << ui->costCenterCombo->toolTip();
        rc = false;
    }

    if (!categoryChanged(ui->accountCombo->getSelected())) {
        infos << ui->accountCombo->toolTip();
        rc = false;
    }

    if(doUserInteraction) {
        /// @todo add dialog here that shows the @a infos
    }
    return rc;
}

bool NewSplitEditor::Private::costCenterChanged(int costCenterIndex)
{
    bool rc = true;
    WidgetHintFrame::hide(ui->costCenterCombo,
                          i18nc("@info:tooltip costcenter combo in split editor", "The cost center this transaction should be assigned to."));
    if(costCenterIndex != -1) {
        if(costCenterRequired && ui->costCenterCombo->currentText().isEmpty()) {
            WidgetHintFrame::show(
                ui->costCenterCombo,
                i18nc("@info:tooltip costcenter combo in split editor", "A cost center assignment is required for a transaction in the selected category."));
            rc = false;
        }
    }
    return rc;
}

bool NewSplitEditor::Private::categoryChanged(const QString& accountId)
{
    bool rc = true;
    isIncomeExpense = false;
    WidgetHintFrame::hide(ui->accountCombo, i18nc("@info:tooltip category combo in split editor", "The category this split should be assigned to."));
    if(!accountId.isEmpty()) {
        try {
            const auto category = MyMoneyFile::instance()->account(accountId);
            const bool isIncomeExpense = category.isIncomeExpense();
            ui->costCenterCombo->setEnabled(isIncomeExpense);
            ui->costCenterLabel->setEnabled(isIncomeExpense);
            ui->numberEdit->setDisabled(isIncomeExpense);
            ui->numberLabel->setDisabled(isIncomeExpense);

            if (isIncomeExpense) {
                ui->numberEdit->clear();
            } else {
                numberChanged(ui->numberEdit->text());
            }

            const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
            const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
            const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);

            // in case the commodity changes, we need to update the shares part
            if (currency.id() != ui->creditDebitEdit->sharesCommodity().id()) {
                ui->creditDebitEdit->setSharesCommodity(currency);
                auto sharesAmount = ui->creditDebitEdit->value();
                ui->creditDebitEdit->setShares(sharesAmount);
                // switch to value display so that we show the transaction commodity
                // for single currency data entry this does not have an effect
                ui->creditDebitEdit->setDisplayState(MultiCurrencyEdit::DisplayValue);

                if (!sharesAmount.isZero()) {
                    KCurrencyCalculator::updateConversion(ui->creditDebitEdit, postDate);
                }
            }

            costCenterRequired = category.isCostCenterRequired();
            rc &= costCenterChanged(ui->costCenterCombo->currentIndex());

        } catch (MyMoneyException &e) {
            qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
        }
    } else {
        WidgetHintFrame::show(ui->accountCombo, i18nc("@info:tooltip category combo in split editor", "A category assignment is required for a split."));
        rc = false;
    }
    return rc;
}

bool NewSplitEditor::Private::numberChanged(const QString& newNumber)
{
    bool rc = true;
    WidgetHintFrame::hide(ui->numberEdit, i18n("The check number used for this transaction."));
    if(!newNumber.isEmpty()) {
        const auto model = MyMoneyFile::instance()->journalModel();
        const auto rows = model->rowCount();
        const auto accountId = ui->accountCombo->getSelected();
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accountId) {
                if (idx.data(eMyMoney::Model::JournalSplitNumberRole).toString() == newNumber) {
                    WidgetHintFrame::show(ui->numberEdit, i18n("The check number <b>%1</b> has already been used in this account.", newNumber));
                    rc = false;
                    break;
                }
            }
        }
    }
    return rc;
}

bool NewSplitEditor::Private::amountChanged()
{
    // bypass a simple reverse in sign because the exchange rate does not change
    if ((shares != -ui->creditDebitEdit->shares()) || (value != -ui->creditDebitEdit->value())) {
        // and if there is no real change, don't call the currency calculator
        if ((shares != ui->creditDebitEdit->shares()) || (value != ui->creditDebitEdit->value())) {
            KCurrencyCalculator::updateConversion(ui->creditDebitEdit, postDate);
            shares = ui->creditDebitEdit->shares();
            value = ui->creditDebitEdit->value();
        }
    } else {
        shares = -shares;
        value = -value;
    }
    return true;
}

void NewSplitEditor::Private::setupTabOrder()
{
    const auto defaultTabOrder = QStringList{
        QLatin1String("creditDebitEdit"),
        QLatin1String("payeeEdit"),
        QLatin1String("numberEdit"),
        QLatin1String("accountCombo"),
        QLatin1String("costCenterCombo"),
        QLatin1String("tagContainer"),
        QLatin1String("memoEdit"),
        QLatin1String("enterButton"),
        QLatin1String("cancelButton"),
    };
    q->setProperty("kmm_defaulttaborder", defaultTabOrder);
    q->setProperty("kmm_currenttaborder", KMyMoneyUtils::tabOrder(QLatin1String("splitTransactionEditor"), defaultTabOrder));

    KMyMoneyUtils::setupTabOrder(q, q->property("kmm_currenttaborder").toStringList());
}

void NewSplitEditor::Private::createCategory()
{
    // delay the execution of this code for 150ms so
    // that a click on the cancel or enter button has
    // a chance to be executed before.
    auto creator = new AccountCreator(q);
    creator->setComboBox(ui->accountCombo);
    creator->addButton(ui->cancelButton);
    creator->addButton(ui->enterButton);
    creator->setAccountType(eMyMoney::Account::Type::Expense);
    if (ui->creditDebitEdit->haveValue() && ui->creditDebitEdit->value().isPositive()) {
        creator->setAccountType(eMyMoney::Account::Type::Income);
    }
    creator->createAccount();
}

void NewSplitEditor::Private::createPayee()
{
    auto creator = new PayeeCreator(q);
    creator->setComboBox(ui->payeeEdit);
    creator->addButton(ui->cancelButton);
    creator->addButton(ui->enterButton);
    creator->createPayee();
}

NewSplitEditor::NewSplitEditor(QWidget* parent, const MyMoneySecurity& commodity, const QString& counterAccountId)
    : QWidget(parent)
    , d(new Private(this))
{
    d->commodity = commodity;
    auto const file = MyMoneyFile::instance();
    auto view = qobject_cast<SplitView*>(parent->parentWidget());
    Q_ASSERT(view != 0);
    d->splitModel = qobject_cast<SplitModel*>(view->model());

    auto const model = MyMoneyFile::instance()->accountsModel();
    d->counterAccount = model->itemById(counterAccountId);

    d->ui->setupUi(this);
    d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
    d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

    auto concatModel = new KConcatenateRowsProxyModel(parent);
    concatModel->addSourceModel(file->payeesModel()->emptyPayee());
    concatModel->addSourceModel(file->payeesModel());
    d->payeesModel->setSortRole(Qt::DisplayRole);
    d->payeesModel->setSourceModel(concatModel);
    d->payeesModel->sort(0);

    d->ui->payeeEdit->setEditable(true);
    d->ui->payeeEdit->lineEdit()->setClearButtonEnabled(true);
    d->ui->payeeEdit->setModel(d->payeesModel);
    d->ui->payeeEdit->setModelColumn(0);
    d->ui->payeeEdit->completer()->setCompletionMode(QCompleter::PopupCompletion);
    d->ui->payeeEdit->completer()->setFilterMode(Qt::MatchContains);

    d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense, eMyMoney::Account::Type::Equity,});
    d->accountsModel->setHideEquityAccounts(false);
    d->accountsModel->setHideZeroBalancedEquityAccounts(false);
    d->accountsModel->setSourceModel(model);
    d->accountsModel->sort(AccountsModel::Column::AccountName);
    d->ui->accountCombo->setModel(d->accountsModel);
    d->ui->accountCombo->setSplitActionVisible(false);

    d->costCenterModel->setSortRole(Qt::DisplayRole);
    d->costCenterModel->setSourceModel(MyMoneyFile::instance()->costCenterModel());
    d->costCenterModel->sort(AccountsModel::Column::AccountName);

    d->ui->costCenterCombo->setEditable(true);
    d->ui->costCenterCombo->setModel(d->costCenterModel);
    d->ui->costCenterCombo->setModelColumn(0);
    d->ui->costCenterCombo->completer()->setFilterMode(Qt::MatchContains);

    d->frameCollection = new WidgetHintFrameCollection(this);
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->accountCombo));
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
    d->frameCollection->addWidget(d->ui->enterButton);

    d->ui->creditDebitEdit->setAllowEmpty(true);
    d->ui->creditDebitEdit->setCommodity(commodity);

    connect(d->ui->numberEdit, &QLineEdit::textChanged, this, [&](const QString& txt) {
        d->numberChanged(txt);
    });
    connect(d->ui->costCenterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int costCenterIndex) {
        d->costCenterChanged(costCenterIndex);
    });
    connect(d->ui->accountCombo, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& categoryId) {
        d->categoryChanged(categoryId);
    });
    connect(d->ui->creditDebitEdit, &CreditDebitEdit::amountChanged, this, [&]() {
        d->amountChanged();
    });

    connect(d->ui->cancelButton, &QToolButton::clicked, this, &NewSplitEditor::reject);
    connect(d->ui->enterButton, &QToolButton::clicked, this, &NewSplitEditor::acceptEdit);

    d->ui->accountCombo->installEventFilter(this);
    d->ui->payeeEdit->installEventFilter(this);

    // setup the tab order
    d->setupTabOrder();

    // determine order of credit and debit edit widgets
    // based on their visual order in the ledger
    int creditColumn = SplitModel::Column::Payment;
    int debitColumn = SplitModel::Column::Deposit;

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

    // set focus to first tab field once we return to event loop
    const auto tabOrder = property("kmm_currenttaborder").toStringList();
    if (!tabOrder.isEmpty()) {
        const auto focusWidget = findChild<QWidget*>(tabOrder.first());
        if (focusWidget) {
            QMetaObject::invokeMethod(focusWidget, "setFocus", Qt::QueuedConnection);
        }
    }
}

NewSplitEditor::~NewSplitEditor()
{
}

void NewSplitEditor::setAmountPlaceHolderText(const QAbstractItemModel* model)
{
    d->ui->creditDebitEdit->setPlaceholderText(model->headerData(SplitModel::Column::Payment, Qt::Horizontal).toString(),
                                               model->headerData(SplitModel::Column::Deposit, Qt::Horizontal).toString());
}

void NewSplitEditor::setPostDate(const QDate& date)
{
    d->postDate = date;
}

void NewSplitEditor::setShowValuesInverted(bool inverse)
{
    d->showValuesInverted = inverse;
}

bool NewSplitEditor::showValuesInverted()
{
    return d->showValuesInverted;
}

bool NewSplitEditor::accepted() const
{
    return d->accepted;
}

void NewSplitEditor::acceptEdit()
{
    if(d->checkForValidSplit()) {
        d->accepted = true;
        emit done();
    }
}

void NewSplitEditor::reject()
{
    emit done();
}

void NewSplitEditor::keyPressEvent(QKeyEvent* event)
{
    if (!event->modifiers() || (event->modifiers() & Qt::KeypadModifier && event->key() == Qt::Key_Enter)) {
        switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            if(focusWidget() == d->ui->cancelButton) {
                reject();
            } else {
                if (d->ui->enterButton->isEnabled() && !d->readOnly) {
                    d->ui->enterButton->setFocus();
                    d->ui->enterButton->click();
                }
                return;
            }
        }
        break;

        case Qt::Key_Escape:
            reject();
            break;

        default:
            event->ignore();
            return;
        }
    } else {
        const auto keySeq = QKeySequence(event->modifiers() + event->key());

        if (keySeq.matches(pActions[eMenu::Action::EditTabOrder]->shortcut())) {
            QPointer<TabOrderDialog> tabOrderDialog = new TabOrderDialog(this);
            auto tabOrderWidget = static_cast<TabOrderEditorInterface*>(qt_metacast("TabOrderEditorInterface"));
            if (tabOrderWidget) {
                tabOrderDialog->setTarget(tabOrderWidget);
                auto tabOrder = property("kmm_defaulttaborder").toStringList();
                tabOrderDialog->setDefaultTabOrder(tabOrder);
                tabOrder = property("kmm_currenttaborder").toStringList();
                tabOrderDialog->setTabOrder(tabOrder);

                if ((tabOrderDialog->exec() == QDialog::Accepted) && tabOrderDialog) {
                    tabOrderWidget->storeTabOrder(tabOrderDialog->tabOrder());
                    d->setupTabOrder();
                }
            }
            tabOrderDialog->deleteLater();
        }
        event->ignore();
    }
}

QString NewSplitEditor::accountId() const
{
    return d->ui->accountCombo->getSelected();
}

void NewSplitEditor::setAccountId(const QString& id)
{
    d->ui->accountCombo->clearEditText();
    d->ui->accountCombo->setSelected(id);
}


QString NewSplitEditor::memo() const
{
    return d->ui->memoEdit->toPlainText();
}

void NewSplitEditor::setMemo(const QString& memo)
{
    d->ui->memoEdit->setPlainText(memo);
}

MyMoneyMoney NewSplitEditor::shares() const
{
    return d->ui->creditDebitEdit->shares();
}

void NewSplitEditor::setShares(const MyMoneyMoney& amount)
{
    d->shares = amount;
    d->ui->creditDebitEdit->setShares(amount);
}

MyMoneyMoney NewSplitEditor::value() const
{
    return d->ui->creditDebitEdit->value();
}

void NewSplitEditor::setValue(const MyMoneyMoney& amount)
{
    d->value = amount;
    d->ui->creditDebitEdit->setValue(amount);
}

QString NewSplitEditor::costCenterId() const
{
    const int row = d->ui->costCenterCombo->currentIndex();
    QModelIndex index = d->ui->costCenterCombo->model()->index(row, 0);
    return d->ui->costCenterCombo->model()->data(index, eMyMoney::Model::Roles::IdRole).toString();
}

void NewSplitEditor::setCostCenterId(const QString& id)
{
    const auto baseIdx = MyMoneyFile::instance()->costCenterModel()->indexById(id);
    if (baseIdx.isValid()) {
        const auto index = MyMoneyFile::baseModel()->mapFromBaseSource(d->costCenterModel, baseIdx);
        if(index.isValid()) {
            d->ui->costCenterCombo->setCurrentIndex(index.row());
        }
    }
}

QString NewSplitEditor::number() const
{
    return d->ui->numberEdit->text();
}

void NewSplitEditor::setNumber(const QString& number)
{
    d->ui->numberEdit->setText(number);
}

QString NewSplitEditor::payeeId() const
{
    const auto idx = d->payeesModel->index(d->ui->payeeEdit->currentIndex(), 0);
    return idx.data(eMyMoney::Model::IdRole).toString();
}

void NewSplitEditor::setPayeeId(const QString& id)
{
    QModelIndexList indexes = d->payeesModel->match(d->payeesModel->index(0, 0), eMyMoney::Model::IdRole, QVariant(id), 1, Qt::MatchFlags(Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive)));
    int row(0);
    if (!indexes.isEmpty()) {
        row = indexes.first().row();
    }
    d->ui->payeeEdit->setCurrentIndex(row);
}

void NewSplitEditor::startLoadingSplit()
{
    d->loadingSplit = true;
}

void NewSplitEditor::finishLoadingSplit()
{
    d->loadingSplit = false;
}

void NewSplitEditor::setReadOnly(bool readOnly)
{
    if (d->readOnly != readOnly) {
        d->readOnly = readOnly;
        if (readOnly) {
            d->frameCollection->removeWidget(d->ui->enterButton);
            d->ui->enterButton->setDisabled(true);
        } else {
            // no need to enable the enter button here as the
            // framewidget will take care of it anyway
            d->frameCollection->addWidget(d->ui->enterButton);
        }
    }
}

void NewSplitEditor::setupUi(QWidget* parent)
{
    if (d->tabOrderUi == nullptr) {
        d->tabOrderUi = new Ui::NewSplitEditor;
    }
    d->tabOrderUi->setupUi(parent);
}

void NewSplitEditor::storeTabOrder(const QStringList& tabOrder)
{
    KMyMoneyUtils::storeTabOrder(QLatin1String("splitTransactionEditor"), tabOrder);
}

bool NewSplitEditor::focusNextPrevChild(bool next)
{
    auto rc = KMyMoneyUtils::tabFocusHelper(this, next);

    if (rc == false) {
        rc = QWidget::focusNextPrevChild(next);
    }
    return rc;
}

bool NewSplitEditor::eventFilter(QObject* o, QEvent* e)
{
    auto cb = qobject_cast<QComboBox*>(o);
    if (o) {
        // filter out wheel events for combo boxes if the popup view is not visible
        if ((e->type() == QEvent::Wheel) && !cb->view()->isVisible()) {
            return true;
        }

        if (e->type() == QEvent::FocusOut) {
            if (o == d->ui->accountCombo) {
                if (!d->ui->accountCombo->popup()->isVisible() && !cb->currentText().isEmpty()) {
                    const auto accountId = d->ui->accountCombo->getSelected();
                    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    if (!accountIdx.isValid() || accountIdx.data(eMyMoney::Model::AccountFullNameRole).toString().compare(cb->currentText())) {
                        d->createCategory();
                    }
                }

            } else if (o == d->ui->payeeEdit) {
                if (!cb->currentText().isEmpty()) {
                    const auto index(cb->findText(cb->currentText(), Qt::MatchExactly | Qt::MatchCaseSensitive));
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
