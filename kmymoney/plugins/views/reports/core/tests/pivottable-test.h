/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PIVOTTABLETEST_H
#define PIVOTTABLETEST_H

#include <QObject>

namespace reports
{
class PivotTableTest;
}

#define KMM_MYMONEY_UNIT_TESTABLE friend class reports::PivotTableTest;

#include "mymoneyfile.h"
#include "mymoneystoragemgr.h"
#include "reporttable.h"

namespace reports
{

class PivotTableTest : public QObject
{
  Q_OBJECT
private:
  MyMoneyStorageMgr* storage;
  MyMoneyFile* file;

private Q_SLOTS:
  void setup();
  void init();
  void cleanup();
  void testNetWorthSingle();
  void testNetWorthOfsetting();
  void testNetWorthOpeningPrior();
  void testNetWorthDateFilter();
  void testNetWorthOpening();
  void testSpendingEmpty();
  void testSingleTransaction();
  void testSubAccount();
  void testFilterIEvsIE();
  void testFilterALvsAL();
  void testFilterALvsIE();
  void testFilterAllvsIE();
  void testFilterBasics();
  void testMultipleCurrencies();
  void testAdvancedFilter();
  void testColumnType();
  void testInvestment();
  void testBudget();
  void testHtmlEncoding();
};

}
#endif // PIVOTTABLETEST_H
