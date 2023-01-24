/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERVIEWPAGE_P_H
#define LEDGERVIEWPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QPointer>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "kmymoneysettings.h"
#include "ktransactionsortoptionsdlg.h"
#include "ledgeraccountfilter.h"
#include "ledgerfilter.h"
#include "ledgerviewsettings.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "newtransactionform.h"
#include "selectedobjects.h"
#include "specialledgeritemfilter.h"

#include <ui_ledgerviewpage.h>

class LedgerViewPage::Private
{
public:
    Private(LedgerViewPage* qq)
        : q(qq)
        , ui(new Ui_LedgerViewPage)
        , accountFilter(nullptr)
        , specialItemFilter(nullptr)
        , stateFilter(nullptr)
        , form(nullptr)
        , stackedView(nullptr)
        , needModelInit(true)
        , showEntryForNewTransaction(false)
        , isInvestmentView(false)
        , precision(-1)
    {
    }

    virtual ~Private()
    {
        delete ui;
    }

    void initWidgets(const QString& configGroupName)
    {
        ui->setupUi(q);

        // make sure, we can disable the detail form but not the ledger view
        ui->m_splitter->setCollapsible(0, false);
        ui->m_splitter->setCollapsible(1, true);

        // make sure the ledger gets all the stretching
        ui->m_splitter->setStretchFactor(0, 3);
        ui->m_splitter->setStretchFactor(1, 1);
        ui->m_splitter->setSizes(QList<int>() << 10000 << ui->m_formWidget->sizeHint().height());

        delayTimer.setSingleShot(true);

        connect(ui->m_ledgerView, &LedgerView::transactionSelected, q, &LedgerViewPage::transactionSelected);
        connect(ui->m_ledgerView, &LedgerView::aboutToStartEdit, q, &LedgerViewPage::aboutToStartEdit);
        connect(ui->m_ledgerView, &LedgerView::aboutToFinishEdit, q, &LedgerViewPage::aboutToFinishEdit);
        connect(ui->m_ledgerView, &LedgerView::aboutToStartEdit, q, &LedgerViewPage::startEdit);
        connect(ui->m_ledgerView, &LedgerView::aboutToFinishEdit, q, &LedgerViewPage::finishEdit);
        connect(ui->m_ledgerView, &LedgerView::transactionSelectionChanged, q, &LedgerViewPage::slotRequestSelectionChanged);
        connect(ui->m_ledgerView, &LedgerView::requestCustomContextMenu, q, &LedgerViewPage::requestCustomContextMenu);

        connect(ui->m_splitter, &QSplitter::splitterMoved, q, &LedgerViewPage::splitterChanged);

        ui->m_ledgerView->setColumnSelectorGroupName(configGroupName);

        needModelInit = true;
    }

    virtual void updateSummaryInformation() const
    {
        if (reconciliationDate.isValid()) {
            ui->m_leftLabel->setText(i18nc("@label:textbox Reconciliation date", "Last reconciliation: %1", MyMoneyUtils::formatDate(reconciliationDate)));
        } else {
            ui->m_leftLabel->setText(i18nc("@label:textbox Reconciliation date", "Never reconciled"));
        }

        if (isInvestmentView) {
            const auto file = MyMoneyFile::instance();
            const auto account = file->accountsModel()->itemById(accountId);
            const auto baseCurrency = file->baseCurrency();

            // collect accounts and their balances
            QMap<QString, MyMoneyMoney> actBalance;
            actBalance[accountId] = file->balance(accountId);
            for (const auto& accId : account.accountList()) {
                actBalance[accId] = file->balance(accId);
            }

            MyMoneyMoney balance;
            bool balanceIsApproximated = false;
            QMap<QString, MyMoneyMoney>::const_iterator it_b;
            for (it_b = actBalance.cbegin(); it_b != actBalance.cend(); ++it_b) {
                MyMoneyAccount stock = file->account(it_b.key());
                QString currencyId = stock.currencyId();
                MyMoneySecurity sec = file->security(currencyId);
                MyMoneyMoney rate(1, 1);

                if (stock.isInvest()) {
                    currencyId = sec.tradingCurrency();
                    const MyMoneyPrice& priceInfo = file->price(sec.id(), currencyId);
                    balanceIsApproximated |= !priceInfo.isValid();
                    rate = priceInfo.rate(sec.tradingCurrency());
                }

                if (currencyId != baseCurrency.id()) {
                    const MyMoneyPrice& priceInfo = file->price(sec.tradingCurrency(), baseCurrency.id());
                    balanceIsApproximated |= !priceInfo.isValid();
                    rate = (rate * priceInfo.rate(baseCurrency.id())).convertPrecision(sec.pricePrecision());
                }
                balance += ((*it_b) * rate).convert(baseCurrency.smallestAccountFraction());
            }

            ui->m_centerLabel->setText(QString());
            ui->m_rightLabel->setText(i18nc("@label:textbox Total value of investment",
                                            "Investment value: %1%2",
                                            balanceIsApproximated ? QLatin1String("~") : QString(),
                                            balance.formatMoney(baseCurrency.tradingSymbol(), precision)));
        } else {
            ui->m_centerLabel->setText(i18nc("@label:textbox Cleared balance", "Cleared: %1", clearedBalance.formatMoney("", precision)));
            if (selections.count(SelectedObjects::JournalEntry) > 1) {
                ui->m_rightLabel->setText(i18nc("@label:textbox %1 sum symbol, %2 number of selected tx, %3 sum of tx",
                                                "%1 of %2: %3",
                                                QChar(0x2211),
                                                selections.count(SelectedObjects::JournalEntry),
                                                selectedTotal.formatMoney("", precision)));
            } else {
                ui->m_rightLabel->setText(i18nc("@label:textbox Total balance", "Balance: %1", totalBalance.formatMoney("", precision)));
            }
        }
    }

