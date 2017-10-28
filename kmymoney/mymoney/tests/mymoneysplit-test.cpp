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

#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"

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
  m->setId("MyID");
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
  m->setAction(MyMoneySplit::ActionAmortization);
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
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionBuyShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::SellShares);
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionBuyShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::Dividend);
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionDividend));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::Yield);
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionYield));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::ReinvestDividend);
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionReinvestDividend));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionAddShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::RemoveShares);
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionAddShares));
  m->setAction(eMyMoney::Split::InvestmentTransactionType::SplitShares);
  QCOMPARE(m->action(), QLatin1String(MyMoneySplit::ActionSplitShares));
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
  s.setCostCenterId("C000005");
  s.setNumber("124");
  s.setBankID("SPID");
  s.setAction(MyMoneySplit::ActionDeposit);
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("SPLIT-CONTAINER");
  doc.appendChild(el);
  s.writeXML(doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement splitContainer = doc.documentElement();
  QCOMPARE(splitContainer.tagName(), QLatin1String("SPLIT-CONTAINER"));
  QCOMPARE(splitContainer.childNodes().size(), 1);

  QCOMPARE(splitContainer.childNodes().at(0).isElement(), true);
  QDomElement split = splitContainer.childNodes().at(0).toElement();
  QCOMPARE(split.tagName(), QLatin1String("SPLIT"));
  QCOMPARE(split.attribute("payee"), QLatin1String("P000001"));
  QCOMPARE(split.attribute("reconcileflag"), QLatin1String("2"));
  QCOMPARE(split.attribute("shares"), QLatin1String("96379/100"));
  QCOMPARE(split.attribute("reconciledate"), QString());
  QCOMPARE(split.attribute("action"), QLatin1String("Deposit"));
  QCOMPARE(split.attribute("bankid"), QLatin1String("SPID"));
  QCOMPARE(split.attribute("account"), QLatin1String("A000076"));
  QCOMPARE(split.attribute("costcenter"), QLatin1String("C000005"));
  QCOMPARE(split.attribute("number"), QLatin1String("124"));
  QCOMPARE(split.attribute("value"), QLatin1String("96379/1000"));
  QCOMPARE(split.attribute("memo"), QString());
  QCOMPARE(split.attribute("id"), QString());
  QCOMPARE(split.childNodes().size(), 1);

  QCOMPARE(split.childNodes().at(0).isElement(), true);
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
                     " <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"MyMemo\" value=\"96379/1000\" account=\"A000076\" costcenter=\"C000005\">\n"
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
    QCOMPARE(s.id().isEmpty(), true);
    QCOMPARE(s.payeeId(), QLatin1String("P000001"));
    QList<QString> tagIdList;
    tagIdList << QLatin1String("G000001");
    QCOMPARE(s.tagIdList(), tagIdList);
    QCOMPARE(s.reconcileDate(), QDate());
    QCOMPARE(s.shares(), MyMoneyMoney(96379, 100));
    QCOMPARE(s.value(), MyMoneyMoney(96379, 1000));
    QCOMPARE(s.number(), QLatin1String("124"));
    QCOMPARE(s.bankID(), QLatin1String("SPID"));
    QCOMPARE(s.reconcileFlag(), eMyMoney::Split::State::Reconciled);
    QCOMPARE(s.action(), QLatin1String(MyMoneySplit::ActionDeposit));
    QCOMPARE(s.accountId(), QLatin1String("A000076"));
    QCOMPARE(s.costCenterId(), QLatin1String("C000005"));
    QCOMPARE(s.memo(), QLatin1String("MyMemo"));
  } catch (const MyMoneyException &) {
  }

}

void MyMoneySplitTest::testReplaceId()
{
  MyMoneySplit s;
  bool changed;

  s.setPayeeId("P000001");
  s.setAccountId("A000076");
  s.setCostCenterId("C000005");

  changed = s.replaceId("X0001", "Y00001");
  QCOMPARE(changed, false);
  QCOMPARE(s.payeeId(), QLatin1String("P000001"));
  QCOMPARE(s.accountId(), QLatin1String("A000076"));
  QCOMPARE(s.costCenterId(), QLatin1String("C000005"));

  changed = s.replaceId("P000002", "P000001");
  QCOMPARE(changed, true);
  QCOMPARE(s.payeeId(), QLatin1String("P000002"));
  QCOMPARE(s.accountId(), QLatin1String("A000076"));
  QCOMPARE(s.costCenterId(), QLatin1String("C000005"));

  changed = s.replaceId("A000079", "A000076");
  QCOMPARE(changed, true);
  QCOMPARE(s.payeeId(), QLatin1String("P000002"));
  QCOMPARE(s.accountId(), QLatin1String("A000079"));
  QCOMPARE(s.costCenterId(), QLatin1String("C000005"));

  changed = s.replaceId("C000006", "C000005");
  QCOMPARE(changed, true);
  QCOMPARE(s.payeeId(), QLatin1String("P000002"));
  QCOMPARE(s.accountId(), QLatin1String("A000079"));
  QCOMPARE(s.costCenterId(), QLatin1String("C000006"));

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
    QCOMPARE(s.payeeId(), QLatin1String("P000001"));
    QCOMPARE(s.replaceId("P2", "P1"), false);
    QCOMPARE(s.matchedTransaction().splits()[0].payeeId(), QLatin1String("P000010"));
    QCOMPARE(s.matchedTransaction().splits()[1].payeeId(), QLatin1String("P000011"));
    QCOMPARE(s.replaceId("P0010", "P000010"), true);
    QCOMPARE(s.matchedTransaction().splits()[0].payeeId(), QLatin1String("P0010"));
    QCOMPARE(s.matchedTransaction().splits()[1].payeeId(), QLatin1String("P000011"));
    QCOMPARE(s.replaceId("P0011", "P000011"), true);
    QCOMPARE(s.matchedTransaction().splits()[0].payeeId(), QLatin1String("P0010"));
    QCOMPARE(s.matchedTransaction().splits()[1].payeeId(), QLatin1String("P0011"));

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySplitTest::testElementNames()
{
  for (auto i = (int)MyMoneySplit::Element::Split; i <= (int)MyMoneySplit::Element::KeyValuePairs; ++i) {
    auto isEmpty = MyMoneySplit::getElName(static_cast<MyMoneySplit::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneySplitTest::testAttributeNames()
{
  for (auto i = (int)MyMoneySplit::Attribute::ID; i < (int)MyMoneySplit::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneySplit::getAttrName(static_cast<MyMoneySplit::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}

