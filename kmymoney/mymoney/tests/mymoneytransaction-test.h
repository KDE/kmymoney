/*
 * Copyright 2002-2012  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
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
