/*
    SPDX-FileCopyrightText: 2015-2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newtransactioneditor.h"
#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractItemView>
#include <QCompleter>
#include <QDebug>
#include <QGlobalStatic>
#include <QHeaderView>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringList>
#include <QTableView>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "costcentermodel.h"
#include "icons.h"
#include "idfilter.h"
#include "journalmodel.h"
#include "kmmyesno.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "knewaccountdlg.h"
#include "ktransactionselectdlg.h"
#include "mymoneyaccount.h"
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
#include "taborder.h"
#include "tagsmodel.h"
#include "widgethintframe.h"

#include "ui_newtransactioneditor.h"

using namespace Icons;

class NewTransactionEditor::Private : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Private)

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
        , costCenterRequired(false)
        , inUpdateVat(false)
        , keepCategoryAmount(false)
        , loadedFromModel(false)
        , counterAccountIsClosed(false)
        , splitModel(parent, &undoStack)
        , m_splitHelper(nullptr)
        , m_externalTabOrder(nullptr)
        , m_tabOrder(QLatin1String("stdTransactionEditor"),
                     QStringList{
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
                     })
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
    bool checkForValidTransaction(bool doUserInteraction = true);
    bool isDatePostOpeningDate(const QDate& date, const QString& accountId);
    bool postdateChanged(const QDate& date);
    bool costCenterChanged(int costCenterIndex);
    void payeeChanged(int payeeIndex);
    Q_SLOT void autoFillTransaction(const QString& payeeId);
    void accountChanged(const QString& id);
    bool categoryChanged(const QString& id);
    bool numberChanged(const QString& newNumber);
    bool amountChanged();
    bool checkForValidAmount();
    bool isIncomeExpense(const QModelIndex& idx) const;
    bool isIncomeExpense(const QString& categoryId) const;
    bool tagsChanged(const QStringList& ids);
    int editSplits();
    void updateWidgetAccess();
    void updateVAT(TaxValueChange amountChanged);
    MyMoneyMoney removeVatSplit();
    MyMoneyMoney splitsSum() const;
    void defaultCategoryAssignment();
    void loadTransaction(QModelIndex idx);
    MyMoneySplit prepareSplit(const MyMoneySplit& sp);
    bool needClearSplitAction(const QString& action) const;
    void updateMemoLink();

    NewTransactionEditor* q;
    Ui_NewTransactionEditor* ui;
    Ui_NewTransactionEditor* tabOrderUi;
    AccountNamesFilterProxyModel* accountsModel;
    AccountNamesFilterProxyModel* categoriesModel;
    QSortFilterProxyModel* costCenterModel;
    QSortFilterProxyModel* payeesModel;
    bool costCenterRequired;
    bool inUpdateVat;
    bool keepCategoryAmount;
    bool loadedFromModel;
    bool counterAccountIsClosed;
    QUndoStack undoStack;
    SplitModel splitModel;
    MyMoneyAccount m_account;
    MyMoneyTransaction m_transaction;
    MyMoneySplit m_split;
    KMyMoneyAccountComboSplitHelper* m_splitHelper;
    TabOrder* m_externalTabOrder;
    TabOrder m_tabOrder;
};

void NewTransactionEditor::Private::updateWidgetAccess()
{
    const auto enable = !m_account.id().isEmpty();
    ui->dateEdit->setEnabled(!counterAccountIsClosed);
    ui->creditDebitEdit->setEnabled(!counterAccountIsClosed || splitModel.rowCount() > 1);
    ui->payeeEdit->setEnabled(enable);
    ui->numberEdit->setEnabled(enable);
    ui->categoryCombo->setEnabled(enable);
    ui->costCenterCombo->setEnabled(enable);
    ui->tagContainer->setEnabled(enable & (splitModel.rowCount() == 1));
    ui->statusCombo->setEnabled(enable);
    ui->memoEdit->setEnabled(enable);
    ui->enterButton->setEnabled(!q->isReadOnly());

    m_splitHelper->setProtectAccountCombo(counterAccountIsClosed);

    if (counterAccountIsClosed) {
        const auto tip = i18nc("@info:tooltip Protected", "This widget is currently protected because the transaction references a closed account.");
        ui->categoryCombo->setToolTip(tip);
        ui->creditDebitEdit->setToolTip(tip);
    } else {
        ui->categoryCombo->setToolTip(QString());
        ui->creditDebitEdit->setToolTip(QString());
    }
}

void NewTransactionEditor::Private::updateWidgetState()
{
    auto index = splitModel.index(0, 0);

    // update the tag combo box
    if (splitModel.rowCount() == 1) {
        ui->tagContainer->setEnabled(true);
        ui->tagContainer->loadTags(index.data(eMyMoney::Model::SplitTagIdRole).toStringList());
    } else {
        ui->tagContainer->setEnabled(false);
        ui->tagContainer->loadTags({});
    }

#ifndef ENABLE_COSTCENTER
    if (!index.data(eMyMoney::Model::SplitCostCenterIdRole).toString().isEmpty()) {
        // in case the cost center widgets are disabled by default, we still need
        // to make them available when we need them due to data found in the engine
        ui->costCenterCombo->show();
        ui->costCenterLabel->show();
    }
#endif

    // update the costcenter combo box
    if (ui->costCenterCombo->isEnabled()) {
        // extract the cost center
        index = MyMoneyFile::instance()->costCenterModel()->indexById(index.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
        if (index.isValid()) {
            const auto row = MyMoneyModelBase::mapFromBaseSource(costCenterModel, index).row();
            ui->costCenterCombo->setCurrentIndex(row);
        }
    }
}

bool NewTransactionEditor::Private::checkForValidAmount()
{
    WidgetHintFrame::hide(ui->creditDebitEdit);
    WidgetHintFrame::hide(ui->categoryCombo);
    const auto difference = (q->transactionAmount() - (-splitsSum())).abs();
    if (!difference.isZero()) {
        if (splitModel.rowCount() == 0) {
            WidgetHintFrame::show(ui->categoryCombo, i18nc("@info:tooltip", "The transaction is missing a category assignment."));
        } else {
            WidgetHintFrame::show(
                ui->creditDebitEdit,
                i18nc("@info:tooltip", "The amount entered is different from the sum of all splits by %1.", difference.formatMoney(m_account.fraction())));
        }
    }
    return true;
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

    if (q->needCreateCategory(ui->categoryCombo) || q->needCreatePayee(ui->payeeEdit)) {
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
    } catch (MyMoneyException&) {
        qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
    }
    return rc;
}

/**
 * Check that the postdate is valid and that all referenced
 * account's opening date is prior to the postdate. Return
 * @a true if all conditions are met.
 */
