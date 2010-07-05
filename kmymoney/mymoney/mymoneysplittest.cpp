/***************************************************************************
                          mymoneysplittest.cpp
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

#include "mymoneysplittest.h"

#include <QtTest/QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneySplitTest;

#include "mymoneyexception.h"
#include "mymoneysplit.h"

QTEST_MAIN(MyMoneySplitTest)

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
  QVERIFY(m->accountId().isEmpty());
  QVERIFY(m->id().isEmpty());
  QVERIFY(m->memo().isEmpty());
  QVERIFY(m->action().isEmpty());
  QVERIFY(m->shares().isZero());
  QVERIFY(m->value().isZero());
  QVERIFY(m->reconcileFlag() == MyMoneySplit::NotReconciled);
  QVERIFY(m->reconcileDate() == QDate());
  QVERIFY(m->transactionId().isEmpty());
}

void MyMoneySplitTest::testSetFunctions()
{
  m->setAccountId("Account");
  m->setMemo("Memo");
  m->setReconcileDate(QDate(1, 2, 3));
  m->setReconcileFlag(MyMoneySplit::Cleared);
  m->setShares(MyMoneyMoney(1234));
  m->setValue(MyMoneyMoney(3456));
  m->setId("MyID");
  m->setPayeeId("Payee");
  m->setAction("Action");
  m->setTransactionId("TestTransaction");
  m->setValue("Key", "Value");

  QVERIFY(m->accountId() == "Account");
  QVERIFY(m->memo() == "Memo");
  QVERIFY(m->reconcileDate() == QDate(1, 2, 3));
  QVERIFY(m->reconcileFlag() == MyMoneySplit::Cleared);
  QVERIFY(m->shares() == MyMoneyMoney(1234));
  QVERIFY(m->value() == MyMoneyMoney(3456));
  QVERIFY(m->id() == "MyID");
  QVERIFY(m->payeeId() == "Payee");
  QVERIFY(m->action() == "Action");
  QVERIFY(m->transactionId() == "TestTransaction");
  QVERIFY(m->value("Key") == "Value");
}


void MyMoneySplitTest::testCopyConstructor()
{
  testSetFunctions();

  MyMoneySplit n(*m);

  QVERIFY(n.accountId() == "Account");
  QVERIFY(n.memo() == "Memo");
  QVERIFY(n.reconcileDate() == QDate(1, 2, 3));
  QVERIFY(n.reconcileFlag() == MyMoneySplit::Cleared);
  QVERIFY(n.shares() == MyMoneyMoney(1234));
  QVERIFY(n.value() == MyMoneyMoney(3456));
  QVERIFY(n.id() == "MyID");
  QVERIFY(n.payeeId() == "Payee");
  QVERIFY(n.action() == "Action");
  QVERIFY(n.transactionId() == "TestTransaction");
  QVERIFY(n.value("Key") == "Value");
}

void MyMoneySplitTest::testAssignmentConstructor()
{
  testSetFunctions();

  MyMoneySplit n;

  n = *m;

  QVERIFY(n.accountId() == "Account");
  QVERIFY(n.memo() == "Memo");
  QVERIFY(n.reconcileDate() == QDate(1, 2, 3));
  QVERIFY(n.reconcileFlag() == MyMoneySplit::Cleared);
  QVERIFY(n.shares() == MyMoneyMoney(1234));
  QVERIFY(n.value() == MyMoneyMoney(3456));
  QVERIFY(n.id() == "MyID");
  QVERIFY(n.payeeId() == "Payee");
  QVERIFY(n.action() == "Action");
  QVERIFY(n.transactionId() == "TestTransaction");
  QVERIFY(n.value("Key") == "Value");
}

void MyMoneySplitTest::testEquality()
{
  testSetFunctions();

  MyMoneySplit n(*m);

  QVERIFY(n == *m);
}

void MyMoneySplitTest::testInequality()
{
  testSetFunctions();

  MyMoneySplit n(*m);

  n.setShares(MyMoneyMoney(3456));
  QVERIFY(!(n == *m));

  n = *m;
  n.setId("Not My ID");
  QVERIFY(!(n == *m));

  n = *m;
  n.setPayeeId("No payee");
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
  n.setReconcileFlag(MyMoneySplit::Frozen);
  QVERIFY(!(n == *m));

  n = *m;
  n.setShares(MyMoneyMoney(4567));
  QVERIFY(!(n == *m));

  n = *m;
  n.setValue(MyMoneyMoney(9876));
  QVERIFY(!(n == *m));

  n = *m;
  n.setTransactionId("NoTransaction");
  QVERIFY(!(n == *m));

  n = *m;
  n.setValue("Key", "NoValue");
  QVERIFY(!(n == *m));
}


void MyMoneySplitTest::testAmortization()
{
  QVERIFY(m->isAmortizationSplit() == false);
  testSetFunctions();
  QVERIFY(m->isAmortizationSplit() == false);
  m->setAction(MyMoneySplit::ActionAmortization);
  QVERIFY(m->isAmortizationSplit() == true);
}

void MyMoneySplitTest::testValue()
{
  m->setValue(MyMoneyMoney(1));
  m->setShares(MyMoneyMoney(2));
  QVERIFY(m->value("EUR", "EUR") == MyMoneyMoney(1));
  QVERIFY(m->value("EUR", "USD") == MyMoneyMoney(2));
}

void MyMoneySplitTest::testSetValue()
{
  QVERIFY(m->value().isZero());
  QVERIFY(m->shares().isZero());
  m->setValue(MyMoneyMoney(1), "EUR", "EUR");
  QVERIFY(m->value() == MyMoneyMoney(1));
  QVERIFY(m->shares().isZero());
  m->setValue(MyMoneyMoney(3), "EUR", "USD");
  QVERIFY(m->value() == MyMoneyMoney(1));
  QVERIFY(m->shares() == MyMoneyMoney(3));
}

void MyMoneySplitTest::testSetAction()
{
  QVERIFY(m->action().isEmpty());
  m->setAction(MyMoneySplit::BuyShares);
  QVERIFY(m->action() == MyMoneySplit::ActionBuyShares);
  m->setAction(MyMoneySplit::SellShares);
  QVERIFY(m->action() == MyMoneySplit::ActionBuyShares);
  m->setAction(MyMoneySplit::Dividend);
  QVERIFY(m->action() == MyMoneySplit::ActionDividend);
  m->setAction(MyMoneySplit::Yield);
  QVERIFY(m->action() == MyMoneySplit::ActionYield);
  m->setAction(MyMoneySplit::ReinvestDividend);
  QVERIFY(m->action() == MyMoneySplit::ActionReinvestDividend);
  m->setAction(MyMoneySplit::AddShares);
  QVERIFY(m->action() == MyMoneySplit::ActionAddShares);
  m->setAction(MyMoneySplit::RemoveShares);
  QVERIFY(m->action() == MyMoneySplit::ActionAddShares);
  m->setAction(MyMoneySplit::SplitShares);
  QVERIFY(m->action() == MyMoneySplit::ActionSplitShares);
}

void MyMoneySplitTest::testIsAutoCalc()
{
  QVERIFY(m->isAutoCalc() == false);
  m->setValue(MyMoneyMoney::autoCalc);
  QVERIFY(m->isAutoCalc() == true);
  m->setShares(MyMoneyMoney::autoCalc);
  QVERIFY(m->isAutoCalc() == true);
  m->setValue(MyMoneyMoney(0));
  QVERIFY(m->isAutoCalc() == true);
  m->setShares(MyMoneyMoney(1));
  QVERIFY(m->isAutoCalc() == false);
}

void MyMoneySplitTest::testWriteXML()
{
  MyMoneySplit s;

  s.setPayeeId("P000001");
  s.setShares(MyMoneyMoney(96379, 100));
  s.setValue(MyMoneyMoney(96379, 1000));
  s.setAccountId("A000076");
  s.setNumber("124");
  s.setBankID("SPID");
  s.setAction(MyMoneySplit::ActionDeposit);
  s.setReconcileFlag(MyMoneySplit::Reconciled);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("SPLIT-CONTAINER");
  doc.appendChild(el);
  s.writeXML(doc, el);

  QString ref = QString(
                  "<!DOCTYPE TEST>\n"
                  "<SPLIT-CONTAINER>\n"
                  " <SPLIT payee=\"P000001\" reconcileflag=\"2\" shares=\"96379/100\" reconciledate=\"\" action=\"Deposit\" bankid=\"SPID\" account=\"A000076\" number=\"124\" value=\"96379/1000\" memo=\"\" id=\"\" />\n"
                  "</SPLIT-CONTAINER>\n");

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  ref.replace(QString(" />\n"), QString("/>\n"));
  ref.replace(QString(" >\n"), QString(">\n"));
#endif

  //qDebug("ref = '%s'", qPrintable(ref));
  //qDebug("doc = '%s'", qPrintable(doc.toString()));

  QVERIFY(doc.toString() == ref);
}

void MyMoneySplitTest::testReadXML()
{
  MyMoneySplit s;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SPLIT-CONTAINER>\n"
                     " <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"MyMemo\" value=\"96379/1000\" account=\"A000076\" />\n"
                     "</SPLIT-CONTAINER>\n");

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<SPLIT-CONTAINER>\n"
                        " <SPLITS payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"\" value=\"96379/1000\" account=\"A000076\" />\n"
                        "</SPLIT-CONTAINER>\n");

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  try {
    s = MyMoneySplit(node);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    s = MyMoneySplit(node);
    QVERIFY(s.id().isEmpty());
    QVERIFY(s.payeeId() == "P000001");
    QVERIFY(s.reconcileDate() == QDate());
    QVERIFY(s.shares() == MyMoneyMoney(96379, 100));
    QVERIFY(s.value() == MyMoneyMoney(96379, 1000));
    QVERIFY(s.number() == "124");
    QVERIFY(s.bankID() == "SPID");
    QVERIFY(s.reconcileFlag() == MyMoneySplit::Reconciled);
    QVERIFY(s.action() == MyMoneySplit::ActionDeposit);
    QVERIFY(s.accountId() == "A000076");
    QVERIFY(s.memo() == "MyMemo");
  } catch (MyMoneyException *e) {
    delete e;
  }

}

#include "mymoneysplittest.moc"

