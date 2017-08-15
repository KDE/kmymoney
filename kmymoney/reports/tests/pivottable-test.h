/***************************************************************************
                          pivottabletest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PIVOTTABLETEST_H
#define PIVOTTABLETEST_H

#include <QtCore/QObject>

namespace reports
{
class PivotTableTest;
}

#define KMM_MYMONEY_UNIT_TESTABLE friend class reports::PivotTableTest;

#include "mymoneyfile.h"
#include "mymoneyseqaccessmgr.h"
#include "reporttable.h"

namespace reports
{

class PivotTableTest : public QObject
{
  Q_OBJECT
private:
  MyMoneySeqAccessMgr* storage;
  MyMoneyFile* file;

private slots:
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
