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

#include "mymoneytransaction-test.h"

#include <QDebug>

#include <QtTest>

#include "mymoneytestutils.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneysplit_p.h"
#include "mymoneytransaction.h"
#include "mymoneytransaction_p.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

QTEST_GUILESS_MAIN(MyMoneyTransactionTest)

void MyMoneyTransactionTest::init()
{
  m = new MyMoneyTransaction();
}

void MyMoneyTransactionTest::cleanup()
{
  delete m;
}

void MyMoneyTransactionTest::testEmptyConstructor()
{
  QVERIFY(m->id().isEmpty());
  QVERIFY(m->entryDate() == QDate());
  QVERIFY(m->memo().isEmpty());
  QVERIFY(m->splits().count() == 0);
}

void MyMoneyTransactionTest::testSetFunctions()
{
  m->setMemo("Memo");
  m->setPostDate(QDate(1, 2, 3));

  QVERIFY(m->postDate() == QDate(1, 2, 3));
  QVERIFY(m->memo() == "Memo");
}

void MyMoneyTransactionTest::testConstructor()
{
  testSetFunctions();
  MyMoneyTransaction a("ID", *m);

  QVERIFY(a.id() == "ID");
  QVERIFY(a.entryDate() == QDate::currentDate());
  QVERIFY(a.memo() == "Memo");
  QVERIFY(a.postDate() == QDate(1, 2, 3));
}

void MyMoneyTransactionTest::testCopyConstructor()
{
  testConstructor();
  MyMoneyTransaction a("ID", *m);
  a.setValue("Key", "Value");

  MyMoneyTransaction n(a);

  QVERIFY(n.id() == "ID");
  QVERIFY(n.entryDate() == QDate::currentDate());
  QVERIFY(n.memo() == "Memo");
  QVERIFY(n.postDate() == QDate(1, 2, 3));
  QVERIFY(n.value("Key") == "Value");
}

void MyMoneyTransactionTest::testAssignmentConstructor()
{
  testConstructor();
  MyMoneyTransaction a("ID", *m);
  a.setValue("Key", "Value");

  MyMoneyTransaction n;

  n = a;

  QVERIFY(n.id() == "ID");
  QVERIFY(n.entryDate() == QDate::currentDate());
  QVERIFY(n.memo() == "Memo");
  QVERIFY(n.postDate() == QDate(1, 2, 3));
  QVERIFY(n.value("Key") == "Value");
}

void MyMoneyTransactionTest::testEquality()
{
  testConstructor();

  MyMoneyTransaction n(*m);

  QVERIFY(n == *m);
  QVERIFY(!(n != *m));
}

void MyMoneyTransactionTest::testInequality()
{
  testConstructor();

  MyMoneyTransaction n(*m);

  n.setPostDate(QDate(1, 1, 1));
  QVERIFY(!(n == *m));
  QVERIFY(n != *m);

  n = *m;
  n.setValue("key", "value");
  QVERIFY(!(n == *m));
  QVERIFY(n != *m);
}

