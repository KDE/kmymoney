/***************************************************************************
                          mymoneytransactiontest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
  void testWriteXML();
  void testReadXML();
  void testReadXMLEx();
  void testAutoCalc();
  void testHasReferenceTo();
  void testIsStockSplit();
  void testAddMissingAccountId();
  void testModifyMissingAccountId();
  void testReplaceId();
  void testElementNames();
  void testAttributeNames();
};
#endif