bool NewTransactionEditor::Private::postdateChanged(const QDate& date)
{
    WidgetHintFrame::hide(ui->dateEdit);

    if (!date.isValid()) {
        WidgetHintFrame::show(ui->dateEdit, i18n("The posting date is invalid."));
        return false;
    }

    // collect all account ids
    QStringList accountIds;
    accountIds << m_account.id();
    const auto rows = splitModel.rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto index = splitModel.index(row, 0);
        accountIds << index.data(eMyMoney::Model::SplitAccountIdRole).toString();
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

/**
 * Check that the cost center information is filled when
 * required for the category and update the first split
 * of a normal transaction with the id of the selected
 * cost center. Returns @a true if cost center assignment
 * is correct.
 */
bool NewTransactionEditor::Private::costCenterChanged(int costCenterIndex)
{
    bool rc = true;
    WidgetHintFrame::hide(ui->costCenterCombo);
    if (costCenterIndex != -1) {
        if (costCenterRequired && ui->costCenterCombo->currentText().isEmpty()) {
#ifndef ENABLE_COSTCENTER
            // in case the cost center widgets are disabled by default, we still need
            // to make them available when we need them due to data found in the engine
            ui->costCenterCombo->show();
            ui->costCenterLabel->show();
#endif
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

    // in case we have a single split, we set the categoryCombo again
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
                    // add a first split with account assigned
                    MyMoneySplit s;
                    s.setAccountId(accountId);
                    // the following call does not assign a split ID
                    // this will be done in SplitModel::addSplitsToTransaction()
                    splitModel.appendSplit(s);
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
                    // switch to value display so that we show the transaction commodity
                    // for single currency data entry this does not have an effect
                    ui->creditDebitEdit->setDisplayState(MultiCurrencyEdit::DisplayValue);
                    ui->creditDebitEdit->setSharesCommodity(currency);
                    auto sharesAmount = ui->creditDebitEdit->value();
                    ui->creditDebitEdit->setShares(sharesAmount);

                    if (!sharesAmount.isZero()) {
                        q->updateConversionRate(ui->creditDebitEdit);
                    }
                }

                splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->value()), eMyMoney::Model::SplitValueRole);
                splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->shares()), eMyMoney::Model::SplitSharesRole);

                updateVAT(ValueUnchanged);

                keepCategoryAmount = false;

            } catch (MyMoneyException&) {
                qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
            }
        } else {
            splitModel.unload();
        }
    }
    checkForValidAmount();
    ui->tagContainer->setEnabled(splitModel.rowCount() == 1);
    return rc;
}

bool NewTransactionEditor::Private::numberChanged(const QString& newNumber)
{
    bool rc = true; // number did change
    WidgetHintFrame::hide(ui->numberEdit);
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
        try {
            if (splitModel.rowCount() == 1) {
                const QModelIndex index = splitModel.index(0, 0);

                if (!keepCategoryAmount) {
                    // check if there is a change in the values other than simply reverting the sign
                    // and get an updated price in that case
                    if ((index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>() != ui->creditDebitEdit->shares())
                        || (index.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>() != ui->creditDebitEdit->value())) {
                        q->updateConversionRate(ui->creditDebitEdit);
                    }

                    splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->shares()), eMyMoney::Model::SplitSharesRole);
                }
                splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-ui->creditDebitEdit->value()), eMyMoney::Model::SplitValueRole);
            }

        } catch (MyMoneyException&) {
            rc = false;
            qDebug() << "Ooops: something went wrong in" << Q_FUNC_INFO;
        }
    } else {
        /// @todo ask what to do: if the rest of the splits is the same amount we could simply reverse the sign
        /// of all splits, otherwise we could ask if the user wants to start the split editor or anything else.
    }
    updateVAT(ValueChanged);
    checkForValidAmount();
    return rc;
}

