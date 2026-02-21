/*
    SPDX-FileCopyrightText: 2026 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kmm_mymoney_export.h"

class KMM_MYMONEY_EXPORT ForecastConfig
{
public:
    int forecastCycles;
    int accountsCycle;
    int forecastDays;
    int beginForecastDay;
    int forecastMethod;
    int historyMethod;
    bool includeFutureTransactions;
    bool includeScheduledTransactions;
    ForecastConfig()
        : forecastCycles(3)
        , accountsCycle(30)
        , forecastDays(90)
        , beginForecastDay(0)
        , forecastMethod(0)
        , historyMethod(1)
        , includeFutureTransactions(true)
        , includeScheduledTransactions(true)
    {
    }
};
