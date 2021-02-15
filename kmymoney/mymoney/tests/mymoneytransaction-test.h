/*
    SPDX-FileCopyrightText: 2002-2012 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTRANSACTIONTEST_H
#define MYMONEYTRANSACTIONTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyTransactionTest;

#include "mymoneytransaction.h"

class MyMoneyTransactionTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyTransaction *m;

private Q_SLOTS:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testSetFunctions();
  void testConstructor();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testEquality();
  void testInequality();
  void testAddSplits();
  void testModifySplits();
  void testDeleteSplits();
  void testExtractSplit();
  void testDeleteAllSplits();
  void testSplitSum();
  void testIsLoanPayment();
  //void testAddDuplicateAccount();
  //void testModifyDuplicateAccount();
  void testAutoCalc();
  void testHasReferenceTo();
  void testIsStockSplit();
  void testAddMissingAccountId();
  void testModifyMissingAccountId();
  void testReplaceId();
};
#endif