void NewTransactionEditor::Private::payeeChanged(int payeeIndex)
{
    const auto payeeId = payeesModel->index(payeeIndex, 0).data(eMyMoney::Model::IdRole).toString();
    const AutoFillMethod autoFillMethod = static_cast<AutoFillMethod>(KMyMoneySettings::autoFillTransaction());

    // we have a new payee assigned to this transaction.
    // in case there is no category assigned for a new transaction,
    // we search for the last transaction of this payee
    // in the selected account.
    if (m_transaction.id().isEmpty() && (splitModel.rowCount() == 0) && (autoFillMethod != AutoFillMethod::NoAutoFill)) {
        // if we got here, we have to autofill. Because we will open a dialog,
        // we simply postpone the call until we reach the main event loop. We
        // do this because running the dialog directly has some unknown influence
        // when controlled by the keyboard on the payee completer object which
        // we can avoid by postponing.
        QMetaObject::invokeMethod(this, "autoFillTransaction", Qt::QueuedConnection, Q_ARG(QString, payeeId));

    } else {
        // copy payee information to second split if there are only two splits
        if (splitModel.rowCount() == 1) {
            const auto idx = splitModel.index(0, 0);
            splitModel.setData(idx, payeeId, eMyMoney::Model::SplitPayeeIdRole);
        }
    }
}

void NewTransactionEditor::Private::autoFillTransaction(const QString& payeeId)
{
    struct uniqTransaction {
        QString journalEntryId;
        MyMoneyMoney amount;
        int matches;
    };

    /**
     * Sum up all splits for the given account in the transaction
     */
    auto sumOfShares = [&](const MyMoneyTransaction& t, const QString& accountId) {
        MyMoneyMoney result;
        for (const auto& split : t.splits()) {
            if (split.accountId() == accountId) {
                result += split.shares();
            }
        }
        return result;
    };

    const auto journalModel = MyMoneyFile::instance()->journalModel();
    MyMoneyTransactionFilter filter(m_account.id());
    filter.addPayee(payeeId);
    QStringList journalEntryIds(journalModel->journalEntryIds(filter));

    if (!journalEntryIds.empty()) {
        const AutoFillMethod autoFillMethod = static_cast<AutoFillMethod>(KMyMoneySettings::autoFillTransaction());
        // ok, we found at least one previous transaction. now we clear out
        // what we have collected so far and add those splits from
        // the previous transaction.
        QMap<QString, struct uniqTransaction> uniqList;

        // collect the journal entries and see if we have any duplicates
        for (const auto& journalEntryId : journalEntryIds) {
            const auto journalEntry = journalModel->itemById(journalEntryId);
            const auto accountSignature = journalEntry.transaction().accountSignature();
            const auto sharesSum = sumOfShares(journalEntry.transaction(), m_account.id());
            int cnt = 0;
            QMap<QString, struct uniqTransaction>::iterator it_u;
            do {
                const QString ukey = QString("%1-%2").arg(accountSignature, cnt);
                it_u = uniqList.find(ukey);
                if (it_u == uniqList.end()) {
                    uniqList[ukey].journalEntryId = journalEntryId;
                    uniqList[ukey].amount = sumOfShares(journalEntry.transaction(), m_account.id());
                    uniqList[ukey].matches = 1;

                } else if (autoFillMethod == AutoFillMethod::AutoFillWithClosestInValue) {
                    // we already have a transaction with this signature. we must
                    // now check, if we should really treat it as a duplicate according
                    // to the value comparison delta.
                    MyMoneyMoney s1 = (*it_u).amount;
                    MyMoneyMoney s2 = sharesSum;
                    if (s2.abs() > s1.abs()) {
                        MyMoneyMoney t(s1);
                        s1 = s2;
                        s2 = t;
                    }
                    MyMoneyMoney diff;
                    if (s2.isZero()) {
                        diff = s1.abs();
                    } else {
                        diff = ((s1 - s2) / s2).convert(10000);
                    }
                    if (diff.isPositive() && diff <= MyMoneyMoney(KMyMoneySettings::autoFillDifference(), 100)) {
                        uniqList[ukey].journalEntryId = journalEntryId;
                        uniqList[ukey].amount = sumOfShares(journalEntry.transaction(), m_account.id());
                        break; // end while loop
                    }
                } else if (autoFillMethod == AutoFillMethod::AutoFillWithMostOftenUsed) {
                    uniqList[ukey].journalEntryId = journalEntryId;
                    uniqList[ukey].amount = sumOfShares(journalEntry.transaction(), m_account.id());
                    (*it_u).matches++;
                    break; // end while loop
                }
                ++cnt;
            } while (it_u != uniqList.end());
        }

        QString journalEntryId;
        if (autoFillMethod != AutoFillMethod::AutoFillWithMostOftenUsed) {
            QPointer<KTransactionSelectDlg> dlg = new KTransactionSelectDlg();
            dlg->setWindowTitle(i18nc("@title:window Autofill selection dialog", "Select autofill transaction"));

            QMap<QString, struct uniqTransaction>::const_iterator it_u;
            QStringList journalEntryIdCollection;
            for (it_u = uniqList.cbegin(); it_u != uniqList.cend(); ++it_u) {
                journalEntryIdCollection << (*it_u).journalEntryId;
            }
            dlg->addTransactions(journalEntryIdCollection);

            // Sort by
            // - ascending post date
            // - descending reconciliation state
            // - descending value
            dlg->ledgerView()->setSortOrder(LedgerSortOrder("1,-9,-4"));
            dlg->ledgerView()->selectMostRecentTransaction();
            if (dlg->exec() == QDialog::Accepted) {
                journalEntryId = dlg->journalEntryId();
            }
        } else {
            int maxCnt = 0;
            QMap<QString, struct uniqTransaction>::const_iterator it_u;
            for (it_u = uniqList.cbegin(); it_u != uniqList.cend(); ++it_u) {
                if ((*it_u).matches > maxCnt) {
                    journalEntryId = (*it_u).journalEntryId;
                    maxCnt = (*it_u).matches;
                }
            }
        }

        if (!journalEntryId.isEmpty()) {
            // keep data we don't want to change by loading
            const auto postDate = ui->dateEdit->date();
            const auto number = ui->numberEdit->text();
            const auto amountFilled = ui->creditDebitEdit->haveValue();
            const auto amount = ui->creditDebitEdit->value();
            const auto memo = ui->memoEdit->toPlainText();

            // now load the existing transaction into the editor
            auto index = MyMoneyFile::instance()->journalModel()->indexById(journalEntryId);
            loadTransaction(index);

            // make sure to really create a new transaction
            m_transaction.clearId();
            ui->statusCombo->setCurrentIndex(static_cast<int>(eMyMoney::Split::State::NotReconciled));
            m_split = prepareSplit(m_split);

            splitModel.resetAllSplitIds();
            for (int row = 0; row < splitModel.rowCount(); ++row) {
                const auto idx = splitModel.index(row, 0);
                splitModel.setData(idx, QVariant::fromValue(eMyMoney::Split::State::NotReconciled), eMyMoney::Model::SplitReconcileFlagRole);
                splitModel.setData(idx, QDate(), eMyMoney::Model::SplitReconcileDateRole);
                splitModel.setData(idx, QString(), eMyMoney::Model::SplitBankIdRole);
                splitModel.setData(idx, QString(), eMyMoney::Model::SplitMemoRole);

                if (needClearSplitAction(idx.data(eMyMoney::Model::SplitActionRole).toString())) {
                    splitModel.setData(idx, QString(), eMyMoney::Model::SplitActionRole);
                }
                // copy payee information to second split if there are only two splits
                // overwrite amount in split if value was already available
                if (splitModel.rowCount() == 1) {
                    splitModel.setData(idx, payeeId, eMyMoney::Model::SplitPayeeIdRole);
                    if (!memo.isEmpty()) {
                        splitModel.setData(idx, memo, eMyMoney::Model::SplitMemoRole);
                    }
                    if (amountFilled) {
                        const auto value = idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
                        const auto shares = idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                        auto price = MyMoneyMoney::ONE;
                        if (!shares.isZero()) {
                            price = value / shares;
                        }
                        splitModel.setData(idx, QVariant::fromValue<MyMoneyMoney>(-amount), eMyMoney::Model::SplitValueRole);
                        splitModel.setData(idx, QVariant::fromValue<MyMoneyMoney>((-amount / price).convert()), eMyMoney::Model::SplitSharesRole);
                    }
                }
            }
            // restore data we don't want to change by loading
            ui->dateEdit->setDate(postDate);

            if (ui->numberEdit->isVisible() && !number.isEmpty()) {
                ui->numberEdit->setText(number);
            } else if (!m_split.number().isEmpty()) {
                ui->numberEdit->setText(KMyMoneyUtils::nextFreeCheckNumber(m_account));
            }

            // if the user already entered an amount we use it to proceed
            if (amountFilled) {
                ui->creditDebitEdit->setValue(amount);
            }

            // if the user already entered a memo we use it
            if (!memo.isEmpty()) {
                ui->memoEdit->setPlainText(memo);
            }

            updateWidgetState();
            updateWidgetAccess();
            checkForValidAmount();
        }
    }

    /// @todo maybe set focus to next tab widget
}