void MyMoneyTransactionTest::testAddSplits()
{
  m->d_func()->setId("TestID");
  MyMoneySplit split1, split2;
  split1.setAccountId("A000001");
  split2.setAccountId("A000002");
  split1.setValue(MyMoneyMoney(100, 100));
  split2.setValue(MyMoneyMoney(200, 100));

  try {
    QVERIFY(m->accountReferenced("A000001") == false);
    QVERIFY(m->accountReferenced("A000002") == false);
    m->addSplit(split1);
    m->addSplit(split2);
    QVERIFY(m->splitCount() == 2);
    QVERIFY(m->splits()[0].accountId() == "A000001");
    QVERIFY(m->splits()[1].accountId() == "A000002");
    QVERIFY(m->accountReferenced("A000001") == true);
    QVERIFY(m->accountReferenced("A000002") == true);
    QVERIFY(m->splits()[0].id() == "S0001");
    QVERIFY(m->splits()[1].id() == "S0002");
    QVERIFY(split1.id() == "S0001");
    QVERIFY(split2.id() == "S0002");
    QVERIFY(m->splits()[0].transactionId() == "TestID");
    QVERIFY(m->splits()[1].transactionId() == "TestID");
    QVERIFY(split1.transactionId() == "TestID");
    QVERIFY(split2.transactionId() == "TestID");

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  // try to add split with assigned ID
  try {
    m->addSplit(split1);
    QFAIL("Exception expected!");

  } catch (const MyMoneyException &) {
  }
}

void MyMoneyTransactionTest::testModifySplits()
{
  testAddSplits();
  MyMoneySplit split;

  split = m->splits()[0];
  split.setAccountId("A000003");
  split.d_func()->setId("S00000000");

  // this one should fail, because the ID is invalid
  try {
    m->modifySplit(split);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  // set id to correct value, and check that it worked
  split.d_func()->setId("S0001");
  try {
    m->modifySplit(split);
    QVERIFY(m->splitCount() == 2);
    QVERIFY(m->splits()[0].accountId() == "A000003");
    QVERIFY(m->splits()[1].accountId() == "A000002");
    QVERIFY(m->accountReferenced("A000001") == false);
    QVERIFY(m->accountReferenced("A000002") == true);
    QVERIFY(m->splits()[0].id() == "S0001");
    QVERIFY(m->splits()[1].id() == "S0002");

    QVERIFY(split.id() == "S0001");
    QVERIFY(split.accountId() == "A000003");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyTransactionTest::testDeleteSplits()
{
  testAddSplits();
  MyMoneySplit split;

  // add a third split
  split.setAccountId("A000003");
  split.setValue(MyMoneyMoney(300, 100));
  try {
    m->addSplit(split);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception!");
  }

  split.d_func()->setId("S00000000");
  // this one should fail, because the ID is invalid
  try {
    m->modifySplit(split);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  // set id to correct value, and check that it worked
  split.d_func()->setId("S0002");
  try {
    m->removeSplit(split);
    QVERIFY(m->splitCount() == 2);
    QVERIFY(m->splits()[0].accountId() == "A000001");
    QVERIFY(m->accountReferenced("A000002") == false);
    QVERIFY(m->accountReferenced("A000001") == true);
    QVERIFY(m->accountReferenced("A000003") == true);
    QVERIFY(m->splits()[0].id() == "S0001");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception!");
  }

  // set id to the other correct value, and check that it worked
  split.d_func()->setId("S0003");
  try {
    m->removeSplit(split);
    QVERIFY(m->splitCount() == 1);
    QVERIFY(m->accountReferenced("A000001") == true);
    QVERIFY(m->accountReferenced("A000003") == false);
    QVERIFY(m->splits()[0].id() == "S0001");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyTransactionTest::testDeleteAllSplits()
{
  testAddSplits();

  try {
    m->removeSplits();
    QVERIFY(m->splitCount() == 0);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyTransactionTest::testExtractSplit()
{
  testAddSplits();
  MyMoneySplit split;

  // this one should fail, as the account is not referenced by
  // any split in the transaction
  try {
    split = m->splitByAccount(QString("A000003"));
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  // this one should be found
  try {
    split = m->splitByAccount(QString("A000002"));
    QVERIFY(split.id() == "S0002");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception!");
  }

  // this one should be found also
  try {
    split = m->splitByAccount(QString("A000002"), false);
    QVERIFY(split.id() == "S0001");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyTransactionTest::testSplitSum()
{
  QVERIFY(m->splitSum().isZero());

  testAddSplits();

  MyMoneySplit s1, s2;

  s1 = m->splits()[0];
  s1.setValue(MyMoneyMoney());
  s2 = m->splits()[1];
  s2.setValue(MyMoneyMoney());

  m->modifySplit(s1);
  m->modifySplit(s2);
  QVERIFY(m->splitSum().isZero());

  s1.setValue(MyMoneyMoney(1234, 100));
  m->modifySplit(s1);
  QVERIFY(m->splitSum() == MyMoneyMoney(1234, 100));

  s2.setValue(MyMoneyMoney(-1234, 100));
  m->modifySplit(s2);
  QVERIFY(m->splitSum().isZero());

  s1.setValue(MyMoneyMoney(5678, 100));
  m->modifySplit(s1);
  QVERIFY(m->splitSum() == MyMoneyMoney(4444, 100));
}

void MyMoneyTransactionTest::testIsLoanPayment()
{
  testAddSplits();
  QVERIFY(m->isLoanPayment() == false);

  MyMoneySplit s1, s2;
  s1 = m->splits()[0];
  s2 = m->splits()[1];

  s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
  m->modifySplit(s1);
  QVERIFY(m->isLoanPayment() == true);
  s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal));
  m->modifySplit(s1);
  QVERIFY(m->isLoanPayment() == false);

  s2.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
  m->modifySplit(s2);
  QVERIFY(m->isLoanPayment() == true);
  s2.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal));
  m->modifySplit(s2);
  QVERIFY(m->isLoanPayment() == false);
}

#if 0
void MyMoneyTransactionTest::testAddDuplicateAccount()
{
  testAddSplits();

  qDebug() << "Split count is" << m->splitCount();

  MyMoneySplit split1, split2;
  split1.setAccountId("A000001");
  split2.setAccountId("A000002");
  split1.setValue(MyMoneyMoney(100));
  split2.setValue(MyMoneyMoney(200));

  try {
    QVERIFY(m->accountReferenced("A000001") == true);
    QVERIFY(m->accountReferenced("A000002") == true);
    m->addSplit(split1);
    m->addSplit(split2);
    qDebug() << "Split count is" << m->splitCount();
    QVERIFY(m->splitCount() == 2);
    QVERIFY(m->splits()[0].accountId() == "A000001");
    QVERIFY(m->splits()[1].accountId() == "A000002");
    QVERIFY(m->accountReferenced("A000001") == true);
    QVERIFY(m->accountReferenced("A000002") == true);
    QVERIFY(m->splits()[0].id() == "S0001");
    QVERIFY(m->splits()[1].id() == "S0002");
    QVERIFY(split1.id() == "S0001");
    QVERIFY(split2.id() == "S0002");

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  QVERIFY(m->splits()[0].value() == MyMoneyMoney(200));
  QVERIFY(m->splits()[1].value() == MyMoneyMoney(400));
}

void MyMoneyTransactionTest::testModifyDuplicateAccount()
{
  testAddSplits();
  MyMoneySplit split;

  split = m->splitByAccount(QString("A000002"));
  split.setAccountId("A000001");
  try {
    m->modifySplit(split);
    QVERIFY(m->splitCount() == 1);
    QVERIFY(m->accountReferenced("A000001") == true);
    QVERIFY(m->splits()[0].value() == MyMoneyMoney(300));

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}
#endif

void MyMoneyTransactionTest::testHasReferenceTo()
{
  MyMoneyTransaction t;
  t.setPostDate(QDate(2001, 12, 28));
  t.setEntryDate(QDate(2003, 9, 29));
  t.d_func()->setId("T000000000000000001");
  t.setMemo("Wohnung:Miete");
  t.setCommodity("EUR");
  t.setValue("key", "value");

  MyMoneySplit s;
  s.setPayeeId("P000001");
  QList<QString> tagIdList;
  tagIdList << "G000001";
  s.setTagIdList(tagIdList);
  s.setShares(MyMoneyMoney(96379, 100));
  s.setValue(MyMoneyMoney(96379, 100));
  s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal));
  s.setAccountId("A000076");
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  t.addSplit(s);

  QVERIFY(t.hasReferenceTo("EUR") == true);
  QVERIFY(t.hasReferenceTo("P000001") == true);
  QVERIFY(t.hasReferenceTo("G000001") == true);
  QVERIFY(t.hasReferenceTo("A000076") == true);
}

void MyMoneyTransactionTest::testAutoCalc()
{
  QVERIFY(m->hasAutoCalcSplit() == false);
  testAddSplits();
  QVERIFY(m->hasAutoCalcSplit() == false);
  MyMoneySplit split;

  split = m->splits()[0];
  split.setShares(MyMoneyMoney::autoCalc);
  split.setValue(MyMoneyMoney::autoCalc);
  m->modifySplit(split);

  QVERIFY(m->hasAutoCalcSplit() == true);
}

void MyMoneyTransactionTest::testIsStockSplit()
{
  QVERIFY(m->isStockSplit() == false);
  testAddSplits();
  QVERIFY(m->isStockSplit() == false);
  m->removeSplits();
  MyMoneySplit s;
  s.setShares(MyMoneyMoney(1, 2));
  s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares));
  s.setAccountId("A0001");
  m->addSplit(s);
  QVERIFY(m->isStockSplit() == true);
}

void MyMoneyTransactionTest::testAddMissingAccountId()
{
  MyMoneySplit s;
  s.setShares(MyMoneyMoney(1, 2));
  try {
    m->addSplit(s);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneyTransactionTest::testModifyMissingAccountId()
{
  testAddSplits();
  MyMoneySplit s = m->splits()[0];
  s.setAccountId(QString());

  try {
    m->modifySplit(s);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneyTransactionTest::testReplaceId()
{
  testAddSplits();

  bool changed;

  try {
    changed = m->replaceId("Joe", "Bla");
    QVERIFY(changed == false);
    QVERIFY(m->splits()[0].accountId() == "A000001");
    QVERIFY(m->splits()[1].accountId() == "A000002");

    changed = m->replaceId("A000003", "A000001");
    QVERIFY(changed == true);
    QVERIFY(m->splits()[0].accountId() == "A000003");
    QVERIFY(m->splits()[1].accountId() == "A000002");

    changed = m->replaceId("A000004", "A000002");
    QVERIFY(changed == true);
    QVERIFY(m->splits()[0].accountId() == "A000003");
    QVERIFY(m->splits()[1].accountId() == "A000004");

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}
