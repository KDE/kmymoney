/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reconciliationledgerviewpage.h"
#include "ledgerviewpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QKeyEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "journalmodel.h"
#include "kendingbalancedlg.h"
#include "kmymoneysettings.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyreconciliationreport.h"
#include "reconciliationmodel.h"
#include "schedulesjournalmodel.h"
#include "specialdatesmodel.h"
#include "transactionmatcher.h"
#include "widgetenums.h"

#include "kmmyesno.h"

using namespace Icons;
using namespace eWidgets;

class ReconciliationLedgerViewPage::Private : public LedgerViewPage::Private
{
public:
    Private(ReconciliationLedgerViewPage* parent)
        : LedgerViewPage::Private(parent)
        , endingBalanceDlg(nullptr)
    {
    }

    QStringList doAutoReconciliation(const QStringList& journalEntryIds, const MyMoneyMoney& difference)
    {
        auto result = journalEntryIds;
        const auto journalModel = MyMoneyFile::instance()->journalModel();
        MyMoneyMoney transactionsBalance;

        // optimize the most common case - all transactions are already cleared
        QStringList unclearedJournalEntryIds;
        for (const auto& journalEntryId : journalEntryIds) {
            const auto idx = journalModel->indexById(journalEntryId);
            if (idx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>() == eMyMoney::Split::State::Cleared) {
                transactionsBalance += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
            } else {
                qDebug() << "Found uncleared" << journalEntryId;
                unclearedJournalEntryIds.append(journalEntryId);
            }
        }
        if (difference == transactionsBalance) {
            return {};
        }

        // only one transaction is uncleared
        if (unclearedJournalEntryIds.count() == 1) {
            const auto idx = journalModel->indexById(unclearedJournalEntryIds.front());
            const auto splitAmount = idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
            if (transactionsBalance + splitAmount == difference) {
                return unclearedJournalEntryIds;
            }
        }

        // more than one transaction is uncleared - apply the algorithm
        // (which I don't understand anymore, the original can be found
        // in KGlobalLedgerViewPrivate::automaticReconciliation()
        return {};
    }