MyMoneySplit NewTransactionEditor::Private::prepareSplit(const MyMoneySplit& sp)
{
    auto split(sp);
    split.setReconcileDate(QDate());
    split.setBankID(QString());
    // older versions of KMyMoney used to set the action
    // we don't need this anymore
    if (needClearSplitAction(split.action())) {
        split.setAction(QString());
    }
    split.setNumber(QString());
    if (!KMyMoneySettings::autoFillUseMemos()) {
        split.setMemo(QString());
    }

    return split;
}

bool NewTransactionEditor::Private::needClearSplitAction(const QString& action) const
{
    return (action != MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization) && action != MyMoneySplit::actionName(eMyMoney::Split::Action::Interest));
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
        updateWidgetAccess();
        checkForValidAmount();

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
    splitModel.removeRow(taxSplitIdx.row());

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
            *m_lockVariable = true;
        }
        ~cleanupHelper()
        {
            *m_lockVariable = false;
        }
        bool* m_lockVariable;
    } cleanupHelper(&inUpdateVat);

    auto categoryId = ui->categoryCombo->getSelected();

    auto taxCategoryId = [&]() {
        if (categoryId.isEmpty()) {
            return QString();
        }
        const auto category = MyMoneyFile::instance()->account(categoryId);
        return category.value(QLatin1String("VatAccount"));
    };

    // if auto vat assignment for this account is turned off
    // we don't care about taxes
    if (m_account.value(QLatin1String("NoVat")).toLower() == QLatin1String("yes"))
        return;

    // more splits than category and tax are not supported
    if (splitModel.rowCount() > 2) {
        return;
    }

    // in order to do anything, we need an amount
    MyMoneyMoney amount, newAmount;
    amount = ui->creditDebitEdit->value();
    if (amount.isZero()) {
        return;
    }

    // If the transaction has a tax and a category split, remove the tax split
    if (splitModel.rowCount() == 2) {
        newAmount = removeVatSplit();
        if (splitModel.rowCount() == 2) { // not removed?
            return;
        }

        // categoryId can be empty in case we update the amount
        // of an already existing split transaction and the VAT
        // split has just been removed. In that case, the splitModel
        // contains a single row from which we can extract the
        // categoryId and continue.
        if (categoryId.isEmpty()) {
            categoryId = splitModel.index(0, 0).data(eMyMoney::Model::SplitAccountIdRole).toString();
        }

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

    // to be able to assign a tax split, the only thing
    // we should have is the split assigned to the category.
    // if we have anything else, bail out
    if (splitModel.rowCount() != 1) {
        return;
    }

    // create a transaction with a single split
    auto t = q->transaction();
    t.setCommodity(m_transaction.commodity());

    // and add the VAT part
    MyMoneyFile::instance()->updateVAT(t);

    // keep the split model in sync with the new data
    splitModel.unload();
    for (const auto& split : t.splits()) {
        if ((split.accountId() == taxId) || split.accountId() == categoryId) {
            splitModel.appendSplit(split);
        }
    }
}

