/***************************************************************************
                          mymoneyforecasttest.h
                          -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYFORECASTTEST_H
#define MYMONEYFORECASTTEST_H

#include <QtCore/QObject>

#include "mymoneyfile.h"
#include "mymoneyseqaccessmgr.h"

#define private public
#include "mymoneyforecast.h"
#undef private

class MyMoneyForecastTest : public QObject
{
  Q_OBJECT
public:
  MyMoneyForecastTest();

private slots:
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
  MyMoneyAccount  *account;

  MyMoneySeqAccessMgr* storage;
  MyMoneyFile* file;

  MyMoneyMoney moT1;
  MyMoneyMoney moT2;
  MyMoneyMoney moT3;
  MyMoneyMoney moT4;
  MyMoneyMoney moT5;
};

#endif
