/*
 * SPDX-FileCopyrightText: 2005 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
