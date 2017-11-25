/***************************************************************************
                          mymoneytransactiontest.cpp
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

#include "mymoneytransaction-test.h"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include <QtTest>

#include "mymoneytestutils.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransaction_p.h"
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
  m->setId("TestID");
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
  split.setId("S00000000");

  // this one should fail, because the ID is invalid
  try {
    m->modifySplit(split);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  // set id to correct value, and check that it worked
  split.setId("S0001");
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

  split.setId("S00000000");
  // this one should fail, because the ID is invalid
  try {
    m->modifySplit(split);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  // set id to correct value, and check that it worked
  split.setId("S0002");
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
  split.setId("S0003");
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

  s1.setAction(MyMoneySplit::ActionAmortization);
  m->modifySplit(s1);
  QVERIFY(m->isLoanPayment() == true);
  s1.setAction(MyMoneySplit::ActionWithdrawal);
  m->modifySplit(s1);
  QVERIFY(m->isLoanPayment() == false);

  s2.setAction(MyMoneySplit::ActionAmortization);
  m->modifySplit(s2);
  QVERIFY(m->isLoanPayment() == true);
  s2.setAction(MyMoneySplit::ActionWithdrawal);
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

void MyMoneyTransactionTest::testWriteXML()
{
  MyMoneyTransaction t;
  t.setPostDate(QDate(2001, 12, 28));
  t.setEntryDate(QDate(2003, 9, 29));
  t.setId("T000000000000000001");
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
  s.setAction(MyMoneySplit::ActionWithdrawal);
  s.setAccountId("A000076");
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  s.setBankID("SPID");
  t.addSplit(s);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("TRANSACTION-CONTAINER");
  doc.appendChild(el);
  t.writeXML(doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement transactionContainer = doc.documentElement();
  QVERIFY(transactionContainer.isElement());
  QCOMPARE(transactionContainer.tagName(), QLatin1String("TRANSACTION-CONTAINER"));
  QVERIFY(transactionContainer.childNodes().size() == 1);
  QVERIFY(transactionContainer.childNodes().at(0).isElement());

  QDomElement transaction = transactionContainer.childNodes().at(0).toElement();
  QCOMPARE(transaction.tagName(), QLatin1String("TRANSACTION"));
  QCOMPARE(transaction.attribute("id"), QLatin1String("T000000000000000001"));
  QCOMPARE(transaction.attribute("postdate"), QLatin1String("2001-12-28"));
  QCOMPARE(transaction.attribute("commodity"), QLatin1String("EUR"));
  QCOMPARE(transaction.attribute("memo"), QLatin1String("Wohnung:Miete"));
  QCOMPARE(transaction.attribute("entrydate"), QLatin1String("2003-09-29"));
  QCOMPARE(transaction.childNodes().size(), 2);

  QVERIFY(transaction.childNodes().at(0).isElement());
  QDomElement splits = transaction.childNodes().at(0).toElement();
  QCOMPARE(splits.tagName(), QLatin1String("SPLITS"));
  QCOMPARE(splits.childNodes().size(), 1);
  QVERIFY(splits.childNodes().at(0).isElement());
  QDomElement split = splits.childNodes().at(0).toElement();
  QCOMPARE(split.tagName(), QLatin1String("SPLIT"));
  QCOMPARE(split.attribute("id"), QLatin1String("S0001"));
  QCOMPARE(split.attribute("payee"), QLatin1String("P000001"));
  QCOMPARE(split.attribute("reconcileflag"), QLatin1String("2"));
  QCOMPARE(split.attribute("shares"), QLatin1String("96379/100"));
  QCOMPARE(split.attribute("reconciledate"), QString());
  QCOMPARE(split.attribute("action"), QLatin1String("Withdrawal"));
  QCOMPARE(split.attribute("bankid"), QLatin1String("SPID"));
  QCOMPARE(split.attribute("account"), QLatin1String("A000076"));
  QCOMPARE(split.attribute("number"), QString());
  QCOMPARE(split.attribute("value"), QLatin1String("96379/100"));
  QCOMPARE(split.attribute("memo"), QString());
  QCOMPARE(split.childNodes().size(), 1);

  QVERIFY(split.childNodes().at(0).isElement());
  QDomElement tag = split.childNodes().at(0).toElement();
  QCOMPARE(tag.tagName(), QLatin1String("TAG"));
  QCOMPARE(tag.attribute("id"), QLatin1String("G000001"));
  QCOMPARE(tag.childNodes().size(), 0);

  QDomElement keyValuePairs = transaction.childNodes().at(1).toElement();
  QCOMPARE(keyValuePairs.tagName(), QLatin1String("KEYVALUEPAIRS"));
  QCOMPARE(keyValuePairs.childNodes().size(), 1);

  QVERIFY(keyValuePairs.childNodes().at(0).isElement());
  QDomElement keyValuePair1 = keyValuePairs.childNodes().at(0).toElement();
  QCOMPARE(keyValuePair1.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair1.attribute("key"), QLatin1String("key"));
  QCOMPARE(keyValuePair1.attribute("value"), QLatin1String("value"));
  QCOMPARE(keyValuePair1.childNodes().size(), 0);
}

void MyMoneyTransactionTest::testReadXML()
{
  MyMoneyTransaction t;

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<TRANSACTION-CONTAINER>\n"
                     " <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"T000000000000000001\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                     "  <SPLITS>\n"
                     "   <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Withdrawal\" bankid=\"SPID\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                     "   <TAG id=\"G000001\"/>\n"
                     "  </SPLITS>\n"
                     "  <KEYVALUEPAIRS>\n"
                     "   <PAIR key=\"key\" value=\"value\" />\n"
                     "  </KEYVALUEPAIRS>\n"
                     " </TRANSACTION>\n"
                     "</TRANSACTION-CONTAINER>\n"
                   );

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<TRANSACTION-CONTAINER>\n"
                        " <TRANS-ACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"T000000000000000001\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                        "  <SPLITS>\n"
                        "   <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Withdrawal\" bankid=\"SPID\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                        "  </SPLITS>\n"
                        "  <KEYVALUEPAIRS>\n"
                        "   <PAIR key=\"key\" value=\"value\" />\n"
                        "  </KEYVALUEPAIRS>\n"
                        " </TRANS-ACTION>\n"
                        "</TRANSACTION-CONTAINER>\n"
                      );

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = MyMoneyTransaction(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  t.setValue("key", "VALUE");
  try {
    t = MyMoneyTransaction(node);
    QVERIFY(t.postDate() == QDate(2001, 12, 28));
    QVERIFY(t.entryDate() == QDate(2003, 9, 29));
    QVERIFY(t.id() == "T000000000000000001");
    QVERIFY(t.memo() == "Wohnung:Miete");
    QVERIFY(t.commodity() == "EUR");
    QVERIFY(t.pairs().count() == 1);
    QVERIFY(t.value("key") == "value");
    QVERIFY(t.splits().count() == 1);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyTransactionTest::testReadXMLEx()
{
  MyMoneyTransaction t;

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<TRANSACTION-CONTAINER>\n"
                     "<TRANSACTION postdate=\"2010-03-05\" memo=\"\" id=\"T000000000000004189\" commodity=\"EUR\" entrydate=\"2010-03-08\" >\n"
                     " <SPLITS>\n"
                     "  <SPLIT payee=\"P000010\" reconciledate=\"\" shares=\"-125000/100\" action=\"Transfer\" bankid=\"A000076-2010-03-05-b6850c0-1\" number=\"\" reconcileflag=\"1\" memo=\"UMBUCHUNG\" value=\"-125000/100\" id=\"S0001\" account=\"A000076\" >\n"
                     "   <KEYVALUEPAIRS>\n"
                     "    <PAIR key=\"kmm-match-split\" value=\"S0002\" />\n"
                     "    <PAIR key=\"kmm-matched-tx\" value=\"&amp;lt;!DOCTYPE MATCH>\n"
                     "    &amp;lt;CONTAINER>\n"
                     "     &amp;lt;TRANSACTION postdate=&quot;2010-03-05&quot; memo=&quot;UMBUCHUNG&quot; id=&quot;&quot; commodity=&quot;EUR&quot; entrydate=&quot;2010-03-08&quot; >\n"
                     "      &amp;lt;SPLITS>\n"
                     "       &amp;lt;SPLIT payee=&quot;P000010&quot; reconciledate=&quot;&quot; shares=&quot;125000/100&quot; action=&quot;Transfer&quot; bankid=&quot;&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;125000/100&quot; id=&quot;S0001&quot; account=&quot;A000087&quot; />\n"
                     "       &amp;lt;SPLIT payee=&quot;P000010&quot; reconciledate=&quot;&quot; shares=&quot;-125000/100&quot; action=&quot;&quot; bankid=&quot;A000076-2010-03-05-b6850c0-1&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;-125000/100&quot; id=&quot;S0002&quot; account=&quot;A000076&quot; />\n"
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
                     "  <SPLIT payee=\"P000010\" reconciledate=\"\" shares=\"125000/100\" action=\"Transfer\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"125000/100\" id=\"S0002\" account=\"A000087\" />\n"
                     " </SPLITS>\n"
                     "</TRANSACTION>\n"
                     "</TRANSACTION-CONTAINER>\n"
                   );
  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = MyMoneyTransaction(node);
    QVERIFY(t.pairs().count() == 0);
    QVERIFY(t.splits().size() == 2);
    QVERIFY(t.splits()[0].pairs().count() == 3);
    QVERIFY(t.splits()[1].pairs().count() == 0);
    QVERIFY(t.splits()[0].isMatched());

    MyMoneyTransaction ti = t.splits()[0].matchedTransaction();
    QVERIFY(ti.pairs().count() == 1);
    QVERIFY(ti.isImported());
    QVERIFY(ti.splits().count() == 2);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyTransactionTest::testHasReferenceTo()
{
  MyMoneyTransaction t;
  t.setPostDate(QDate(2001, 12, 28));
  t.setEntryDate(QDate(2003, 9, 29));
  t.setId("T000000000000000001");
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
  s.setAction(MyMoneySplit::ActionWithdrawal);
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
  s.setAction(MyMoneySplit::ActionSplitShares);
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

void MyMoneyTransactionTest::testElementNames()
{
  for (auto i = (int)Transaction::Element::Split; i <= (int)Transaction::Element::Splits; ++i) {
    auto isEmpty = MyMoneyTransactionPrivate::getElName(static_cast<Transaction::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyTransactionTest::testAttributeNames()
{
  for (auto i = (int)Transaction::Attribute::Name; i < (int)Transaction::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyTransactionPrivate::getAttrName(static_cast<Transaction::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
