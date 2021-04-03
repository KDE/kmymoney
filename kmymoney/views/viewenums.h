/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWENUMS_H
#define VIEWENUMS_H

#include <QHashFunctions>

enum class View { Home = 0, Institutions, Accounts, Schedules, Categories, Tags,
                  Payees, Ledgers, Investments, Reports, Budget, Forecast, OnlineJobOutbox, NewLedgers, None
                };

inline uint qHash(const View key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}

namespace eView {
enum class Tag { All = 0,
                 Referenced, // used tags
                 Unused,     // unused tags
                 Opened,     // not closed tags
                 Closed,     // closed tags
               };

enum class Intent {
    None,
    UpdateActions,
    OpenContextMenu,
    OpenObject,
    ShowPayee,
    ShowTransaction,
    SynchronizeAccountInInvestmentView,
    SynchronizeAccountInLedgersView,
    ToggleColumn,
    UpdateNetWorth,
    UpdateProfit,
    StartEnteringOverdueScheduledTransactions,
    FinishEnteringOverdueScheduledTransactions,
    EnterSchedule,
    ReportProgress,
    ReportProgressMessage,
    SelectRegisterTransactions,
    AccountReconciled,
    SetOnlinePlugins,
};

enum class Action {
    None,
    Refresh,
    SetDefaultFocus,
    AboutToShow,
    Print,
    SwitchView,
    ClosePayeeIdentifierSource,
    EditInstitution,
    EditSchedule,
    CleanupBeforeFileClose,
    InitializeAfterFileOpen,
    DisableViewDepenedendActions,
    ShowBalanceChart,
};

}

#endif
