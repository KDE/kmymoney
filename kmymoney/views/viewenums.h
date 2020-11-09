/***************************************************************************
                          viewenums.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VIEWENUMS_H
#define VIEWENUMS_H

#include <QHashFunctions>

enum class View { Home = 0, Institutions, Accounts, Schedules, Categories, Tags,
                  Payees, NewLedgers, Investments, Reports, Budget, Forecast, OnlineJobOutbox, OldLedgers, None };

inline uint qHash(const View key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

namespace eView {
  enum class Tag { All = 0,
                   Referenced, // used tags
                   Unused,     // unused tags
                   Opened,     // not closed tags
                   Closed      // closed tags
                 };

  enum class Intent {
    None,
    UpdateActions,
    OpenContextMenu,
    OpenObject,
    ShowPayee,
    ShowTransactionInLedger,
    SynchronizeAccountInInvestmentView,
    SynchronizeAccountInLedgersView,
    ToggleColumn,
    UpdateNetWorth,                             // deprecated
    UpdateProfit,                               // deprecated
    StartEnteringOverdueScheduledTransactions,
    FinishEnteringOverdueScheduledTransactions,
    EnterSchedule,
    ReportProgress,
    ReportProgressMessage,
    SelectRegisterTransactions,
    AccountReconciled,
    SetOnlinePlugins
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
    ShowBalanceChart
  };

}

#endif
