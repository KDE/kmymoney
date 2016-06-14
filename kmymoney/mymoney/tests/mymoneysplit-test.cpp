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

#include "mymoneysplit-test.h"

#include <QtTest/QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneySplitTest;

#include "mymoneyexception.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"

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
  m->setShares(MyMoneyMoney(1234, 100));
  m->setValue(MyMoneyMoney(3456, 100));
  m->setId("MyID");
  m->setPayeeId("Payee");
  QList<QString> tagIdList;
  tagIdList << "Tag";
  m->setTagIdList(tagIdList);
  m->setAction("Action");
  m->setTransactionId("TestTransaction");
  m->setValue("Key", "Value");

  QVERIFY(m->accountId() == "Account");
  QVERIFY(m->memo() == "Memo");
  QVERIFY(m->reconcileDate() == QDate(1, 2, 3));
  QVERIFY(m->reconcileFlag() == MyMoneySplit::Cleared);
  QVERIFY(m->shares() == MyMoneyMoney(1234, 100));
  QVERIFY(m->value() == MyMoneyMoney(3456, 100));
  QVERIFY(m->id() == "MyID");
  QVERIFY(m->payeeId() == "Payee");
  QVERIFY(m->tagIdList() == tagIdList);
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
  QVERIFY(n.shares() == MyMoneyMoney(1234, 100));
  QVERIFY(n.value() == MyMoneyMoney(3456, 100));
  QVERIFY(n.id() == "MyID");
  QVERIFY(n.payeeId() == "Payee");
  QList<QString> tagIdList;
  tagIdList << "Tag";
  QVERIFY(m->tagIdList() == tagIdList);
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
  QVERIFY(n.shares() == MyMoneyMoney(1234, 100));
  QVERIFY(n.value() == MyMoneyMoney(3456, 100));
  QVERIFY(n.id() == "MyID");
  QVERIFY(n.payeeId() == "Payee");
  QList<QString> tagIdList;
  tagIdList << "Tag";
  QVERIFY(m->tagIdList() == tagIdList);
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

  n.setShares(MyMoneyMoney(3456, 100));
  QVERIFY(!(n == *m));

  n = *m;
  n.setId("Not My ID");
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
  n.setReconcileFlag(MyMoneySplit::Frozen);
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
  m->setValue(MyMoneyMoney(1, 100));
  m->setShares(MyMoneyMoney(2, 100));
  QVERIFY(m->value("EUR", "EUR") == MyMoneyMoney(1, 100));
  QVERIFY(m->value("EUR", "USD") == MyMoneyMoney(2, 100));
}

void MyMoneySplitTest::testSetValue()
{
  QVERIFY(m->value().isZero());
  QVERIFY(m->shares().isZero());
  m->setValue(MyMoneyMoney(1, 100), "EUR", "EUR");
  QVERIFY(m->value() == MyMoneyMoney(1, 100));
  QVERIFY(m->shares().isZero());
  m->setValue(MyMoneyMoney(3, 100), "EUR", "USD");
  QVERIFY(m->value() == MyMoneyMoney(1, 100));
  QVERIFY(m->shares() == MyMoneyMoney(3, 100));
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
  m->setValue(MyMoneyMoney());
  QVERIFY(m->isAutoCalc() == true);
  m->setShares(MyMoneyMoney(1, 100));
  QVERIFY(m->isAutoCalc() == false);
}

void MyMoneySplitTest::testWriteXML()
{
  MyMoneySplit s;

  s.setPayeeId("P000001");
  QList<QString> tagIdList;
  tagIdList << "G000001";
  s.setTagIdList(tagIdList);
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

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement splitContainer = doc.documentElement();
  QCOMPARE(splitContainer.tagName(), QLatin1String("SPLIT-CONTAINER"));
  QCOMPARE(splitContainer.childNodes().size(), 1);

  QVERIFY(splitContainer.childNodes().at(0).isElement());
  QDomElement split = splitContainer.childNodes().at(0).toElement();
  QCOMPARE(split.tagName(), QLatin1String("SPLIT"));
  QCOMPARE(split.attribute("payee"), QLatin1String("P000001"));
  QCOMPARE(split.attribute("reconcileflag"), QLatin1String("2"));
  QCOMPARE(split.attribute("shares"), QLatin1String("96379/100"));
  QCOMPARE(split.attribute("reconciledate"), QString());
  QCOMPARE(split.attribute("action"), QLatin1String("Deposit"));
  QCOMPARE(split.attribute("bankid"), QLatin1String("SPID"));
  QCOMPARE(split.attribute("account"), QLatin1String("A000076"));
  QCOMPARE(split.attribute("number"), QLatin1String("124"));
  QCOMPARE(split.attribute("value"), QLatin1String("96379/1000"));
  QCOMPARE(split.attribute("memo"), QString());
  QCOMPARE(split.attribute("id"), QString());
  QCOMPARE(split.childNodes().size(), 1);

  QVERIFY(split.childNodes().at(0).isElement());
  QDomElement tag = split.childNodes().at(0).toElement();
  QCOMPARE(tag.tagName(), QLatin1String("TAG"));
  QCOMPARE(tag.attribute("id"), QLatin1String("G000001"));
  QCOMPARE(tag.childNodes().size(), 0);

  QString ref = QString(
                  "<!DOCTYPE TEST>\n"
                  "<SPLIT-CONTAINER>\n"
                  " <SPLIT payee=\"P000001\" reconcileflag=\"2\" shares=\"96379/100\" reconciledate=\"\" action=\"Deposit\" bankid=\"SPID\" account=\"A000076\" number=\"124\" value=\"96379/1000\" memo=\"\" id=\"\">\n"
                  "  <TAG id=\"G000001\"/>\n"
                  " </SPLIT>\n"
                  "</SPLIT-CONTAINER>\n");
}

