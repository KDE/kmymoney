/***************************************************************************
                          mymoneyaccounttest.h
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
