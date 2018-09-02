/*
 * Copyright 2002-2016  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneysplit-test.h"

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneySplitTest;

#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneysplit_p.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

QTEST_GUILESS_MAIN(MyMoneySplitTest)

void MyMoneySplitTest::init()
{
  m = new MyMoneySplit();
}

void MyMoneySplitTest::cleanup()
{
  delete m;
}

void MyMoneySplitTest::testEmptyConstructor()
{
  QCOMPARE(m->accountId().isEmpty(), true);
  QCOMPARE(m->id().isEmpty(), true);
  QCOMPARE(m->memo().isEmpty(), true);
  QCOMPARE(m->action().isEmpty(), true);
  QCOMPARE(m->shares().isZero(), true);
  QCOMPARE(m->value().isZero(), true);
  QCOMPARE(m->reconcileFlag(), eMyMoney::Split::State::NotReconciled);
  QCOMPARE(m->reconcileDate(), QDate());
  QCOMPARE(m->transactionId().isEmpty(), true);
  QCOMPARE(m->costCenterId().isEmpty(), true);
}

void MyMoneySplitTest::testSetFunctions()
{
  m->setAccountId("Account");
  m->setMemo("Memo");
  m->setReconcileDate(QDate(1, 2, 3));
  m->setReconcileFlag(eMyMoney::Split::State::Cleared);
  m->setShares(MyMoneyMoney(1234, 100));
  m->setValue(MyMoneyMoney(3456, 100));
  m->d_func()->setId("MyID");
  m->setPayeeId("Payee");
  m->setCostCenterId("CostCenter");
  QList<QString> tagIdList;
  tagIdList << "Tag";
  m->setTagIdList(tagIdList);
  m->setAction("Action");
  m->setTransactionId("TestTransaction");
  m->setValue("Key", "Value");

  QCOMPARE(m->accountId(), QLatin1String("Account"));
  QCOMPARE(m->memo(), QLatin1String("Memo"));
  QCOMPARE(m->reconcileDate(), QDate(1, 2, 3));
  QCOMPARE(m->reconcileFlag(), eMyMoney::Split::State::Cleared);
  QCOMPARE(m->shares(), MyMoneyMoney(1234, 100));
  QCOMPARE(m->value(), MyMoneyMoney(3456, 100));
  QCOMPARE(m->id(), QLatin1String("MyID"));
  QCOMPARE(m->payeeId(), QLatin1String("Payee"));
  QCOMPARE(m->tagIdList(), tagIdList);
  QCOMPARE(m->action(), QLatin1String("Action"));
  QCOMPARE(m->transactionId(), QLatin1String("TestTransaction"));
  QCOMPARE(m->value("Key"), QLatin1String("Value"));
  QCOMPARE(m->costCenterId(), QLatin1String("CostCenter"));
}


void MyMoneySplitTest::testCopyConstructor()
{
  testSetFunctions();

  MyMoneySplit n(*m);

  QCOMPARE(n.accountId(), QLatin1String("Account"));
  QCOMPARE(n.memo(), QLatin1String("Memo"));
  QCOMPARE(n.reconcileDate(), QDate(1, 2, 3));
  QCOMPARE(n.reconcileFlag(), eMyMoney::Split::State::Cleared);
  QCOMPARE(n.shares(), MyMoneyMoney(1234, 100));
  QCOMPARE(n.value(), MyMoneyMoney(3456, 100));
  QCOMPARE(n.id(), QLatin1String("MyID"));
  QCOMPARE(n.payeeId(), QLatin1String("Payee"));
  QList<QString> tagIdList;
  tagIdList << "Tag";
  QCOMPARE(n.tagIdList(), tagIdList);
  QCOMPARE(n.action(), QLatin1String("Action"));
  QCOMPARE(n.transactionId(), QLatin1String("TestTransaction"));
  QCOMPARE(n.value("Key"), QLatin1String("Value"));
  QCOMPARE(n.costCenterId(), QLatin1String("CostCenter"));
}

void MyMoneySplitTest::testAssignmentConstructor()
{
  testSetFunctions();

  MyMoneySplit n;

  n = *m;

  QCOMPARE(n.accountId(), QLatin1String("Account"));
  QCOMPARE(n.memo(), QLatin1String("Memo"));
  QCOMPARE(n.reconcileDate(), QDate(1, 2, 3));
  QCOMPARE(n.reconcileFlag(), eMyMoney::Split::State::Cleared);
  QCOMPARE(n.shares(), MyMoneyMoney(1234, 100));
  QCOMPARE(n.value(), MyMoneyMoney(3456, 100));
  QCOMPARE(n.id(), QLatin1String("MyID"));
  QCOMPARE(n.payeeId(), QLatin1String("Payee"));
  QList<QString> tagIdList;
  tagIdList << QLatin1String("Tag");
  QCOMPARE(n.tagIdList(), tagIdList);
  QCOMPARE(n.action(), QLatin1String("Action"));
  QCOMPARE(n.transactionId(), QLatin1String("TestTransaction"));
  QCOMPARE(n.value("Key"), QLatin1String("Value"));
  QCOMPARE(n.costCenterId(), QLatin1String("CostCenter"));
}

void MyMoneySplitTest::testEquality()
{
  testSetFunctions();

  MyMoneySplit n(*m);

  QCOMPARE(n, *m);
}

void MyMoneySplitTest::testInequality()
{
  testSetFunctions();

  MyMoneySplit n(*m);

  n.setShares(MyMoneyMoney(3456, 100));
  QVERIFY(!(n == *m));

  n = *m;
  n.d_func()->setId("Not My ID");
  QVERIFY(!(n == *m));

  n = *m;
  n.setPayeeId("No payee");
  QVERIFY(!(n == *m));

  n = *m;
  QList<QString> tagIdList;
  tagIdList << "No tag";
  n.setTagIdList(tagIdList);
  QVERIFY(!(n == *m));

  n = *m;
  n.setAction("No action");
  QVERIFY(!(n == *m));

  n = *m;
  n.setNumber("No number");
  QVERIFY(!(n == *m));

  n = *m;
  n.setAccountId("No account");
  QVERIFY(!(n == *m));

  n = *m;
  n.setMemo("No memo");
  QVERIFY(!(n == *m));

  n = *m;
  n.setReconcileDate(QDate(3, 4, 5));
  QVERIFY(!(n == *m));

  n = *m;
  n.setReconcileFlag(eMyMoney::Split::State::Frozen);
  QVERIFY(!(n == *m));

  n = *m;
  n.setShares(MyMoneyMoney(4567, 100));
  QVERIFY(!(n == *m));

  n = *m;
  n.setValue(MyMoneyMoney(9876, 100));
  QVERIFY(!(n == *m));

  n = *m;
  n.setTransactionId("NoTransaction");
  QVERIFY(!(n == *m));

  n = *m;
  n.setValue("Key", "NoValue");
  QVERIFY(!(n == *m));

  n = *m;
  n.setCostCenterId("NoCostCenter");
  QVERIFY(!(n == *m));
}

void MyMoneySplitTest::testUnaryMinus()
{
  testSetFunctions();

  MyMoneySplit n = -*m;

  QCOMPARE(n.accountId(), QLatin1String("Account"));
  QCOMPARE(n.memo(), QLatin1String("Memo"));
  QCOMPARE(n.reconcileDate(), QDate(1, 2, 3));
  QCOMPARE(n.reconcileFlag(), eMyMoney::Split::State::Cleared);
  QCOMPARE(n.shares(), MyMoneyMoney(-1234, 100));
  QCOMPARE(n.value(), MyMoneyMoney(-3456, 100));
  QCOMPARE(n.id(), QLatin1String("MyID"));
  QCOMPARE(n.payeeId(), QLatin1String("Payee"));
  QList<QString> tagIdList;
  tagIdList << "Tag";
  QCOMPARE(n.tagIdList(), tagIdList);
  QCOMPARE(n.action(), QLatin1String("Action"));
  QCOMPARE(n.transactionId(), QLatin1String("TestTransaction"));
  QCOMPARE(n.value("Key"), QLatin1String("Value"));
  QCOMPARE(n.costCenterId(), QLatin1String("CostCenter"));
}

void MyMoneySplitTest::testAmortization()
{
  QCOMPARE(m->isAmortizationSplit(), false);
  testSetFunctions();
  QCOMPARE(m->isAmortizationSplit(), false);
  m->setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
  QCOMPARE(m->isAmortizationSplit(), true);
}

void MyMoneySplitTest::testValue()
{
  m->setValue(MyMoneyMoney(1, 100));
  m->setShares(MyMoneyMoney(2, 100));
  QCOMPARE(m->value("EUR", "EUR"), MyMoneyMoney(1, 100));
  QCOMPARE(m->value("EUR", "USD"), MyMoneyMoney(2, 100));
}

void MyMoneySplitTest::testSetValue()
{
  QCOMPARE(m->value().isZero(), true);
  QCOMPARE(m->shares().isZero(), true);
  m->setValue(MyMoneyMoney(1, 100), "EUR", "EUR");
  QCOMPARE(m->value(), MyMoneyMoney(1, 100));
  QCOMPARE(m->shares().isZero(), true);
  m->setValue(MyMoneyMoney(3, 100), "EUR", "USD");
  QCOMPARE(m->value(), MyMoneyMoney(1, 100));
  QCOMPARE(m->shares(), MyMoneyMoney(3, 100));
}

void MyMoneySplitTest::testSetAction()
{
  QCOMPARE(m->action().isEmpty(), true);
  m->setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::SellShares);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::Dividend);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::Dividend));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::Yield);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::Yield));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::ReinvestDividend);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::AddShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::RemoveShares);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::AddShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::SplitShares);
  QCOMPARE(m->action(), MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares));
}

void MyMoneySplitTest::testIsAutoCalc()
{
  QCOMPARE(m->isAutoCalc(), false);
  m->setValue(MyMoneyMoney::autoCalc);
  QCOMPARE(m->isAutoCalc(), true);
  m->setShares(MyMoneyMoney::autoCalc);
  QCOMPARE(m->isAutoCalc(), true);
  m->setValue(MyMoneyMoney());
  QCOMPARE(m->isAutoCalc(), true);
  m->setShares(MyMoneyMoney(1, 100));
  QCOMPARE(m->isAutoCalc(), false);
}