void NewTransactionEditor::Private::defaultCategoryAssignment()
{
    if (splitModel.rowCount() == 0) {
        const auto payeeIdx = payeesModel->index(ui->payeeEdit->currentIndex(), 0);
        const auto defaultAccount = payeeIdx.data(eMyMoney::Model::PayeeDefaultAccountRole).toString();
        if (!defaultAccount.isEmpty()) {
            categoryChanged(defaultAccount);
        }
    }
}

/**
 * @note @a idx must be the base model index
 */
void NewTransactionEditor::Private::loadTransaction(QModelIndex idx)
{
    // we block sending out signals for the account and category combo here
    // to avoid calling NewTransactionEditorPrivate::categoryChanged which
    // does not work properly when loading the editor
    QSignalBlocker accountBlocker(ui->accountCombo->lineEdit());
    ui->accountCombo->clearEditText();
    QSignalBlocker categoryBlocker(ui->categoryCombo->lineEdit());
    ui->categoryCombo->clearEditText();

    // find which item has this id and set is as the current item
    const auto selectedSplitRow = idx.row();

    // keep a copy of the transaction and split
    m_transaction = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).transaction();
    m_split = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).split();
    const auto list = idx.model()->match(idx.model()->index(0, 0),
                                         eMyMoney::Model::JournalTransactionIdRole,
                                         idx.data(eMyMoney::Model::JournalTransactionIdRole),
                                         -1, // all splits
                                         Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

    // make sure the commodity is the one of the current account
    // in case we have exactly two splits. This is a precondition
    // used by the transaction editor to work properly.
    auto amountValue = m_split.value();
    if (m_transaction.splitCount() == 2) {
        amountValue = m_split.shares();
        m_split.setValue(amountValue);
    }

    // preset the value to be used for the amount widget
    auto amountShares = m_split.shares();

    // block the signals sent out from the model here so that
    // connected widgets don't overwrite the values we just loaded
    // because they are not yet set (d->ui->creditDebitEdit)
    QSignalBlocker blocker(splitModel);

    // make sure that the split model is empty when a new
    // transaction is loaded
    splitModel.unload();

    for (const auto& splitIdx : list) {
        if (selectedSplitRow == splitIdx.row()) {
            ui->dateEdit->setDate(splitIdx.data(eMyMoney::Model::TransactionPostDateRole).toDate());

            const auto payeeId = splitIdx.data(eMyMoney::Model::SplitPayeeIdRole).toString();
            const QModelIndex payeeIdx = MyMoneyFile::instance()->payeesModel()->indexById(payeeId);
            if (payeeIdx.isValid()) {
                ui->payeeEdit->setCurrentIndex(MyMoneyFile::baseModel()->mapFromBaseSource(payeesModel, payeeIdx).row());
            } else {
                ui->payeeEdit->setCurrentIndex(0);
            }

            ui->memoEdit->clear();
            ui->memoEdit->insertPlainText(splitIdx.data(eMyMoney::Model::SplitMemoRole).toString());
            ui->memoEdit->moveCursor(QTextCursor::Start);
            ui->memoEdit->ensureCursorVisible();

            ui->numberEdit->setText(splitIdx.data(eMyMoney::Model::SplitNumberRole).toString());
            ui->statusCombo->setCurrentIndex(splitIdx.data(eMyMoney::Model::SplitReconcileFlagRole).toInt());
            updateMemoLink();
        } else {
            splitModel.appendSplit(MyMoneyFile::instance()->journalModel()->itemByIndex(splitIdx).split());

            if (splitIdx.data(eMyMoney::Model::TransactionSplitCountRole) == 2) {
                // force the value of the second split to be the same as for the first
                idx = splitModel.index(0, 0);
                splitModel.setData(idx, QVariant::fromValue<MyMoneyMoney>(-amountValue), eMyMoney::Model::SplitValueRole);

                // use the shares based on the second split
                amountShares = -(splitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());

                // adjust the commodity for the shares
                const auto accountId = splitIdx.data(eMyMoney::Model::SplitAccountIdRole).toString();
                const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
                const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);
                ui->creditDebitEdit->setSharesCommodity(currency);

                counterAccountIsClosed = accountIdx.data(eMyMoney::Model::AccountIsClosedRole).toBool();
                ui->tagContainer->loadTags(splitIdx.data(eMyMoney::Model::SplitTagIdRole).toStringList());
            }
        }
    }
    m_transaction.setCommodity(m_account.currencyId());

    // then setup the amount widget and update the state
    // of all other widgets
    ui->creditDebitEdit->setValue(amountValue);
    ui->creditDebitEdit->setShares(amountShares);

    updateWidgetState();
    updateWidgetAccess();
    checkForValidAmount();

    m_splitHelper->updateWidget();
}

