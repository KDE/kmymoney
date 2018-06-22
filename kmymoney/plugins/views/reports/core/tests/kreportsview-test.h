/*
 * Copyright 2005       Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2005       Ace Jones <acejones@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KREPORTSVIEWTEST_H
#define KREPORTSVIEWTEST_H

#include <QObject>
#include "mymoneyfile.h"
#include "mymoneystoragemgr.h"

class KReportsViewTest : public QObject
{
  Q_OBJECT

private:
  MyMoneyAccount  *m;

  MyMoneyStorageMgr* storage;
  MyMoneyFile* file;

private Q_SLOTS:
  void init();
  void cleanup();
  void testNetWorthSingle();
  void testNetWorthOfsetting();
  void testNetWorthOpeningPrior();
  void testNetWorthDateFilter();
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
  void testXMLWrite();
  void testQueryBasics();
  void testCashFlowAnalysis();
  void testAccountQuery();
  void testInvestment();
  void testWebQuotes();
  void testDateFormat();
  void testHasReferenceTo();
};

#endif
