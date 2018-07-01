/***************************************************************************
                          querytabletest.h
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

#ifndef QUERYTABLETEST_H
#define QUERYTABLETEST_H

#include <QtCore/QObject>
#include "mymoneyfile.h"
#include "mymoneyseqaccessmgr.h"

namespace reports {
class QueryTable;
}

class QueryTableTest : public QObject
{
  Q_OBJECT
private:
  MyMoneySeqAccessMgr* storage;
  MyMoneyFile* file;

private:
  void writeTabletoHTML(reports::QueryTable &table, const QString &filename);
  void saveKMyMoneyFile(const QString &filename);

private slots:
  void init();
  void cleanup();
  void testQueryBasics();
  void testCashFlowAnalysis();
  void testAccountQuery();
  void testInvestment();
  void testBalanceColumn();
  void testBalanceColumnWithMultipleCurrencies();
  void testTaxReport();
};

#endif