void NewTransactionEditor::Private::updateMemoLink()
{
    try {
        const MyMoneyPayee& payeeObj = MyMoneyFile::instance()->payeeByName(ui->payeeEdit->currentText());
        QUrl url = payeeObj.payeeLink(ui->memoEdit->toPlainText());
        if (url.isEmpty()) {
            ui->linkLabel->setText("");
            return;
        }
        ui->linkLabel->setTextFormat(Qt::RichText);
        ui->linkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->linkLabel->setOpenExternalLinks(true);
        ui->linkLabel->setText(QString("<a href=\"%1\">%2</a>").arg(url.toString(), i18n("Link")));
        qDebug() << url;
    } catch (MyMoneyException&) {
        ui->linkLabel->setText("");
    }
}

NewTransactionEditor::NewTransactionEditor(QWidget* parent, const QString& accountId)
    : TransactionEditorBase(parent, accountId)
    , MyMoneyFactory(this)
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

    d->m_tabOrder.setWidget(this);

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

    d->m_splitHelper = new KMyMoneyAccountComboSplitHelper(d->ui->categoryCombo, &d->splitModel);
    connect(d->m_splitHelper, &KMyMoneyAccountComboSplitHelper::accountComboEnabled, d->ui->costCenterCombo, &QComboBox::setEnabled);
    connect(d->m_splitHelper, &KMyMoneyAccountComboSplitHelper::accountComboEnabled, d->ui->costCenterLabel, &QComboBox::setEnabled);
    connect(d->m_splitHelper, &KMyMoneyAccountComboSplitHelper::accountComboEnabled, this, &NewTransactionEditor::categorySelectionChanged);

    d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type>{
        eMyMoney::Account::Type::Asset,
        eMyMoney::Account::Type::Liability,
        eMyMoney::Account::Type::Equity,
    });
    d->accountsModel->setHideEquityAccounts(false);
    d->accountsModel->setHideZeroBalancedEquityAccounts(false);
    d->accountsModel->setHideZeroBalancedAccounts(false);
    d->accountsModel->setShowAllEntries(KMyMoneySettings::showAllAccounts());
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
    d->categoriesModel->setShowAllEntries(KMyMoneySettings::showAllAccounts());
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

#ifndef ENABLE_COSTCENTER
    d->ui->costCenterCombo->hide();
    d->ui->costCenterLabel->hide();
#endif

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
    connect(d->ui->payeeEdit->lineEdit(), &QLineEdit::textEdited, this, [&](const QString& txt) {
        if (txt.isEmpty()) {
            d->ui->payeeEdit->setCurrentIndex(-1);
        }
    });

    connect(
        d->ui->payeeEdit->lineEdit(),
        &QLineEdit::textEdited,
        this,
        [&](const QString& txt) {
            if (!txt.isEmpty()) {
                // when the user types something, select the first entry in the popup
                const auto view = d->ui->payeeEdit->completer()->popup();
                const auto viewsModel = view->model();
                // prevent that setting the current index propagates the full
                // name into the edit widget
                QSignalBlocker blocker(view->selectionModel());
                view->setCurrentIndex(viewsModel->index(0, 0));
            }
        },
        Qt::QueuedConnection);

    connect(d->ui->categoryCombo->lineEdit(), &QLineEdit::textEdited, this, [&](const QString& txt) {
        if (txt.isEmpty()) {
            d->ui->categoryCombo->setSelected(QString());
        }
    });
    d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
    d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

    d->ui->statusCombo->setModel(MyMoneyFile::instance()->statusModel());

    d->ui->creditDebitEdit->setAllowEmpty(true);

    widgetHintFrameCollection()->addFrame(new WidgetHintFrame(d->ui->dateEdit));
    widgetHintFrameCollection()->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
    widgetHintFrameCollection()->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
    widgetHintFrameCollection()->addFrame(new WidgetHintFrame(d->ui->creditDebitEdit, WidgetHintFrame::Warning));
    widgetHintFrameCollection()->addFrame(new WidgetHintFrame(d->ui->categoryCombo, WidgetHintFrame::Warning));
    widgetHintFrameCollection()->addWidget(d->ui->enterButton);

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

    connect(d->ui->cancelButton, &QToolButton::clicked, this, &NewTransactionEditor::reject);
    connect(d->ui->enterButton, &QToolButton::clicked, this, &NewTransactionEditor::acceptEdit);

    // handle some events in certain conditions different from default
    d->ui->payeeEdit->installEventFilter(this);
    d->ui->costCenterCombo->installEventFilter(this);
    d->ui->tagContainer->tagCombo()->installEventFilter(this);
    d->ui->categoryCombo->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);
    d->ui->memoEdit->installEventFilter(this);

    setCancelButton(d->ui->cancelButton);
    setEnterButton(d->ui->enterButton);

    // force setup of filters
    slotSettingsChanged();
}

NewTransactionEditor::~NewTransactionEditor()
{
}

void NewTransactionEditor::setAmountPlaceHolderText(const QAbstractItemModel* model)
{
    d->ui->creditDebitEdit->setPlaceholderText(model->headerData(JournalModel::Column::Payment, Qt::Horizontal).toString(),
                                               model->headerData(JournalModel::Column::Deposit, Qt::Horizontal).toString());
}

