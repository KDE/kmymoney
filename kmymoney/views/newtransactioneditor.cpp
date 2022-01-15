/*
    SPDX-FileCopyrightText: 2015-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "newtransactioneditor.h"

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

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KConcatenateRowsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "costcentermodel.h"
#include "icons.h"
#include "idfilter.h"
#include "journalmodel.h"
#include "kcurrencycalculator.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneysettings.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "payeesmodel.h"
#include "securitiesmodel.h"
#include "splitdialog.h"
#include "splitmodel.h"
#include "statusmodel.h"
#include "tagsmodel.h"
#include "widgethintframe.h"

#include "ui_newtransactioneditor.h"

using namespace Icons;

class NewTransactionEditor::Private
{
public:
    enum TaxValueChange {
        ValueUnchanged,
        ValueChanged,
    };
    Private(NewTransactionEditor* parent)
        : q(parent)
        , ui(new Ui_NewTransactionEditor)
        , tabOrderUi(nullptr)
        , accountsModel(new AccountNamesFilterProxyModel(parent))
        , categoriesModel(new AccountNamesFilterProxyModel(parent))
        , costCenterModel(new QSortFilterProxyModel(parent))
        , payeesModel(new QSortFilterProxyModel(parent))
        , accepted(false)
        , costCenterRequired(false)
        , inUpdateVat(false)
        , splitModel(parent, &undoStack)
        , frameCollection(nullptr)
    {
        accountsModel->setObjectName(QLatin1String("NewTransactionEditor::accountsModel"));
        categoriesModel->setObjectName(QLatin1String("NewTransactionEditor::categoriesModel"));
        costCenterModel->setObjectName(QLatin1String("SortedCostCenterModel"));
        payeesModel->setObjectName(QLatin1String("SortedPayeesModel"));
        splitModel.setObjectName(QLatin1String("SplitModel"));

        costCenterModel->setSortLocaleAware(true);
        costCenterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        payeesModel->setSortLocaleAware(true);
        payeesModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    }

    ~Private()
    {
        delete ui;
    }

    void updateWidgetState();
    void setupTabOrder();
    bool checkForValidTransaction(bool doUserInteraction = true);
    bool isDatePostOpeningDate(const QDate& date, const QString& accountId);
    bool postdateChanged(const QDate& date);
    bool costCenterChanged(int costCenterIndex);
    bool payeeChanged(int payeeIndex);
    void accountChanged(const QString& id);
    bool categoryChanged(const QString& id);
    bool numberChanged(const QString& newNumber);
    bool amountChanged();
    bool isIncomeExpense(const QModelIndex& idx) const;
    bool isIncomeExpense(const QString& categoryId) const;
    bool tagsChanged(const QStringList& ids);
    int editSplits();
    void updateWidgetAccess();
    void updateVAT(TaxValueChange amountChanged);
    MyMoneyMoney removeVatSplit();
    MyMoneyMoney splitsSum() const;

    NewTransactionEditor* q;
    Ui_NewTransactionEditor* ui;
    Ui_NewTransactionEditor* tabOrderUi;
    AccountNamesFilterProxyModel* accountsModel;
    AccountNamesFilterProxyModel* categoriesModel;
    QSortFilterProxyModel* costCenterModel;
    QSortFilterProxyModel* payeesModel;
    bool accepted;
    bool costCenterRequired;
    bool inUpdateVat;
    QUndoStack undoStack;
    SplitModel splitModel;
    MyMoneyAccount m_account;
    MyMoneyTransaction m_transaction;
    MyMoneySplit m_split;
    WidgetHintFrameCollection* frameCollection;
};

void NewTransactionEditor::Private::updateWidgetAccess()
{
    const auto enable = !m_account.id().isEmpty();
    ui->dateEdit->setEnabled(enable);
    ui->creditDebitEdit->setEnabled(enable);
    ui->payeeEdit->setEnabled(enable);
    ui->numberEdit->setEnabled(enable);
    ui->categoryCombo->setEnabled(enable);
    ui->costCenterCombo->setEnabled(enable);
    ui->tagContainer->setEnabled(enable);
    ui->statusCombo->setEnabled(enable);
    ui->memoEdit->setEnabled(enable);
    ui->enterButton->setEnabled(!q->isReadOnly());
}

void NewTransactionEditor::Private::updateWidgetState()
{
    // update the category combo box
    auto index = splitModel.index(0, 0);

    // update the costcenter combo box
    if (ui->costCenterCombo->isEnabled()) {
        // extract the cost center
        index = MyMoneyFile::instance()->costCenterModel()->indexById(index.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
        if (index.isValid())
            ui->costCenterCombo->setCurrentIndex(costCenterModel->mapFromSource(index).row());
    }
}

bool NewTransactionEditor::Private::checkForValidTransaction(bool doUserInteraction)
{
    QStringList infos;
    bool rc = true;
    if (!postdateChanged(ui->dateEdit->date())) {
        infos << ui->dateEdit->toolTip();
        rc = false;
    }

    if (!costCenterChanged(ui->costCenterCombo->currentIndex())) {
        infos << ui->costCenterCombo->toolTip();
        rc = false;
    }

    if (doUserInteraction) {
        /// @todo add dialog here that shows the @a infos about the problem
    }
    return rc;
}

bool NewTransactionEditor::Private::isDatePostOpeningDate(const QDate& date, const QString& accountId)
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
    } catch (MyMoneyException& e) {
        qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
    }
    return rc;
}

bool NewTransactionEditor::Private::postdateChanged(const QDate& date)
{
    bool rc = true;
    WidgetHintFrame::hide(ui->dateEdit, i18n("The posting date of the transaction."));

    // collect all account ids
    QStringList accountIds;
    accountIds << m_account.id();
    const auto rows = splitModel.rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto index = splitModel.index(row, 0);
        accountIds << index.data(eMyMoney::Model::SplitAccountIdRole).toString();
    }

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


bool NewTransactionEditor::Private::costCenterChanged(int costCenterIndex)
{
    bool rc = true;
    WidgetHintFrame::hide(ui->costCenterCombo, i18n("The cost center this transaction should be assigned to."));
    if (costCenterIndex != -1) {
        if (costCenterRequired && ui->costCenterCombo->currentText().isEmpty()) {
            WidgetHintFrame::show(ui->costCenterCombo, i18n("A cost center assignment is required for a transaction in the selected category."));
            rc = false;
        }
        if (rc == true && splitModel.rowCount() == 1) {
            auto index = costCenterModel->index(costCenterIndex, 0);
            const auto costCenterId = index.data(eMyMoney::Model::IdRole).toString();
            index = splitModel.index(0, 0);

            splitModel.setData(index, costCenterId, eMyMoney::Model::SplitCostCenterIdRole);
        }
    }

    return rc;
}

bool NewTransactionEditor::Private::isIncomeExpense(const QString& categoryId) const
{
    if (!categoryId.isEmpty()) {
        MyMoneyAccount category = MyMoneyFile::instance()->account(categoryId);
        return category.isIncomeExpense();
    }
    return false;
}

bool NewTransactionEditor::Private::isIncomeExpense(const QModelIndex& idx) const
{
    return isIncomeExpense(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
}

void NewTransactionEditor::Private::accountChanged(const QString& id)
{
    m_account = MyMoneyFile::instance()->accountsModel()->itemById(id);
    m_split.setAccountId(id);

    m_transaction.setCommodity(m_account.currencyId());

    // in case we have a single split, we set the accountCombo again
    // so that a possible foreign currency is also taken care of.
    if (splitModel.rowCount() == 1) {
        ui->categoryCombo->setSelected(splitModel.index(0, 0).data(eMyMoney::Model::SplitAccountIdRole).toString());
    }

    updateWidgetAccess();
}

bool NewTransactionEditor::Private::categoryChanged(const QString& accountId)
{
    bool rc = true;
    if (splitModel.rowCount() <= 1) {
        if (!accountId.isEmpty()) {
            try {
                MyMoneyAccount category = MyMoneyFile::instance()->account(accountId);
                const bool isIncomeExpense = category.isIncomeExpense();
                ui->costCenterCombo->setEnabled(isIncomeExpense);
                ui->costCenterLabel->setEnabled(isIncomeExpense);
                costCenterRequired = category.isCostCenterRequired();

                // make sure we have a split in the model
                if (splitModel.rowCount() == 0) {
                    // add an empty split
                    MyMoneySplit s;
                    splitModel.addItem(s);
                }

                const auto index = splitModel.index(0, 0);
                splitModel.setData(index, accountId, eMyMoney::Model::SplitAccountIdRole);

                rc &= costCenterChanged(ui->costCenterCombo->currentIndex());
                rc &= postdateChanged(ui->dateEdit->date());
                payeeChanged(ui->payeeEdit->currentIndex());

                // extract the categories currency
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
                        KCurrencyCalculator::updateConversion(ui->creditDebitEdit, ui->dateEdit->date());
                    }
                }

                splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->value()), eMyMoney::Model::SplitValueRole);
                splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->shares()), eMyMoney::Model::SplitSharesRole);

                updateVAT(ValueUnchanged);

            } catch (MyMoneyException& e) {
                qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
            }
        } else {
            splitModel.unload();
        }
    }
    return rc;
}

bool NewTransactionEditor::Private::numberChanged(const QString& newNumber)
{
    bool rc = true; // number did change
    WidgetHintFrame::hide(ui->numberEdit, i18n("The check number used for this transaction."));
    if (!newNumber.isEmpty()) {
        auto model = MyMoneyFile::instance()->journalModel();
        const QModelIndexList list = model->match(model->index(0, 0), eMyMoney::Model::SplitNumberRole,
                                     QVariant(newNumber),
                                     -1,                         // all splits
                                     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
        for (const auto& idx : list) {
            if (idx.data(eMyMoney::Model::SplitAccountIdRole).toString() == m_account.id()
                && idx.data(eMyMoney::Model::JournalTransactionIdRole).toString().compare(m_transaction.id())) {
                WidgetHintFrame::show(ui->numberEdit, i18n("The check number <b>%1</b> has already been used in this account.", newNumber));
                rc = false;
                break;
            }
        }
    }
    return rc;
}

bool NewTransactionEditor::Private::amountChanged()
{
    bool rc = true;
    if (ui->creditDebitEdit->haveValue() && (splitModel.rowCount() <= 1)) {
        rc = false;
        try {
            if (splitModel.rowCount() == 1) {
                const QModelIndex index = splitModel.index(0, 0);

                // check if there is a change in the values other than simply reverting the sign
                // and get an updated price in that case
                if ((index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>() != ui->creditDebitEdit->shares())
                    || (index.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>() != ui->creditDebitEdit->value())) {
                    KCurrencyCalculator::updateConversion(ui->creditDebitEdit, ui->dateEdit->date());
                }

                splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->shares()), eMyMoney::Model::SplitSharesRole);
                splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->value()), eMyMoney::Model::SplitValueRole);
            }
            rc = true;

        } catch (MyMoneyException& e) {
            qDebug() << "Ooops: something went wrong in" << Q_FUNC_INFO;
        }
    } else {
        /// @todo ask what to do: if the rest of the splits is the same amount we could simply reverse the sign
        /// of all splits, otherwise we could ask if the user wants to start the split editor or anything else.
    }
    updateVAT(ValueChanged);
    return rc;
}

bool NewTransactionEditor::Private::payeeChanged(int payeeIndex)
{
    // copy payee information to second split if there are only two splits
    if (splitModel.rowCount() == 1) {
        const auto idx = splitModel.index(0, 0);
        const auto payeeId = payeesModel->index(payeeIndex, 0).data(eMyMoney::Model::IdRole).toString();
        splitModel.setData(idx, payeeId, eMyMoney::Model::SplitPayeeIdRole);
    }
    return true;
}

bool NewTransactionEditor::Private::tagsChanged(const QStringList& ids)
{
    if (splitModel.rowCount() == 1) {
        const auto idx = splitModel.index(0, 0);
        splitModel.setData(idx, ids, eMyMoney::Model::SplitTagIdRole);
    }
    return true;
}

MyMoneyMoney NewTransactionEditor::Private::splitsSum() const
{
    const auto rows = splitModel.rowCount();
    MyMoneyMoney value;
    for(int row = 0; row < rows; ++row) {
        const auto idx = splitModel.index(row, 0);
        value += idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
    }
    return value;
}

int NewTransactionEditor::Private::editSplits()
{
    const auto transactionFactor(ui->creditDebitEdit->value().isNegative() ? MyMoneyMoney::ONE : MyMoneyMoney::MINUS_ONE);

    SplitModel dlgSplitModel(q, nullptr, splitModel);

    // create an empty split at the end
    // used to create new splits, but only
    // when not in read-only mode
    if (!q->isReadOnly())
        dlgSplitModel.appendEmptySplit();

    // in case the transaction does only have a single split (the
    // one referencing the account) we keep a possible filled memo
    // and add it to the empty split.
    if ((dlgSplitModel.rowCount() == 1) && (!ui->memoEdit->toPlainText().isEmpty())) {
        const auto idx = dlgSplitModel.index(0, 0);
        dlgSplitModel.setData(idx, ui->memoEdit->toPlainText(), eMyMoney::Model::SplitMemoRole);
    }
    auto commodityId = m_transaction.commodity();
    if (commodityId.isEmpty())
        commodityId = m_account.currencyId();
    dlgSplitModel.setTransactionCommodity(commodityId);
    const auto commodity = MyMoneyFile::instance()->security(commodityId);

    QPointer<SplitDialog> splitDialog = new SplitDialog(commodity, -(q->transactionAmount()), m_account.fraction(), transactionFactor, q);
    const auto payeeId = payeesModel->index(ui->payeeEdit->currentIndex(), 0).data(eMyMoney::Model::IdRole).toString();
    splitDialog->setTransactionPayeeId(payeeId);
    splitDialog->setModel(&dlgSplitModel);
    splitDialog->setReadOnly(q->isReadOnly());

    int rc = splitDialog->exec();

    if (splitDialog && (rc == QDialog::Accepted)) {
        // remove that empty split again before we update the splits
        // no need to check for presence, removeEmptySplit() does that
        dlgSplitModel.removeEmptySplit();

        // copy the splits model contents
        splitModel = dlgSplitModel;

        // update the transaction amount
        ui->creditDebitEdit->setSharesCommodity(ui->creditDebitEdit->valueCommodity());
        ui->creditDebitEdit->setValue(-splitDialog->transactionAmount());
        auto amountShares = -splitDialog->transactionAmount();

        // the price might have been changed, so we have to update our copy
        // but only if there is one counter split
        if (splitModel.rowCount() == 1) {
            const auto idx = splitModel.index(0, 0);

            // use the shares based on the second split
            amountShares = -(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());

            // adjust the commodity for the shares
            const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
            const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
            const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
            const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);
            ui->creditDebitEdit->setSharesCommodity(currency);
        }
        ui->creditDebitEdit->setShares(amountShares);

        updateWidgetState();

        QWidget* next = ui->tagContainer->tagCombo();
        if (ui->costCenterCombo->isEnabled()) {
            next = ui->costCenterCombo;
        }
        next->setFocus();
    }

    if (splitDialog) {
        splitDialog->deleteLater();
    }

    return rc;
}

MyMoneyMoney NewTransactionEditor::Private::removeVatSplit()
{
    const auto rows = splitModel.rowCount();
    if (rows != 2)
        return ui->creditDebitEdit->value();

    QModelIndex netSplitIdx;
    QModelIndex taxSplitIdx;
    bool netValue(false);

    for (int row = 0; row < rows; ++row) {
        const auto idx = splitModel.index(row, 0);
        const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        const auto account = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
        // in case of failure, we simply stop processing
        if (account.id().isEmpty()) {
            return ui->creditDebitEdit->value();
        }
        if (!account.value(QLatin1String("VatAccount")).isEmpty()) {
            netValue = (account.value(QLatin1String("VatAmount")).toLower() == QLatin1String("net"));
            netSplitIdx = idx;
        } else if (!account.value(QLatin1String("VatRate")).isEmpty()) {
            taxSplitIdx = idx;
        }
    }

    // return if not all splits are setup
    if (!(taxSplitIdx.isValid() && netSplitIdx.isValid())) {
        return ui->creditDebitEdit->value();
    }

    MyMoneyMoney amount;
    // reduce the splits
    if (netValue) {
        amount = -(netSplitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
    } else {
        amount = -(netSplitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>()
                   + taxSplitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
    }

    // remove the tax split
    splitModel.removeRow(netSplitIdx.row());

    return amount;
}

void NewTransactionEditor::Private::updateVAT(TaxValueChange amountChanged)
{
    if (inUpdateVat) {
        return;
    }

    struct cleanupHelper {
        cleanupHelper(bool* lockVariable)
            : m_lockVariable(lockVariable)
        {
            *lockVariable = true;
        }
        ~cleanupHelper()
        {
            *m_lockVariable = false;
        }
        bool* m_lockVariable;
    } cleanupHelper(&inUpdateVat);

    const auto categoryId = ui->categoryCombo->getSelected();

    auto taxCategoryId = [&]() {
        if (categoryId.isEmpty()) {
            return QString();
        }
        const auto category = MyMoneyFile::instance()->account(categoryId);
        return category.value("VatAccount");
    };

    // if auto vat assignment for this account is turned off
    // we don't care about taxes
    if (m_account.value(QLatin1String("NoVat")).toLower() == QLatin1String("yes"))
        return;

    // more splits than category and tax are not supported
    if (splitModel.rowCount() > 2)
        return;

    // in order to do anything, we need an amount
    MyMoneyMoney amount, newAmount;
    amount = ui->creditDebitEdit->value();
    if (amount.isZero())
        return;

    MyMoneyAccount category;

    // If the transaction has a tax and a category split, remove the tax split
    if (splitModel.rowCount() == 2) {
        newAmount = removeVatSplit();
        if (splitModel.rowCount() == 2) // not removed?
            return;

        // now we have a single split with a category and check if the
        // value has changed and we need to update that split
        if (amountChanged == ValueChanged) {
            categoryChanged(categoryId);
        }
    } else {
        newAmount = amount;
    }

    const auto taxId = taxCategoryId();
    if (taxId.isEmpty())
        return;

    // seems we have everything we need
    if (amountChanged == ValueChanged)
        newAmount = amount;

    if (splitModel.rowCount() != 1)
        return;

    auto t = q->transaction();
    t.setCommodity(m_transaction.commodity());
    MyMoneyFile::instance()->updateVAT(t);

    // clear current splits and add them again
    splitModel.unload();
    for (const auto& split : t.splits()) {
        if ((split.accountId() == taxId) || split.accountId() == categoryId) {
            splitModel.appendSplit(split);
        }
    }
}

void NewTransactionEditor::Private::setupTabOrder()
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
    q->setProperty("kmm_currenttaborder", q->tabOrder(QLatin1String("stdTransactionEditor"), defaultTabOrder));

    q->setupTabOrder(q->property("kmm_currenttaborder").toStringList());
}

NewTransactionEditor::NewTransactionEditor(QWidget* parent, const QString& accountId)
    : TransactionEditorBase(parent, accountId)
    , d(new Private(this))
{
    auto const file = MyMoneyFile::instance();
    auto const model = file->accountsModel();
    // extract account information from model
    const auto index = model->indexById(accountId);
    d->m_account = model->itemByIndex(index);

    d->ui->setupUi(this);

    // default is to hide the account selection combobox
    setShowAccountCombo(false);

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

    const auto* splitHelper = new KMyMoneyAccountComboSplitHelper(d->ui->categoryCombo, &d->splitModel);
    connect(splitHelper, &KMyMoneyAccountComboSplitHelper::accountComboEnabled, d->ui->costCenterCombo, &QComboBox::setEnabled);
    connect(splitHelper, &KMyMoneyAccountComboSplitHelper::accountComboEnabled, d->ui->costCenterLabel, &QComboBox::setEnabled);

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
    connect(d->ui->payeeEdit->lineEdit(), &QLineEdit::textEdited,
            [&](const QString& txt)
    {
        if (txt.isEmpty()) {
            d->ui->payeeEdit->setCurrentIndex(0);
        }
    }
           );
    connect(d->ui->categoryCombo->lineEdit(), &QLineEdit::textEdited, [&](const QString& txt) {
        if (txt.isEmpty()) {
            d->ui->categoryCombo->setSelected(QString());
        }
    });
    d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
    d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

    d->ui->statusCombo->setModel(MyMoneyFile::instance()->statusModel());

    d->ui->dateEdit->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));

    d->ui->creditDebitEdit->setAllowEmpty(true);

    d->frameCollection = new WidgetHintFrameCollection(this);
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->dateEdit));
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
    d->frameCollection->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
    d->frameCollection->addWidget(d->ui->enterButton);

    connect(d->ui->numberEdit, &QLineEdit::textChanged, this, [&](const QString& newNumber) {
        d->numberChanged(newNumber);
    });

    connect(d->ui->costCenterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int costCenterIndex) {
        d->costCenterChanged(costCenterIndex);
    });

    connect(d->ui->accountCombo, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& id) {
        d->accountChanged(id);
    });
    connect(d->ui->categoryCombo, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& id) {
        d->categoryChanged(id);
    });

    connect(d->ui->categoryCombo, &KMyMoneyAccountCombo::splitDialogRequest, this, [&]() {
        d->editSplits();
    });

    connect(d->ui->dateEdit, &KMyMoneyDateEdit::dateChanged, this, [&](const QDate& date) {
        d->postdateChanged(date);
        emit postDateChanged(date);
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

    connect(d->ui->cancelButton, &QToolButton::clicked, this, &NewTransactionEditor::reject);
    connect(d->ui->enterButton, &QToolButton::clicked, this, &NewTransactionEditor::acceptEdit);

    // handle some events in certain conditions different from default
    d->ui->payeeEdit->installEventFilter(this);
    d->ui->costCenterCombo->installEventFilter(this);
    d->ui->tagContainer->tagCombo()->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);

    setCancelButton(d->ui->cancelButton);
    setEnterButton(d->ui->enterButton);
}

NewTransactionEditor::~NewTransactionEditor()
{
}

void NewTransactionEditor::setAmountPlaceHolderText(const QAbstractItemModel* model)
{
    d->ui->creditDebitEdit->setPlaceholderText(model->headerData(JournalModel::Column::Payment, Qt::Horizontal).toString(),
                                               model->headerData(JournalModel::Column::Deposit, Qt::Horizontal).toString());
}

bool NewTransactionEditor::accepted() const
{
    return d->accepted;
}

void NewTransactionEditor::acceptEdit()
{
    if (d->checkForValidTransaction()) {
        d->accepted = true;
        emit done();
    }
}

void NewTransactionEditor::loadSchedule(const MyMoneySchedule& schedule)
{
    if (schedule.transaction().splitCount() == 0) {
        // new schedule
        d->m_transaction = MyMoneyTransaction();
        d->m_transaction.setCommodity(MyMoneyFile::instance()->baseCurrency().id());
        d->m_split = MyMoneySplit();
        d->m_split.setAccountId(QString());
        const auto lastUsedPostDate = KMyMoneySettings::lastUsedPostDate();
        if (lastUsedPostDate.isValid()) {
            d->ui->dateEdit->setDate(lastUsedPostDate.date());
        } else {
            d->ui->dateEdit->setDate(QDate::currentDate());
        }
        QSignalBlocker accountBlocker(d->ui->categoryCombo->lineEdit());
        d->ui->accountCombo->clearEditText();
        QSignalBlocker categoryBlocker(d->ui->categoryCombo->lineEdit());
        d->ui->categoryCombo->clearEditText();
        d->updateWidgetAccess();

        const auto commodity = MyMoneyFile::instance()->currency(d->m_transaction.commodity());
        d->ui->creditDebitEdit->setCommodity(commodity);

    } else {
        // existing schedule
        d->m_transaction = schedule.transaction();
        d->m_split = d->m_transaction.splits().first();

        const auto commodity = MyMoneyFile::instance()->currency(d->m_transaction.commodity());
        d->ui->creditDebitEdit->setCommodity(commodity);

        // make sure the commodity is the one of the current account
        // in case we have exactly two splits. This is a precondition
        // used by the transaction editor to work properly.
        auto amountValue = d->m_split.value();
        if (d->m_transaction.splitCount() == 2) {
            amountValue = d->m_split.shares();
            d->m_split.setValue(amountValue);
        }

        // preset the value to be used for the amount widget
        auto amountShares = d->m_split.shares();

        for (const auto& split : d->m_transaction.splits()) {
            if (split.id() == d->m_split.id()) {
                d->ui->dateEdit->setDate(d->m_transaction.postDate());

                const auto payeeId = split.payeeId();
                const QModelIndex payeeIdx = MyMoneyFile::instance()->payeesModel()->indexById(payeeId);
                if (payeeIdx.isValid()) {
                    d->ui->payeeEdit->setCurrentIndex(MyMoneyFile::baseModel()->mapFromBaseSource(d->payeesModel, payeeIdx).row());
                } else {
                    d->ui->payeeEdit->setCurrentIndex(0);
                }

                d->ui->memoEdit->clear();
                d->ui->memoEdit->insertPlainText(split.memo());
                d->ui->memoEdit->moveCursor(QTextCursor::Start);
                d->ui->memoEdit->ensureCursorVisible();

                d->ui->numberEdit->setText(split.number());
                d->ui->statusCombo->setCurrentIndex(static_cast<int>(split.reconcileFlag()));
                d->ui->tagContainer->loadTags(split.tagIdList());
            } else {
                // we block sending out signals for the category combo here to avoid
                // calling NewTransactionEditorPrivate::categoryChanged which does not
                // work properly when loading the editor
                QSignalBlocker categoryComboBlocker(d->ui->categoryCombo);
                d->splitModel.appendSplit(split);
                if (d->m_transaction.splitCount() == 2) {
                    // force the value of the second split to be the same as for the first
                    const auto idx = d->splitModel.index(0, 0);
                    d->splitModel.setData(idx, QVariant::fromValue<MyMoneyMoney>(-amountValue), eMyMoney::Model::SplitValueRole);

                    // use the shares based on the second split
                    amountShares = -split.shares();

                    // adjust the commodity for the shares
                    const auto accountId = split.accountId();
                    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
                    const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);
                    d->ui->creditDebitEdit->setSharesCommodity(currency);
                }
            }
        }
        d->m_transaction.setCommodity(d->m_account.currencyId());

        // then setup the amount widget and update the state
        // of all other widgets
        d->ui->creditDebitEdit->setValue(amountValue);
        d->ui->creditDebitEdit->setShares(amountShares);
        d->updateWidgetState();
    }
}

void NewTransactionEditor::loadTransaction(const QModelIndex& index)
{
    // we may also get here during saving the transaction as
    // a callback from the model, but we can safely ignore it
    if (d->accepted || !index.isValid())
        return;

    auto idx = MyMoneyFile::baseModel()->mapToBaseSource(index);
    const auto commodity = MyMoneyFile::instance()->currency(d->m_account.currencyId());
    // set both the commodities to be the same here, in case of a two split transaction
    // and the other one being in a different commodity, we adjust that later on
    d->ui->creditDebitEdit->setCommodity(commodity);

    if (idx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        d->m_transaction = MyMoneyTransaction();
        d->m_transaction.setCommodity(commodity.id());

        d->m_split = MyMoneySplit();
        d->m_split.setAccountId(d->m_account.id());
        const auto lastUsedPostDate = KMyMoneySettings::lastUsedPostDate();
        if (lastUsedPostDate.isValid()) {
            d->ui->dateEdit->setDate(lastUsedPostDate.date());
        } else {
            d->ui->dateEdit->setDate(QDate::currentDate());
        }
        QSignalBlocker accountBlocker(d->ui->accountCombo->lineEdit());
        d->ui->accountCombo->clearEditText();
        QSignalBlocker categoryBlocker(d->ui->categoryCombo->lineEdit());
        d->ui->categoryCombo->clearEditText();

        d->ui->creditDebitEdit->setSharesCommodity(commodity);
        // the default exchange rate is 1 so we don't need to set it here

    } else {
        // find which item has this id and set is as the current item
        const auto selectedSplitRow = idx.row();

        // keep a copy of the transaction and split
        d->m_transaction = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).transaction();
        d->m_split = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).split();
        const auto list = idx.model()->match(idx.model()->index(0, 0), eMyMoney::Model::JournalTransactionIdRole,
                                             idx.data(eMyMoney::Model::JournalTransactionIdRole),
                                             -1,                         // all splits
                                             Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

        // make sure the commodity is the one of the current account
        // in case we have exactly two splits. This is a precondition
        // used by the transaction editor to work properly.
        auto amountValue = d->m_split.value();
        if (d->m_transaction.splitCount() == 2) {
            amountValue = d->m_split.shares();
            d->m_split.setValue(amountValue);
        }

        // preset the value to be used for the amount widget
        auto amountShares = d->m_split.shares();

        for (const auto& splitIdx : list) {
            if (selectedSplitRow == splitIdx.row()) {
                d->ui->dateEdit->setDate(splitIdx.data(eMyMoney::Model::TransactionPostDateRole).toDate());

                const auto payeeId = splitIdx.data(eMyMoney::Model::SplitPayeeIdRole).toString();
                const QModelIndex payeeIdx = MyMoneyFile::instance()->payeesModel()->indexById(payeeId);
                if (payeeIdx.isValid()) {
                    d->ui->payeeEdit->setCurrentIndex(MyMoneyFile::baseModel()->mapFromBaseSource(d->payeesModel, payeeIdx).row());
                } else {
                    d->ui->payeeEdit->setCurrentIndex(0);
                }

                d->ui->memoEdit->clear();
                d->ui->memoEdit->insertPlainText(splitIdx.data(eMyMoney::Model::SplitMemoRole).toString());
                d->ui->memoEdit->moveCursor(QTextCursor::Start);
                d->ui->memoEdit->ensureCursorVisible();

                d->ui->numberEdit->setText(splitIdx.data(eMyMoney::Model::SplitNumberRole).toString());
                d->ui->statusCombo->setCurrentIndex(splitIdx.data(eMyMoney::Model::SplitReconcileFlagRole).toInt());
                d->ui->tagContainer->loadTags(splitIdx.data(eMyMoney::Model::SplitTagIdRole).toStringList());
            } else {
                // we block sending out signals for the category combo here to avoid
                // calling NewTransactionEditorPrivate::categoryChanged which does not
                // work properly when loading the editor
                QSignalBlocker categoryComboBlocker(d->ui->categoryCombo);
                d->splitModel.appendSplit(MyMoneyFile::instance()->journalModel()->itemByIndex(splitIdx).split());
                if (splitIdx.data(eMyMoney::Model::TransactionSplitCountRole) == 2) {
                    // force the value of the second split to be the same as for the first
                    idx = d->splitModel.index(0, 0);
                    d->splitModel.setData(idx, QVariant::fromValue<MyMoneyMoney>(-amountValue), eMyMoney::Model::SplitValueRole);

                    // use the shares based on the second split
                    amountShares = -(splitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());

                    // adjust the commodity for the shares
                    const auto accountId = splitIdx.data(eMyMoney::Model::SplitAccountIdRole).toString();
                    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
                    const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);
                    d->ui->creditDebitEdit->setSharesCommodity(currency);
                }
            }
        }
        d->m_transaction.setCommodity(d->m_account.currencyId());

        // then setup the amount widget and update the state
        // of all other widgets
        d->ui->creditDebitEdit->setValue(amountValue);
        d->ui->creditDebitEdit->setShares(amountShares);
        d->updateWidgetState();
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


void NewTransactionEditor::editSplits()
{
    d->editSplits() == QDialog::Accepted ? acceptEdit() : reject();
}

MyMoneyMoney NewTransactionEditor::transactionAmount() const
{
    auto amount = -d->splitsSum();
    if (amount.isZero()) {
        amount = d->ui->creditDebitEdit->value();
    }
    return amount;
}

MyMoneyTransaction NewTransactionEditor::transaction() const
{
    MyMoneyTransaction t;

    if (!d->m_transaction.id().isEmpty()) {
        t = d->m_transaction;
    } else {
        // we keep the date when adding a new transaction
        // for the next new one
        KMyMoneySettings::setLastUsedPostDate(QDateTime(d->ui->dateEdit->date()));
    }

    // first remove the splits that are gone
    for (const auto& split : t.splits()) {
        if (split.id() == d->m_split.id()) {
            continue;
        }
        const auto rows = d->splitModel.rowCount();
        int row;
        for (row = 0; row < rows; ++row) {
            const QModelIndex index = d->splitModel.index(row, 0);
            if (index.data(eMyMoney::Model::IdRole).toString() == split.id()) {
                break;
            }
        }

        // if the split is not in the model, we get rid of it
        if (d->splitModel.rowCount() == row) {
            t.removeSplit(split);
        }
    }

    // now we update the split we are opened for
    MyMoneySplit sp(d->m_split);

    // in case the transaction does not have a split
    // at this point, we need to make sure that we
    // add the first one and don't try to modify it
    // we do so by clearing its id
    if (t.splitCount() == 0) {
        sp.clearId();
    }

    sp.setNumber(d->ui->numberEdit->text());
    sp.setMemo(d->ui->memoEdit->toPlainText());
    // setting up the shares and value members. In case there is
    // no or more than two splits, we can take the amount shown
    // in the widgets directly. In case of 2 splits, we take
    // the negative value of the second split (the one in the
    // splitModel) and use it as value and shares since the
    // displayed value in the widget may be shown in a different
    // currency
    if (d->splitModel.rowCount() == 1) {
        const QModelIndex idx = d->splitModel.index(0, 0);
        const auto val = idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
        sp.setShares(-val);
        sp.setValue(-val);
    } else {
        sp.setShares(d->ui->creditDebitEdit->value());
        sp.setValue(d->ui->creditDebitEdit->value());
    }

    if (sp.reconcileFlag() != eMyMoney::Split::State::Reconciled && !sp.reconcileDate().isValid()
        && d->ui->statusCombo->currentIndex() == (int)eMyMoney::Split::State::Reconciled) {
        sp.setReconcileDate(QDate::currentDate());
    }

    sp.setReconcileFlag(static_cast<eMyMoney::Split::State>(d->ui->statusCombo->currentIndex()));

    const auto payeeRow = d->ui->payeeEdit->currentIndex();
    const auto payeeIdx = d->payeesModel->index(payeeRow, 0);
    sp.setPayeeId(payeeIdx.data(eMyMoney::Model::IdRole).toString());
    sp.setTagIdList(d->ui->tagContainer->selectedTags());

    if (sp.id().isEmpty()) {
        t.addSplit(sp);
    } else {
        t.modifySplit(sp);
    }
    t.setPostDate(d->ui->dateEdit->date());

    // now update and add what we have in the model
    d->splitModel.addSplitsToTransaction(t);

    return t;
}

void NewTransactionEditor::saveTransaction()
{
    auto t = transaction();

    MyMoneyFileTransaction ft;
    try {
        if (t.id().isEmpty()) {
            MyMoneyFile::instance()->addTransaction(t);
        } else {
            MyMoneyFile::instance()->modifyTransaction(t);
        }
        ft.commit();

    } catch (const MyMoneyException& e) {
        qDebug() << Q_FUNC_INFO << "something went wrong" << e.what();
    }
}

bool NewTransactionEditor::eventFilter(QObject* o, QEvent* e)
{
    auto cb = qobject_cast<QComboBox*>(o);
    if (o) {
        // filter out wheel events for combo boxes if the popup view is not visible
        if ((e->type() == QEvent::Wheel) && !cb->view()->isVisible()) {
            return true;
        }
    }
    return QWidget::eventFilter(o, e);
}

QDate NewTransactionEditor::postDate() const
{
    return d->ui->dateEdit->date();
}

void NewTransactionEditor::setShowAccountCombo(bool show) const
{
    d->ui->accountLabel->setVisible(show);
    d->ui->accountCombo->setVisible(show);
    d->ui->accountCombo->setSplitActionVisible(false);
}

void NewTransactionEditor::setShowButtons(bool show) const
{
    d->ui->enterButton->setVisible(show);
    d->ui->cancelButton->setVisible(show);
}

void NewTransactionEditor::setShowNumberWidget(bool show) const
{
    d->ui->numberLabel->setVisible(show);
    d->ui->numberEdit->setVisible(show);
}

void NewTransactionEditor::setAccountId(const QString& accountId)
{
    d->ui->accountCombo->setSelected(accountId);
}

void NewTransactionEditor::setReadOnly(bool readOnly)
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

void NewTransactionEditor::setupUi(QWidget* parent)
{
    if (d->tabOrderUi == nullptr) {
        d->tabOrderUi = new Ui::NewTransactionEditor;
    }
    d->tabOrderUi->setupUi(parent);
    d->tabOrderUi->accountLabel->setVisible(false);
    d->tabOrderUi->accountCombo->setVisible(false);
}

void NewTransactionEditor::storeTabOrder(const QStringList& tabOrder)
{
    TransactionEditorBase::storeTabOrder(QLatin1String("stdTransactionEditor"), tabOrder);
}