void MyMoneySplitTest::testReadXML()
{
  MyMoneySplit s;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SPLIT-CONTAINER>\n"
                     " <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"MyMemo\" value=\"96379/1000\" account=\"A000076\">\n"
                     "  <TAG id=\"G000001\"/>\n"
                     " </SPLIT>\n"
                     "</SPLIT-CONTAINER>\n");

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<SPLIT-CONTAINER>\n"
                        " <SPLITS payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"\" value=\"96379/1000\" account=\"A000076\" />\n"
                        " <TAG id=\"G000001\"/>\n"
                        "</SPLIT-CONTAINER>\n");

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  try {
    s = MyMoneySplit(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    s = MyMoneySplit(node);
    QVERIFY(s.id().isEmpty());
    QVERIFY(s.payeeId() == "P000001");
    QList<QString> tagIdList;
    tagIdList << "G000001";
    QVERIFY(s.tagIdList() == tagIdList);
    QVERIFY(s.reconcileDate() == QDate());
    QVERIFY(s.shares() == MyMoneyMoney(96379, 100));
    QVERIFY(s.value() == MyMoneyMoney(96379, 1000));
    QVERIFY(s.number() == "124");
    QVERIFY(s.bankID() == "SPID");
    QVERIFY(s.reconcileFlag() == MyMoneySplit::Reconciled);
    QVERIFY(s.action() == MyMoneySplit::ActionDeposit);
    QVERIFY(s.accountId() == "A000076");
    QVERIFY(s.memo() == "MyMemo");
  } catch (const MyMoneyException &) {
  }

}

void MyMoneySplitTest::testReplaceId()
{
  MyMoneySplit s;
  bool changed;

  s.setPayeeId("P000001");
  s.setAccountId("A000076");

  changed = s.replaceId("X0001", "Y00001");
  QVERIFY(changed == false);
  QVERIFY(s.payeeId() == "P000001");
  QVERIFY(s.accountId() == "A000076");

  changed = s.replaceId("P000002", "P000001");
  QVERIFY(changed == true);
  QVERIFY(s.payeeId() == "P000002");
  QVERIFY(s.accountId() == "A000076");

  changed = s.replaceId("A000079", "A000076");
  QVERIFY(changed == true);
  QVERIFY(s.payeeId() == "P000002");
  QVERIFY(s.accountId() == "A000079");

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SPLIT-CONTAINER>\n"
                     "  <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-125000/100\" action=\"Transfer\" bankid=\"A000076-2010-03-05-b6850c0-1\" number=\"\" reconcileflag=\"1\" memo=\"UMBUCHUNG\" value=\"-125000/100\" id=\"S0001\" account=\"A000076\" >\n"
                     "   <KEYVALUEPAIRS>\n"
                     "    <PAIR key=\"kmm-match-split\" value=\"S0002\" />\n"
                     "    <PAIR key=\"kmm-matched-tx\" value=\"&amp;lt;!DOCTYPE MATCH>\n"
                     "    &amp;lt;CONTAINER>\n"
                     "     &amp;lt;TRANSACTION postdate=&quot;2010-03-05&quot; memo=&quot;UMBUCHUNG&quot; id=&quot;&quot; commodity=&quot;EUR&quot; entrydate=&quot;2010-03-08&quot; >\n"
                     "      &amp;lt;SPLITS>\n"
                     "       &amp;lt;SPLIT payee=&quot;P000010&quot; reconciledate=&quot;&quot; shares=&quot;125000/100&quot; action=&quot;Transfer&quot; bankid=&quot;&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;125000/100&quot; id=&quot;S0001&quot; account=&quot;A000087&quot; />\n"
                     "       &amp;lt;SPLIT payee=&quot;P000011&quot; reconciledate=&quot;&quot; shares=&quot;-125000/100&quot; action=&quot;&quot; bankid=&quot;A000076-2010-03-05-b6850c0-1&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;-125000/100&quot; id=&quot;S0002&quot; account=&quot;A000076&quot; />\n"
                     "      &amp;lt;/SPLITS>\n"
                     "      &amp;lt;KEYVALUEPAIRS>\n"
                     "       &amp;lt;PAIR key=&quot;Imported&quot; value=&quot;true&quot; />\n"
                     "      &amp;lt;/KEYVALUEPAIRS>\n"
                     "     &amp;lt;/TRANSACTION>\n"
                     "    &amp;lt;/CONTAINER>\n"
                     "\" />\n"
                     "    <PAIR key=\"kmm-orig-memo\" value=\"\" />\n"
                     "   </KEYVALUEPAIRS>\n"
                     "  </SPLIT>\n"
                     "</SPLIT-CONTAINER>\n"
                   );
  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    s = MyMoneySplit(node);
    QVERIFY(s.payeeId() == "P000001");
    QVERIFY(s.replaceId("P2", "P1") == false);
    QVERIFY(s.matchedTransaction().splits()[0].payeeId() == "P000010");
    QVERIFY(s.matchedTransaction().splits()[1].payeeId() == "P000011");
    QVERIFY(s.replaceId("P0010", "P000010") == true);
    QVERIFY(s.matchedTransaction().splits()[0].payeeId() == "P0010");
    QVERIFY(s.matchedTransaction().splits()[1].payeeId() == "P000011");
    QVERIFY(s.replaceId("P0011", "P000011") == true);
    QVERIFY(s.matchedTransaction().splits()[0].payeeId() == "P0010");
    QVERIFY(s.matchedTransaction().splits()[1].payeeId() == "P0011");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}