void NewTransactionEditor::loadSchedule(const MyMoneySchedule& schedule, ScheduleEditType editType)
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
        QSignalBlocker accountBlocker(d->ui->accountCombo->lineEdit());
        d->ui->accountCombo->clearEditText();
        QSignalBlocker categoryBlocker(d->ui->categoryCombo->lineEdit());
        d->ui->categoryCombo->clearEditText();
        d->updateWidgetAccess();

        const auto commodity = MyMoneyFile::instance()->currency(d->m_transaction.commodity());
        d->ui->creditDebitEdit->setCommodity(commodity);

    } else {
        // existing schedule
        // since a scheduled transaction does not have an id, we assign it here so
        // that we can identify a transaction of an existing schedule. It will be
        // cleared when retrieving the transaction in the schedule editor via
        // KEditScheduleDlgPrivate::transaction()
        d->m_transaction = MyMoneyTransaction(schedule.id(), schedule.transaction());
        d->m_split = d->m_transaction.splits().first();

        const auto commodity = MyMoneyFile::instance()->currency(d->m_transaction.commodity());
        d->ui->creditDebitEdit->setCommodity(commodity);
        // update the commodity in case it was empty
        d->m_transaction.setCommodity(commodity.id());

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

        // block the signals sent out from the model here so that
        // connected widgets don't overwrite the values we just loaded
        // because they are not yet set (d->ui->creditDebitEdit)
        QSignalBlocker splitModelSignalBlocker(d->splitModel);

        // block the signals sent out from the payee edit widget so that
        // the autofill logic is not triggered when loading the schdule
        QSignalBlocker autoFillSignalBlocker(d->ui->payeeEdit);

        for (const auto& split : d->m_transaction.splits()) {
            if (split.id() == d->m_split.id()) {
                if (editType == EditSchedule) {
                    d->ui->dateEdit->setDate(d->m_transaction.postDate());
                } else {
                    d->ui->dateEdit->setDate(schedule.adjustedNextDueDate());
                }

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
                d->updateMemoLink();
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
        d->updateWidgetAccess();
        d->checkForValidAmount();

        d->m_splitHelper->updateWidget();
    }
}

void NewTransactionEditor::loadTransaction(const QModelIndex& index)
{
    // we may also get here during saving the transaction as
    // a callback from the model, but we can safely ignore it
    // same when we get called from the delegate's setEditorData()
    // method
    if (accepted() || !index.isValid() || d->loadedFromModel)
        return;

    d->loadedFromModel = true;

    auto idx = MyMoneyFile::baseModel()->mapToBaseSource(index);
    const auto commodity = MyMoneyFile::instance()->currency(d->m_account.currencyId());
    // set both the commodities to be the same here, in case of a two split transaction
    // and the other one being in a different commodity, we adjust that later on
    d->ui->creditDebitEdit->setCommodity(commodity);

    // we block sending out signals for the account and category combo here
    // to avoid calling NewTransactionEditorPrivate::categoryChanged which
    // does not work properly when loading the editor
    QSignalBlocker accountBlocker(d->ui->accountCombo->lineEdit());
    d->ui->accountCombo->clearEditText();
    QSignalBlocker categoryBlocker(d->ui->categoryCombo->lineEdit());
    d->ui->categoryCombo->clearEditText();

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

        d->ui->creditDebitEdit->setSharesCommodity(commodity);
        // the default exchange rate is 1 so we don't need to set it here

    } else {
        d->loadTransaction(idx);
    }

    setInitialFocus();
}


void NewTransactionEditor::editSplits()
{
    d->editSplits() == QDialog::Accepted ? acceptEdit() : reject();
}

MyMoneyMoney NewTransactionEditor::transactionAmount() const
{
    auto amount = d->ui->creditDebitEdit->value();
    if (amount.isZero()) {
        amount = -d->splitsSum();
    }
    return amount;
}

