/*
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYFORECASTTEST_H
#define MYMONEYFORECASTTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyForecastTest;

#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneystoragemgr.h"

#include "mymoneyforecast.h"

class MyMoneyForecastTest : public QObject
{
    Q_OBJECT
public:
    MyMoneyForecastTest();

private Q_SLOTS:
    void init();
    void cleanup();
    void testEmptyConstructor();
    void testDoForecast();
    void testDoForecastInit();
    void testGetForecastAccountList();
    void testCalculateAccountTrend();
    void testGetForecastBalance();
    void testIsForecastAccount();
    void testDoFutureScheduledForecast();
    void testDaysToMinimumBalance();
    void testDaysToZeroBalance();
    void testScheduleForecast();
    void testSkipOpeningDate();
    void testAccountMinimumBalanceDateList();
    void testAccountMaximumBalanceDateList();
    void testAccountAverageBalance();
    void testBeginForecastDate();
    void testHistoryDays();
    void testCreateBudget();
    void testLinearRegression();

protected:
    MyMoneyForecast *m;

private:
    MyMoneyStorageMgr* storage;
    MyMoneyFile* file;

    MyMoneyMoney moT1;
    MyMoneyMoney moT2;
    MyMoneyMoney moT3;
    MyMoneyMoney moT4;
    MyMoneyMoney moT5;
};

#endif