    QStringList journalEntriesToReconcile()
    {
        const auto endDate = endingBalanceDlg->statementDate();
        const auto model = ui->m_ledgerView->model();
        const auto journalModel = MyMoneyFile::instance()->journalModel();
        const auto rows = model->rowCount();
        // collect possible candidates
        QStringList journalEntryIds;
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            const auto baseIdx = journalModel->mapToBaseSource(idx);
            if (baseIdx.model() == journalModel) {
                if (baseIdx.data(eMyMoney::Model::TransactionPostDateRole).toDate() <= endDate) {
                    const auto state = baseIdx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>();
                    if ((state == eMyMoney::Split::State::NotReconciled) || (state == eMyMoney::Split::State::Cleared)) {
                        journalEntryIds.append(baseIdx.data(eMyMoney::Model::IdRole).toString());
                    }
                }
            }
        }
        return journalEntryIds;
    }

    void autoReconciliation()
    {
        const auto startBalance = endingBalanceDlg->previousBalance();
        const auto endBalance = endingBalanceDlg->endingBalance();

        // collect possible candidates
        QStringList journalEntryIds(journalEntriesToReconcile());
        if (!journalEntryIds.isEmpty()) {
            const auto autoClearList = doAutoReconciliation(journalEntryIds, endBalance - startBalance);
            if (!autoClearList.empty()) {
                QString message =
                    i18n("KMyMoney has detected transactions matching your reconciliation data.\nWould you like KMyMoney to clear these transactions for you?");
                if (KMessageBox::questionTwoActions(q,
                                                    message,
                                                    i18n("Automatic reconciliation"),
                                                    KMMYesNo::yes(),
                                                    KMMYesNo::no(),
                                                    "AcceptAutomaticReconciliation")
                    == KMessageBox::PrimaryAction) {
                    // Select the journal entries to be cleared
                    SelectedObjects tempSelections;
                    tempSelections.setSelection(SelectedObjects::JournalEntry, autoClearList);
                    Q_EMIT q->requestSelectionChanged(tempSelections);
                    // mark them cleared
                    pActions[eMenu::Action::MarkCleared]->trigger();
                    // and reset the selection to what it was before
                    Q_EMIT q->requestSelectionChanged(selections);
                }
            }
        }
    }

    void startReconciliation()
    {
        delete endingBalanceDlg;
        endingBalanceDlg = nullptr;

        const auto file = MyMoneyFile::instance();
        try {
            auto account = file->account(accountId);
            endingBalanceDlg = new KEndingBalanceDlg(account, q);
            if (account.isAssetLiability()) {
                if (endingBalanceDlg->exec() == QDialog::Accepted) {
                    if (KMyMoneySettings::autoReconciliation()) {
                        autoReconciliation();
                    }
                    auto ti = endingBalanceDlg->interestTransaction();
                    auto tc = endingBalanceDlg->chargeTransaction();
                    MyMoneyFileTransaction ft;
                    try {
                        account.deletePair("lastReconciledBalance");
                        account.setValue("statementBalance", endingBalanceDlg->endingBalance().toString());
                        account.setValue("statementDate", endingBalanceDlg->statementDate().toString(Qt::ISODate));
                        MyMoneyFile::instance()->modifyAccount(account);

                        if (ti != MyMoneyTransaction()) {
                            MyMoneyFile::instance()->addTransaction(ti);
                        }
                        if (tc != MyMoneyTransaction()) {
                            MyMoneyFile::instance()->addTransaction(tc);
                        }
                        ft.commit();
                        file->reconciliationModel()->updateData();

                    } catch (const MyMoneyException& e) {
                        qWarning("interest transaction not stored: '%s'", e.what());
                    }
                    updateSummaryInformation();
                } else {
                    pActions[eMenu::Action::CancelReconciliation]->trigger();
                }
            } else {
                pActions[eMenu::Action::CancelReconciliation]->trigger();
            }
        } catch (const MyMoneyException& e) {
            qDebug() << "Starting reconciliation dialog failed" << e.what();
        }
    }

    bool cancelReconciliation()
    {
        const auto file = MyMoneyFile::instance();
        auto reconciliationAccount = file->account(accountId);

        reconciliationAccount.deletePair("lastReconciledBalance");
        reconciliationAccount.deletePair("statementBalance");
        reconciliationAccount.deletePair("statementDate");
        MyMoneyFileTransaction ft;
        try {
            // update the account data
            file->modifyAccount(reconciliationAccount);
            ft.commit();
            file->reconciliationModel()->updateData();

        } catch (const MyMoneyException&) {
            qDebug() << "Unexpected exception while cancelling of reconciliation of" << reconciliationAccount.name();
        }
        return true;
    }

    bool finishReconciliation()
    {
        const auto file = MyMoneyFile::instance();
        const auto journalModel = file->journalModel();

        // collect candidates
        QStringList journalEntryIds(journalEntriesToReconcile());
        if (!journalEntryIds.isEmpty()) {
            auto balance = file->balance(accountId, endingBalanceDlg->statementDate());

            // walk the list of journalEntries to figure out the balance(s)
            for (const auto& journalEntryId : journalEntryIds) {
                const auto idx = journalModel->indexById(journalEntryId);
                if (idx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>() == eMyMoney::Split::State::NotReconciled) {
                    balance -= idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                }
            }

            if (endingBalanceDlg->endingBalance() != balance) {
                auto message = i18n(
                    "You are about to finish the reconciliation of this account with a difference between your bank statement and the transactions marked as "
                    "cleared.\n"
                    "Are you sure you want to finish the reconciliation?");
                if (KMessageBox::questionTwoActions(q, message, i18n("Confirm end of reconciliation"), KMMYesNo::yes(), KMMYesNo::no())
                    == KMessageBox::SecondaryAction) {
                    return false;
                }
            }
        }

        MyMoneyFileTransaction ft;

        // refresh object
        auto reconciliationAccount = file->account(accountId);

        // only update the last statement balance here, if we haven't a newer one due
        // to download of online statements.
        if (reconciliationAccount.value("lastImportedTransactionDate").isEmpty()
            || QDate::fromString(reconciliationAccount.value("lastImportedTransactionDate"), Qt::ISODate) < endingBalanceDlg->statementDate()) {
            reconciliationAccount.setValue("lastStatementBalance", endingBalanceDlg->endingBalance().toString());
            // in case we override the last statement balance here, we have to make sure
            // that we don't show the online balance anymore, as it might be different
            reconciliationAccount.deletePair("lastImportedTransactionDate");
        }
        reconciliationAccount.setLastReconciliationDate(endingBalanceDlg->statementDate());

        // keep a record of this reconciliation
        reconciliationAccount.addReconciliation(endingBalanceDlg->statementDate(), endingBalanceDlg->endingBalance());

        reconciliationAccount.deletePair("lastReconciledBalance");
        reconciliationAccount.deletePair("statementBalance");
        reconciliationAccount.deletePair("statementDate");

        try {
            // update the account data
            file->modifyAccount(reconciliationAccount);

            // walk the list of transactions/splits and mark the cleared ones as reconciled
            QStringList reconciledJournalEntryIds;
            TransactionMatcher matcher;
            for (const auto& journalEntryId : journalEntryIds) {
                const auto journalEntry = journalModel->itemById(journalEntryId);
                auto sp = journalEntry.split();

                // skip the ones that are not marked cleared
                if (sp.reconcileFlag() != eMyMoney::Split::State::Cleared)
                    continue;

                auto t = journalEntry.transaction();

                sp.setReconcileFlag(eMyMoney::Split::State::Reconciled);
                sp.setReconcileDate(endingBalanceDlg->statementDate());
                t.setImported(false);
                t.modifySplit(sp);

                // update the engine ...
                file->modifyTransaction(t);

                matcher.accept(t, sp);

                // ... and the list
                reconciledJournalEntryIds.append(journalEntryId);
            }
            ft.commit();

            /// send information to plugins through a QAction. Data is
            /// a) accountId
            /// b) reconciledJournalEntryIds
            /// c) statementDate
            /// d) previousBalance
            /// e) endingBalance

            MyMoneyReconciliationReport report;
            report.accountId = reconciliationAccount.id();
            report.journalEntryIds = journalEntryIds;
            report.statementDate = endingBalanceDlg->statementDate();
            report.startingBalance = endingBalanceDlg->previousBalance();
            report.endingBalance = endingBalanceDlg->endingBalance();

            if (!report.accountId.isEmpty() && !report.journalEntryIds.isEmpty() && report.statementDate.isValid()) {
                pActions[eMenu::Action::ReconciliationReport]->setData(QVariant::fromValue(report));
                pActions[eMenu::Action::ReconciliationReport]->trigger();
            }

        } catch (const MyMoneyException&) {
            qDebug("Unexpected exception when setting cleared to reconcile");
        }
        return true;
    }

    void postponeReconciliation()
    {
        MyMoneyFileTransaction ft;
        const auto file = MyMoneyFile::instance();

        // refresh object
        auto reconciliationAccount = file->account(accountId);

        if (!reconciliationAccount.id().isEmpty()) {
            reconciliationAccount.setValue("lastReconciledBalance", endingBalanceDlg->previousBalance().toString());
            reconciliationAccount.setValue("statementBalance", endingBalanceDlg->endingBalance().toString());
            reconciliationAccount.setValue("statementDate", endingBalanceDlg->statementDate().toString(Qt::ISODate));

            try {
                file->modifyAccount(reconciliationAccount);
                ft.commit();
            } catch (const MyMoneyException&) {
                qDebug("Unexpected exception when setting last reconcile info into account");
                ft.rollback();
            }
        }
    }

    void updateAccountData(const MyMoneyAccount& account) override
    {
        LedgerViewPage::Private::updateAccountData(account);

        if (account.accountType() == eMyMoney::Account::Type::Investment) {
            sortOrderType = LedgerViewSettings::SortOrderReconcileInvest;
        } else {
            sortOrderType = LedgerViewSettings::SortOrderReconcileStd;
        }

        // check if we have a specific sort order or rely on the default
        if (!account.value("kmm-sort-reconcile").isEmpty()) {
            sortOrder = LedgerSortOrder(account.value("kmm-sort-reconcile"));
        } else {
            sortOrder = LedgerViewSettings::instance()->sortOrder(sortOrderType);
        }
    }

    void updateSummaryInformation() const override
    {
        ui->m_reconciliationContainer->setVisible(endingBalanceDlg != nullptr);
        if (endingBalanceDlg) {
            const auto endingBalance = endingBalanceDlg->endingBalance();
            const auto balance = MyMoneyFile::instance()->journalModel()->clearedBalance(accountId, endingBalanceDlg->statementDate());
            ui->m_leftLabel->setText(i18nc("@label:textbox Statement balance", "Statement: %1", endingBalance.formatMoney("", precision)));
            ui->m_centerLabel->setText(i18nc("@label:textbox Cleared balance", "Cleared: %1", balance.formatMoney("", precision)));
            ui->m_rightLabel->setText(i18nc("@label:textbox Difference to statement", "Difference: %1", (balance - endingBalance).formatMoney("", precision)));
        }
    }

    void clearFilter() override
    {
        LedgerViewPage::Private::clearFilter();
        stateFilter->setStateFilter(LedgerFilter::State::NotReconciled);
    }

    KEndingBalanceDlg* endingBalanceDlg;
};