MyMoneyTransaction NewTransactionEditor::transaction() const
{
    MyMoneyTransaction t;

    if (!d->m_transaction.id().isEmpty()) {
        t = d->m_transaction;
    } else {
        // use the commodity of the account
        t.setCommodity(d->m_transaction.commodity());

        // we keep the date when adding a new transaction
        // for the next new one
        KMyMoneySettings::setLastUsedPostDate(d->ui->dateEdit->date().startOfDay());
    }

    // first remove the splits that are gone
    const auto splits = t.splits();
    for (const auto& split : splits) {
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
        sp.setPrice(MyMoneyMoney::ONE);
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

    if (sp.id().isEmpty()) {
        t.addSplit(sp);
    } else {
        t.modifySplit(sp);
    }
    t.setPostDate(d->ui->dateEdit->date());

    // memo handling only takes place when we have two splits
    // in all other cases, we do not touch the memo items
    if (d->splitModel.rowCount() == 1) {
        // if the memo in the split model is empty, we use the one from the widget in the other split
        const auto idx = d->splitModel.index(0, 0);
        QString counterMemo = idx.data(eMyMoney::Model::SplitMemoRole).toString();
        if (counterMemo.isEmpty()) {
            counterMemo = sp.memo();
        } else if (sp.memo() != counterMemo) {
            // in case the two memos differ, the next step is dependent on the account type
            // of the counter account:
            //   case a) income/expense account or
            //   case b) any other account type
            // in case a) we copy the memo from the widget and
            // in case b) we ask the user what to do
            if (MyMoneyAccount::isIncomeExpense(idx.data(eMyMoney::Model::AccountTypeRole).value<eMyMoney::Account::Type>())) {
                counterMemo = sp.memo();
            } else if (KMessageBox::questionTwoActions(
                           parentWidget(),
                           i18n("Do you want to replace memo<p><i>%1</i></p>with memo<p><i>%2</i></p>in the other split?", counterMemo, sp.memo()),
                           i18n("Copy memo"),
                           KMMYesNo::yes(),
                           KMMYesNo::no(),
                           QStringLiteral("CopyMemoOver"))
                       == KMessageBox::PrimaryAction) {
                counterMemo = sp.memo();
            }
        }
        d->splitModel.setData(idx, counterMemo, eMyMoney::Model::SplitMemoRole);
    }

    // now update and add what we have in the model
    d->splitModel.addSplitsToTransaction(t);

    return t;
}

QStringList NewTransactionEditor::saveTransaction(const QStringList& selectedJournalEntries)
{
    auto t = transaction();

    auto selection(selectedJournalEntries);
    connect(MyMoneyFile::instance()->journalModel(), &JournalModel::idChanged, this, [&](const QString& currentId, const QString& previousId) {
        selection.replaceInStrings(previousId, currentId);
    });

    MyMoneyFileTransaction ft;
    try {
        if (t.id().isEmpty()) {
            MyMoneyFile::instance()->addTransaction(t);
            selection = journalEntrySelection(t.id(), d->m_account.id());
        } else {
            t.setImported(false);
            MyMoneyFile::instance()->modifyTransaction(t);
        }
        ft.commit();

    } catch (const MyMoneyException& e) {
        qDebug() << Q_FUNC_INFO << "something went wrong" << e.what();
        selection = selectedJournalEntries;
    }

    return selection;
}

bool NewTransactionEditor::eventFilter(QObject* o, QEvent* e)
{
    if (o) {
        auto combobox = qobject_cast<QComboBox*>(o);

        if (e->type() == QEvent::FocusOut) {
            if (combobox == d->ui->categoryCombo) {
                if (needCreateCategory(d->ui->categoryCombo)) {
                    createCategory(d->ui->categoryCombo, defaultCategoryType(d->ui->creditDebitEdit));
                }

            } else if (o == d->ui->payeeEdit) {
                // in case the popup for the payee is visible we need to copy the
                // selected text into the combobox widget so that the payee can
                // be found in the list
                if (combobox->lineEdit()->completer()->popup()->isVisible()) {
                    const auto view = combobox->lineEdit()->completer()->popup();
                    const auto model = view->selectionModel();
                    const auto selectedRows = model->selectedRows();
                    if (!selectedRows.isEmpty()) {
                        combobox->setCurrentText(selectedRows.at(0).data(eMyMoney::Model::PayeeNameRole).toString());
                    }
                }

                if (needCreatePayee(combobox)) {
                    createPayee(combobox);

                } else if (!combobox->currentText().isEmpty()) {
                    const auto index(combobox->findText(combobox->currentText()));
                    combobox->setCurrentIndex(index);
                    // check if category is filled and fill with
                    // default for payee if one is setup
                    d->defaultCategoryAssignment();
                }
            } else if (o == d->ui->tagContainer->tagCombo()) {
                if (needCreateTag(combobox)) {
                    createTag(d->ui->tagContainer);
                }
            }
        } else if (e->type() == QEvent::FocusIn) {
            if (o == d->ui->payeeEdit) {
                // set case sensitivity so that a payee with the same spelling
                // but different case will be presented in the popup view of
                // the completion box. We need to do that because the CaseSensitive
                // mode is set when the focus leaves the widget (see above).
                d->ui->payeeEdit->completer()->setCaseSensitivity(Qt::CaseInsensitive);
            }
        }
    }
    return TransactionEditorBase::eventFilter(o, e);
}

QDate NewTransactionEditor::postDate() const
{
    return d->ui->dateEdit->date();
}

void NewTransactionEditor::setShowAccountCombo(bool show) const
{
    d->ui->accountLabel->setVisible(show);
    d->ui->accountCombo->setVisible(show);
    d->ui->topMarginWidget->setVisible(show);
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
            widgetHintFrameCollection()->removeWidget(d->ui->enterButton);
            d->ui->enterButton->setDisabled(true);
        } else {
            // no need to enable the enter button here as the
            // frameCollection will take care of it anyway
            widgetHintFrameCollection()->addWidget(d->ui->enterButton);
        }
    }
}

QWidget* NewTransactionEditor::setupUi(QWidget* parent)
{
    if (d->tabOrderUi == nullptr) {
        d->tabOrderUi = new Ui::NewTransactionEditor;
    }
    d->tabOrderUi->setupUi(parent);
    d->tabOrderUi->accountLabel->setVisible(d->ui->accountLabel->isVisible());
    d->tabOrderUi->accountCombo->setVisible(d->ui->accountCombo->isVisible());
    return this;
}

void NewTransactionEditor::storeTabOrder(const QStringList& tabOrder)
{
    // if we are embedded into another widget (e.g. KEditScheduleDlg) then
    // we never save our own tab order even if called accidentally.
    if (!d->m_externalTabOrder) {
        d->m_tabOrder.setTabOrder(tabOrder);
    }
}

void NewTransactionEditor::setExternalTabOrder(TabOrder* tabOrder)
{
    d->m_externalTabOrder = tabOrder;
}

TabOrder* NewTransactionEditor::tabOrder() const
{
    if (d->m_externalTabOrder) {
        return d->m_externalTabOrder;
    }
    return &d->m_tabOrder;
}

void NewTransactionEditor::slotSettingsChanged()
{
    TransactionEditorBase::slotSettingsChanged();

    d->categoriesModel->setShowAllEntries(KMyMoneySettings::showAllAccounts());
    d->accountsModel->setShowAllEntries(KMyMoneySettings::showAllAccounts());
}

void NewTransactionEditor::setKeepCategoryAmount(bool keepCategoryAmount)
{
    d->keepCategoryAmount = keepCategoryAmount;
}

bool NewTransactionEditor::isTransactionDataValid() const
{
    return d->checkForValidTransaction(false);
}

#include "newtransactioneditor.moc"
