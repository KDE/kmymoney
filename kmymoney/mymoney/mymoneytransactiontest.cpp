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

#include "mymoneytransactiontest.h"

MyMoneyTransactionTest::MyMoneyTransactionTest()
{
}


void MyMoneyTransactionTest::setUp()
{
  m = new MyMoneyTransaction();
}

void MyMoneyTransactionTest::tearDown()
{
  delete m;
}

void MyMoneyTransactionTest::testEmptyConstructor()
{
  CPPUNIT_ASSERT(m->id().isEmpty());
  CPPUNIT_ASSERT(m->entryDate() == QDate());
  CPPUNIT_ASSERT(m->memo().isEmpty());
  CPPUNIT_ASSERT(m->splits().count() == 0);
}

void MyMoneyTransactionTest::testSetFunctions()
{
  m->setMemo("Memo");
  m->setPostDate(QDate(1, 2, 3));

  CPPUNIT_ASSERT(m->postDate() == QDate(1, 2, 3));
  CPPUNIT_ASSERT(m->memo() == "Memo");
}

void MyMoneyTransactionTest::testConstructor()
{
  testSetFunctions();
  MyMoneyTransaction a("ID", *m);

  CPPUNIT_ASSERT(a.id() == "ID");
  CPPUNIT_ASSERT(a.entryDate() == QDate::currentDate());
  CPPUNIT_ASSERT(a.memo() == "Memo");
  CPPUNIT_ASSERT(a.postDate() == QDate(1, 2, 3));
}

void MyMoneyTransactionTest::testCopyConstructor()
{
  testConstructor();
  MyMoneyTransaction a("ID", *m);
  a.setValue("Key", "Value");

  MyMoneyTransaction n(a);

  CPPUNIT_ASSERT(n.id() == "ID");
  CPPUNIT_ASSERT(n.entryDate() == QDate::currentDate());
  CPPUNIT_ASSERT(n.memo() == "Memo");
  CPPUNIT_ASSERT(n.postDate() == QDate(1, 2, 3));
  CPPUNIT_ASSERT(n.value("Key") == "Value");
}

void MyMoneyTransactionTest::testAssignmentConstructor()
{
  testConstructor();
  MyMoneyTransaction a("ID", *m);
  a.setValue("Key", "Value");

  MyMoneyTransaction n;

  n = a;

  CPPUNIT_ASSERT(n.id() == "ID");
  CPPUNIT_ASSERT(n.entryDate() == QDate::currentDate());
  CPPUNIT_ASSERT(n.memo() == "Memo");
  CPPUNIT_ASSERT(n.postDate() == QDate(1, 2, 3));
  CPPUNIT_ASSERT(n.value("Key") == "Value");
}

void MyMoneyTransactionTest::testEquality()
{
  testConstructor();

  MyMoneyTransaction n(*m);

  CPPUNIT_ASSERT(n == *m);
  CPPUNIT_ASSERT(!(n != *m));
}

void MyMoneyTransactionTest::testInequality()
{
  testConstructor();

  MyMoneyTransaction n(*m);

  n.setPostDate(QDate(1, 1, 1));
  CPPUNIT_ASSERT(!(n == *m));
  CPPUNIT_ASSERT(n != *m);

  n = *m;
  n.setValue("key", "value");
  CPPUNIT_ASSERT(!(n == *m));
  CPPUNIT_ASSERT(n != *m);
}