    virtual void clearFilter()
    {
        stateFilter->clearFilter();
        ui->m_filterContainer->hide();
        ui->m_ledgerView->setFocus();
        QMetaObject::invokeMethod(ui->m_ledgerView, &LedgerView::ensureCurrentItemIsVisible, Qt::QueuedConnection);
    }

    virtual void updateAccountData(const MyMoneyAccount& account)
    {
        reconciliationDate = account.lastReconciliationDate();
        precision = MyMoneyMoney::denomToPrec(account.fraction());
        accountName = account.name();

        sortOptionKey = QLatin1String("kmm-sort-std");
        const auto sortOption = account.value(sortOptionKey);

        if (account.accountType() == eMyMoney::Account::Type::Investment) {
            sortOrderType = LedgerViewSettings::SortOrderInvest;
        } else {
            sortOrderType = LedgerViewSettings::SortOrderStd;
        }

        const LedgerSortOrder previousSortOrder = sortOrder;

        // check if we have a specific sort order or rely on the default
        if (!sortOption.isEmpty()) {
            sortOrder = LedgerSortOrder(sortOption);
        } else {
            sortOrder = LedgerViewSettings::instance()->sortOrder(sortOrderType);
        }

        if (previousSortOrder != sortOrder) {
            specialItemFilter->setLedgerSortOrder(sortOrder);
            specialItemFilter->forceReload();
        }
    }

    void selectSortOrder()
    {
        auto file = MyMoneyFile::instance();
        QPointer<KTransactionSortOptionsDlg> dlg = new KTransactionSortOptionsDlg(q);
        auto account = file->accountsModel()->itemById(accountId);
        const auto defaultSortOption =
            (sortOrderType == LedgerViewSettings::SortOrderStd) ? KMyMoneySettings::sortNormalView() : KMyMoneySettings::sortNormalView();
        dlg->setSortOption(account.value(sortOptionKey), defaultSortOption);
        if ((dlg->exec() == QDialog::Accepted) && dlg) {
            const auto newSortOption = dlg->sortOption();
            if (newSortOption != account.value(sortOptionKey)) {
                if (newSortOption.isEmpty()) {
                    account.deletePair(sortOptionKey);
                } else {
                    account.setValue(sortOptionKey, newSortOption);
                }
                MyMoneyFileTransaction ft;
                try {
                    file->modifyAccount(account);
                    ft.commit();
                } catch (const MyMoneyException& e) {
                    qDebug() << "Unable to store sort options with account" << account.name() << e.what();
                }
            }
        }
        delete dlg;
    }

    LedgerViewPage* q;
    Ui_LedgerViewPage* ui;
    LedgerAccountFilter* accountFilter;
    SpecialLedgerItemFilter* specialItemFilter;
    LedgerFilter* stateFilter;
    NewTransactionForm* form;
    LedgerViewPage* stackedView;
    QSet<QString> hideFormReasons;
    QString accountId;
    QString accountName;
    QString sortOptionKey;
    SelectedObjects selections;
    QTimer delayTimer;
    QDate reconciliationDate;
    MyMoneyMoney totalBalance;
    MyMoneyMoney clearedBalance;
    MyMoneyMoney selectedTotal;
    LedgerViewSettings::SortOrderType sortOrderType;
    LedgerSortOrder sortOrder;
    bool needModelInit;
    bool showEntryForNewTransaction;
    bool isInvestmentView;
    int precision;
};

#endif // LEDGERVIEWPAGE_P_H
