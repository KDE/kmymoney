/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERVIEWPAGE_P_H
#define LEDGERVIEWPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgeraccountfilter.h"
#include "ledgerfilter.h"
#include "mymoneymoney.h"
#include "mymoneyutils.h"
#include "newtransactionform.h"
#include "selectedobjects.h"
#include "specialdatesfilter.h"
#include <ui_ledgerviewpage.h>

class LedgerViewPage::Private
{
public:
    Private(LedgerViewPage* qq)
        : q(qq)
        , ui(new Ui_LedgerViewPage)
        , accountFilter(nullptr)
        , specialDatesFilter(nullptr)
        , stateFilter(nullptr)
        , form(nullptr)
        , stackedView(nullptr)
        , needModelInit(true)
        , showEntryForNewTransaction(false)
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
            ui->m_leftLabel->setText(i18nc("@label:textbox Reconciliation date", "Last reconciliation: %1").arg(MyMoneyUtils::formatDate(reconciliationDate)));
        } else {
            ui->m_leftLabel->setText(i18nc("@label:textbox Reconciliation date", "Never reconciled"));
        }
        ui->m_centerLabel->setText(i18nc("@label:textbox Cleared balance", "Cleared: %1", clearedBalance.formatMoney("", precision)));
        ui->m_rightLabel->setText(i18nc("@label:textbox Total balance", "Balance: %1", totalBalance.formatMoney("", precision)));
    }

    LedgerViewPage* q;
    Ui_LedgerViewPage* ui;
    LedgerAccountFilter* accountFilter;
    SpecialDatesFilter* specialDatesFilter;
    LedgerFilter* stateFilter;
    NewTransactionForm* form;
    LedgerViewPage* stackedView;
    QSet<QString> hideFormReasons;
    QString accountId;
    QString accountName;
    SelectedObjects selections;
    QTimer delayTimer;
    QDate reconciliationDate;
    MyMoneyMoney totalBalance;
    MyMoneyMoney clearedBalance;
    bool needModelInit;
    bool showEntryForNewTransaction;
    int precision;
};

#endif // LEDGERVIEWPAGE_P_H