ReconciliationLedgerViewPage::ReconciliationLedgerViewPage(QWidget* parent, const QString& configGroupName)
    : LedgerViewPage(*new Private(this), parent, configGroupName)
{
    // in reconciliation mode we use a fixed state filter
    d->ui->m_filterBox->setCurrentIndex(static_cast<int>(LedgerFilter::State::NotReconciled));
    d->ui->m_filterBox->setDisabled(true);
}

ReconciliationLedgerViewPage::~ReconciliationLedgerViewPage()
{
}

void ReconciliationLedgerViewPage::setAccount(const MyMoneyAccount& account)
{
    LedgerViewPage::setAccount(account);
    if (d->needModelInit) {
        return;
    }

    d->selections.setSelection(SelectedObjects::ReconciliationAccount, account.id());
    d->stateFilter->setStateFilter(LedgerFilter::State::NotReconciled);
}

bool ReconciliationLedgerViewPage::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_UNUSED(selections)

    auto dd = static_cast<ReconciliationLedgerViewPage::Private*>(d);
    switch (action) {
    case eMenu::Action::StartReconciliation:
        dd->startReconciliation();
        break;

    case eMenu::Action::PostponeReconciliation:
        dd->postponeReconciliation();
        break;

    case eMenu::Action::FinishReconciliation:
        return dd->finishReconciliation();

    case eMenu::Action::CancelReconciliation:
        return dd->cancelReconciliation();

    default:
        break;
    }
    return true;
}

void ReconciliationLedgerViewPage::updateSummaryInformation(const QHash<QString, AccountBalances>& balances)
{
    Q_UNUSED(balances)

    auto dd = static_cast<ReconciliationLedgerViewPage::Private*>(d);
    dd->updateSummaryInformation();
}