void MyMoneyTransactionTest::testAddSplits()
{
  m->setId("TestID");
  MyMoneySplit split1, split2;
  split1.setAccountId("A000001");
  split2.setAccountId("A000002");
  split1.setValue(MyMoneyMoney(100));
  split2.setValue(MyMoneyMoney(200));

  try {
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
    CPPUNIT_ASSERT(m->accountReferenced("A000002") == false);
    m->addSplit(split1);
    m->addSplit(split2);
    CPPUNIT_ASSERT(m->splitCount() == 2);
    CPPUNIT_ASSERT(m->splits()[0].accountId() == "A000001");
    CPPUNIT_ASSERT(m->splits()[1].accountId() == "A000002");
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
    CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
    CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");
    CPPUNIT_ASSERT(m->splits()[1].id() == "S0002");
    CPPUNIT_ASSERT(split1.id() == "S0001");
    CPPUNIT_ASSERT(split2.id() == "S0002");
    CPPUNIT_ASSERT(m->splits()[0].transactionId() == "TestID");
    CPPUNIT_ASSERT(m->splits()[1].transactionId() == "TestID");
    CPPUNIT_ASSERT(split1.transactionId() == "TestID");
    CPPUNIT_ASSERT(split2.transactionId() == "TestID");

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // try to add split with assigned ID
  try {
    m->addSplit(split1);
    CPPUNIT_FAIL("Exception expected!");

  } catch (MyMoneyException *e) {
    delete e;
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
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // set id to correct value, and check that it worked
  split.setId("S0001");
  try {
    m->modifySplit(split);
    CPPUNIT_ASSERT(m->splitCount() == 2);
    CPPUNIT_ASSERT(m->splits()[0].accountId() == "A000003");
    CPPUNIT_ASSERT(m->splits()[1].accountId() == "A000002");
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
    CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
    CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");
    CPPUNIT_ASSERT(m->splits()[1].id() == "S0002");

    CPPUNIT_ASSERT(split.id() == "S0001");
    CPPUNIT_ASSERT(split.accountId() == "A000003");

  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception!");
    delete e;
  }
}

void MyMoneyTransactionTest::testDeleteSplits()
{
  testAddSplits();
  MyMoneySplit split;

  // add a third split
  split.setAccountId("A000003");
  split.setValue(MyMoneyMoney(300));
  try {
    m->addSplit(split);
  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception!");
    delete e;
  }

  split.setId("S00000000");
  // this one should fail, because the ID is invalid
  try {
    m->modifySplit(split);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // set id to correct value, and check that it worked
  split.setId("S0002");
  try {
    m->removeSplit(split);
    CPPUNIT_ASSERT(m->splitCount() == 2);
    CPPUNIT_ASSERT(m->splits()[0].accountId() == "A000001");
    CPPUNIT_ASSERT(m->accountReferenced("A000002") == false);
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
    CPPUNIT_ASSERT(m->accountReferenced("A000003") == true);
    CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");

  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception!");
    delete e;
  }

  // set id to the other correct value, and check that it worked
  split.setId("S0003");
  try {
    m->removeSplit(split);
    CPPUNIT_ASSERT(m->splitCount() == 1);
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
    CPPUNIT_ASSERT(m->accountReferenced("A000003") == false);
    CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");

  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception!");
    delete e;
  }
}

void MyMoneyTransactionTest::testDeleteAllSplits()
{
  testAddSplits();

  try {
    m->removeSplits();
    CPPUNIT_ASSERT(m->splitCount() == 0);
  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception!");
    delete e;
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
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // this one should be found
  try {
    split = m->splitByAccount(QString("A000002"));
    CPPUNIT_ASSERT(split.id() == "S0002");

  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception!");
    delete e;
  }

  // this one should be found also
  try {
    split = m->splitByAccount(QString("A000002"), false);
    CPPUNIT_ASSERT(split.id() == "S0001");
  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception!");
    delete e;
  }
}

void MyMoneyTransactionTest::testSplitSum()
{
  CPPUNIT_ASSERT(m->splitSum().isZero());

  testAddSplits();

  MyMoneySplit s1, s2;

  s1 = m->splits()[0];
  s1.setValue(MyMoneyMoney(0));
  s2 = m->splits()[1];
  s2.setValue(MyMoneyMoney(0));

  m->modifySplit(s1);
  m->modifySplit(s2);
  CPPUNIT_ASSERT(m->splitSum().isZero());

  s1.setValue(MyMoneyMoney(1234));
  m->modifySplit(s1);
  CPPUNIT_ASSERT(m->splitSum() == MyMoneyMoney(1234));

  s2.setValue(MyMoneyMoney(-1234));
  m->modifySplit(s2);
  CPPUNIT_ASSERT(m->splitSum().isZero());

  s1.setValue(MyMoneyMoney(5678));
  m->modifySplit(s1);
  CPPUNIT_ASSERT(m->splitSum() == MyMoneyMoney(4444));
}

void MyMoneyTransactionTest::testIsLoanPayment()
{
  testAddSplits();
  CPPUNIT_ASSERT(m->isLoanPayment() == false);

  MyMoneySplit s1, s2;
  s1 = m->splits()[0];
  s2 = m->splits()[1];

  s1.setAction(MyMoneySplit::ActionAmortization);
  m->modifySplit(s1);
  CPPUNIT_ASSERT(m->isLoanPayment() == true);
  s1.setAction(MyMoneySplit::ActionWithdrawal);
  m->modifySplit(s1);
  CPPUNIT_ASSERT(m->isLoanPayment() == false);

  s2.setAction(MyMoneySplit::ActionAmortization);
  m->modifySplit(s2);
  CPPUNIT_ASSERT(m->isLoanPayment() == true);
  s2.setAction(MyMoneySplit::ActionWithdrawal);
  m->modifySplit(s2);
  CPPUNIT_ASSERT(m->isLoanPayment() == false);
}

void MyMoneyTransactionTest::testAddDuplicateAccount()
{
  testAddSplits();

  MyMoneySplit split1, split2;
  split1.setAccountId("A000001");
  split2.setAccountId("A000002");
  split1.setValue(MyMoneyMoney(100));
  split2.setValue(MyMoneyMoney(200));

  try {
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
    CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
    m->addSplit(split1);
    m->addSplit(split2);
    CPPUNIT_ASSERT(m->splitCount() == 2);
    CPPUNIT_ASSERT(m->splits()[0].accountId() == "A000001");
    CPPUNIT_ASSERT(m->splits()[1].accountId() == "A000002");
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
    CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
    CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");
    CPPUNIT_ASSERT(m->splits()[1].id() == "S0002");
    CPPUNIT_ASSERT(split1.id() == "S0001");
    CPPUNIT_ASSERT(split2.id() == "S0002");

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  CPPUNIT_ASSERT(m->splits()[0].value() == MyMoneyMoney(200));
  CPPUNIT_ASSERT(m->splits()[1].value() == MyMoneyMoney(400));
}

void MyMoneyTransactionTest::testModifyDuplicateAccount()
{
  testAddSplits();
  MyMoneySplit split;

  split = m->splitByAccount(QString("A000002"));
  split.setAccountId("A000001");
  try {
    m->modifySplit(split);
    CPPUNIT_ASSERT(m->splitCount() == 1);
    CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
    CPPUNIT_ASSERT(m->splits()[0].value() == MyMoneyMoney(300));

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}
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
  s.setShares(MyMoneyMoney(96379, 100));
  s.setValue(MyMoneyMoney(96379, 100));
  s.setAction(MyMoneySplit::ActionWithdrawal);
  s.setAccountId("A000076");
  s.setReconcileFlag(MyMoneySplit::Reconciled);
  s.setBankID("SPID");
  t.addSplit(s);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("TRANSACTION-CONTAINER");
  doc.appendChild(el);
  t.writeXML(doc, el);

  QString ref = QString(
                  "<!DOCTYPE TEST>\n"
                  "<TRANSACTION-CONTAINER>\n"
                  " <TRANSACTION postdate=\"2001-12-28\" commodity=\"EUR\" memo=\"Wohnung:Miete\" id=\"T000000000000000001\" entrydate=\"2003-09-29\" >\n"
                  "  <SPLITS>\n"
                  "   <SPLIT payee=\"P000001\" reconcileflag=\"2\" shares=\"96379/100\" reconciledate=\"\" action=\"Withdrawal\" bankid=\"SPID\" account=\"A000076\" number=\"\" value=\"96379/100\" memo=\"\" id=\"S0001\" />\n"
                  "  </SPLITS>\n"
                  "  <KEYVALUEPAIRS>\n"
                  "   <PAIR key=\"key\" value=\"value\" />\n"
                  "  </KEYVALUEPAIRS>\n"
                  " </TRANSACTION>\n"
                  "</TRANSACTION-CONTAINER>\n"

                );

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  ref.replace(QString(" />\n"), QString("/>\n"));
  ref.replace(QString(" >\n"), QString(">\n"));
#endif

  //qDebug("ref = '%s'", qPrintable(ref));
  //qDebug("doc = '%s'", qPrintable(doc.toString()));

  CPPUNIT_ASSERT(doc.toString() == ref);
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
    CPPUNIT_FAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  t.setValue("key", "VALUE");
  try {
    t = MyMoneyTransaction(node);
    CPPUNIT_ASSERT(t.m_postDate == QDate(2001, 12, 28));
    CPPUNIT_ASSERT(t.m_entryDate == QDate(2003, 9, 29));
    CPPUNIT_ASSERT(t.id() == "T000000000000000001");
    CPPUNIT_ASSERT(t.m_memo == "Wohnung:Miete");
    CPPUNIT_ASSERT(t.m_commodity == "EUR");
    CPPUNIT_ASSERT(t.pairs().count() == 1);
    CPPUNIT_ASSERT(t.value("key") == "value");
    CPPUNIT_ASSERT(t.splits().count() == 1);
  } catch (MyMoneyException *e) {
    delete e;
    CPPUNIT_FAIL("Unexpected exception");
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
    CPPUNIT_ASSERT(t.pairs().count() == 0);
    CPPUNIT_ASSERT(t.splits().size() == 2);
    CPPUNIT_ASSERT(t.splits()[0].pairs().count() == 3);
    CPPUNIT_ASSERT(t.splits()[1].pairs().count() == 0);
    CPPUNIT_ASSERT(t.splits()[0].isMatched());

    MyMoneyTransaction ti = t.splits()[0].matchedTransaction();
    CPPUNIT_ASSERT(ti.pairs().count() == 1);
    CPPUNIT_ASSERT(ti.isImported());
    CPPUNIT_ASSERT(ti.splits().count() == 2);
  } catch(MyMoneyException *e) {
    delete e;
    CPPUNIT_FAIL("Unexpected exception");
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
  s.setShares(MyMoneyMoney(96379, 100));
  s.setValue(MyMoneyMoney(96379, 100));
  s.setAction(MyMoneySplit::ActionWithdrawal);
  s.setAccountId("A000076");
  s.setReconcileFlag(MyMoneySplit::Reconciled);
  t.addSplit(s);

  CPPUNIT_ASSERT(t.hasReferenceTo("EUR") == true);
  CPPUNIT_ASSERT(t.hasReferenceTo("P000001") == true);
  CPPUNIT_ASSERT(t.hasReferenceTo("A000076") == true);
}

void MyMoneyTransactionTest::testAutoCalc()
{
  CPPUNIT_ASSERT(m->hasAutoCalcSplit() == false);
  testAddSplits();
  CPPUNIT_ASSERT(m->hasAutoCalcSplit() == false);
  MyMoneySplit split;

  split = m->splits()[0];
  split.setShares(MyMoneyMoney::autoCalc);
  split.setValue(MyMoneyMoney::autoCalc);
  m->modifySplit(split);

  CPPUNIT_ASSERT(m->hasAutoCalcSplit() == true);
}

void MyMoneyTransactionTest::testIsStockSplit()
{
  CPPUNIT_ASSERT(m->isStockSplit() == false);
  testAddSplits();
  CPPUNIT_ASSERT(m->isStockSplit() == false);
  m->removeSplits();
  MyMoneySplit s;
  s.setShares(MyMoneyMoney(1, 2));
  s.setAction(MyMoneySplit::ActionSplitShares);
  s.setAccountId("A0001");
  m->addSplit(s);
  CPPUNIT_ASSERT(m->isStockSplit() == true);
}

void MyMoneyTransactionTest::testAddMissingAccountId()
{
  MyMoneySplit s;
  s.setShares(MyMoneyMoney(1, 2));
  try {
    m->addSplit(s);
    CPPUNIT_FAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyTransactionTest::testModifyMissingAccountId()
{
  testAddSplits();
  MyMoneySplit s = m->splits()[0];
  s.setAccountId(QString());

  try {
    m->modifySplit(s);
    CPPUNIT_FAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

