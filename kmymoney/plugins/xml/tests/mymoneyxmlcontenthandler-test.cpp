/*
    SPDX-FileCopyrightText: 2002-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2016 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyxmlcontenthandler-test.h"

#include <QTest>
#include "../mymoneystoragexml.cpp"
#include "mymoneyobject_p.h"
#include "mymoneyobject.h"
#include "mymoneyexception.h"
#include "tests/testutilities.h"
using namespace test;

QTEST_GUILESS_MAIN(MyMoneyXmlContentHandlerTest)

class TestMyMoneyObjectPrivate : public MyMoneyObjectPrivate
{
};

class TestMyMoneyObject : public MyMoneyObject
{
  Q_DECLARE_PRIVATE(TestMyMoneyObject)

public:
  TestMyMoneyObject() : MyMoneyObject(*new MyMoneyObjectPrivate) {}
  TestMyMoneyObject(const QString &id) :
    MyMoneyObject(*new MyMoneyObjectPrivate, id)
  {
  }

  TestMyMoneyObject(const TestMyMoneyObject &other) :
    MyMoneyObject(*new MyMoneyObjectPrivate(*other.d_func()), other.id())
  {
  }

  TestMyMoneyObject(TestMyMoneyObject &&other) : TestMyMoneyObject()
  {
    swap(*this, other);
  }

  TestMyMoneyObject & operator=(TestMyMoneyObject other);
  friend void swap(TestMyMoneyObject& first, TestMyMoneyObject& second);
  ~TestMyMoneyObject() final override
  {
  }

  bool hasReferenceTo(const QString&) const final override
  {
    return false;
  }
  QSet<QString> referencedObjects() const final override
  {
    return {};
  }
  static TestMyMoneyObject readBaseXML(const QDomElement &node, bool forceId = true)
  {
    TestMyMoneyObject obj(node.attribute(QStringLiteral("id")));
    if (obj.id().isEmpty() && forceId)
      throw MYMONEYEXCEPTION_CSTRING("Node has no ID");
    return obj;
  }
};

void swap(TestMyMoneyObject& first, TestMyMoneyObject& second)
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

TestMyMoneyObject & TestMyMoneyObject::operator=(TestMyMoneyObject other)
{
  swap(*this, other);
  return *this;
}

void MyMoneyXmlContentHandlerTest::init()
{
}

void MyMoneyXmlContentHandlerTest::cleanup()
{
  MyMoneyFile::instance()->unload();
}

void MyMoneyXmlContentHandlerTest::setupAccounts()
{
  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acIncome = (MyMoneyFile::instance()->income().id());
  curBase = makeBaseCurrency();
  acChecking = makeAccount("A000076","Checking Account 1", eMyMoney::Account::Type::Checkings, moZero, QDate(2014, 5, 15), acAsset);
  acTransfer = makeAccount("A000276","Checking Account 2", eMyMoney::Account::Type::Checkings, moZero, QDate(2014, 5, 15), acAsset);
}


void MyMoneyXmlContentHandlerTest::readMyMoneyObject()
{
  TestMyMoneyObject t;

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<TRANSACTION-CONTAINER>\n"
                     " <MYMONEYOBJECT id=\"T000000000000000001\" >\n"
                     " </MYMONEYOBJECT>\n"
                     "</TRANSACTION-CONTAINER>\n"
                   );

  QString ref_false1 = QString(
                         "<!DOCTYPE TEST>\n"
                         "<TRANSACTION-CONTAINER>\n"
                         " <MYMONEYOBJECT id=\"\" >\n"
                         " </MYMONEYOBJECT>\n"
                         "</TRANSACTION-CONTAINER>\n"
                       );

  QString ref_false2 = QString(
                         "<!DOCTYPE TEST>\n"
                         "<TRANSACTION-CONTAINER>\n"
                         " <MYMONEYOBJECT >\n"
                         " </MYMONEYOBJECT>\n"
                         "</TRANSACTION-CONTAINER>\n"
                       );

  QDomDocument doc;
  QDomElement node;

  // id="" but required
  doc.setContent(ref_false1);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject::readBaseXML(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  // id attribute missing but required
  doc.setContent(ref_false2);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject::readBaseXML(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  // id present
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject::readBaseXML(node);
    QVERIFY(t.id() == "T000000000000000001");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  // id="" but not required
  doc.setContent(ref_false1);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject::readBaseXML(node, false);
    QVERIFY(t.id().isEmpty());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::readKeyValueContainer()
{
  MyMoneyKeyValueContainer kvp;
  kvp.setValue("Key", "Value");
  kvp.setValue("key", "value");

  QString ref_ok(
    "<!DOCTYPE TEST>\n"
    "<KVP-CONTAINER>\n"
    " <KEYVALUEPAIRS>\n"
    "  <PAIR key=\"key\" value=\"Value\" />\n"
    "  <PAIR key=\"Key\" value=\"value\" />\n"
    " </KEYVALUEPAIRS>\n"
    "</KVP-CONTAINER>\n");

  QString ref_false(
    "<!DOCTYPE TEST>\n"
    "<KVP-CONTAINER>\n"
    " <KEYVALUE-PAIRS>\n"
    "  <PAIR key=\"key\" value=\"Value\" />\n"
    "  <PAIR key=\"Key\" value=\"value\" />\n"
    " </KEYVALUE-PAIRS>\n"
    "</KVP-CONTAINER>\n");


  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  // make sure, an empty node does not trigger an exception
  try {
    QDomElement e;
    MyMoneyKeyValueContainer k;
    MyMoneyXmlContentHandler::addToKeyValueContainer(k, e);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  try {
    MyMoneyKeyValueContainer k;
    MyMoneyXmlContentHandler::addToKeyValueContainer(k, node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();
  try {
    MyMoneyKeyValueContainer k;
    MyMoneyXmlContentHandler::addToKeyValueContainer(k, node);
//    QVERIFY(k.d_func()->m_kvp.count() == 2); to be enabled soon
    QVERIFY(k.value("key") == "Value");
    QVERIFY(k.value("Key") == "value");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::writeKeyValueContainer()
{
  MyMoneyKeyValueContainer kvp;
  kvp.setValue("Key", "Value");
  kvp.setValue("key", "value");

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("KVP-CONTAINER");
  doc.appendChild(el);
  MyMoneyXmlContentHandler::writeKeyValueContainer(kvp, doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement kvpContainer = doc.documentElement();
  QCOMPARE(kvpContainer.tagName(), QLatin1String("KVP-CONTAINER"));
  QCOMPARE(kvpContainer.childNodes().size(), 1);

  QVERIFY(kvpContainer.childNodes().at(0).isElement());
  QDomElement keyValuePairs = kvpContainer.childNodes().at(0).toElement();
  QCOMPARE(keyValuePairs.tagName(), QLatin1String("KEYVALUEPAIRS"));
  QCOMPARE(keyValuePairs.childNodes().size(), 2);

  QVERIFY(keyValuePairs.childNodes().at(0).isElement());
  QDomElement keyValuePair1 = keyValuePairs.childNodes().at(0).toElement();
  QCOMPARE(keyValuePair1.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair1.attribute("key"), QLatin1String("Key"));
  QCOMPARE(keyValuePair1.attribute("value"), QLatin1String("Value"));
  QCOMPARE(keyValuePair1.childNodes().size(), 0);

  QVERIFY(keyValuePairs.childNodes().at(1).isElement());
  QDomElement keyValuePair2 = keyValuePairs.childNodes().at(1).toElement();
  QCOMPARE(keyValuePair2.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair2.attribute("key"), QLatin1String("key"));
  QCOMPARE(keyValuePair2.attribute("value"), QLatin1String("value"));
  QCOMPARE(keyValuePair2.childNodes().size(), 0);
}

void MyMoneyXmlContentHandlerTest::readTransaction()
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
    t = MyMoneyXmlContentHandler::readTransaction(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  t.setValue("key", "VALUE");
  try {
    t = MyMoneyXmlContentHandler::readTransaction(node);
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

void MyMoneyXmlContentHandlerTest::readTransactionEx()
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
                     "    <PAIR key=\"kmm-matched-tx\" value=\"&#60;!DOCTYPE MATCH>\n"
                     "    &#60;CONTAINER>\n"
                     "     &#60;TRANSACTION postdate=&quot;2010-03-05&quot; memo=&quot;UMBUCHUNG&quot; id=&quot;&quot; commodity=&quot;EUR&quot; entrydate=&quot;2010-03-08&quot; >\n"
                     "      &#60;SPLITS>\n"
                     "       &#60;SPLIT payee=&quot;P000010&quot; reconciledate=&quot;&quot; shares=&quot;125000/100&quot; action=&quot;Transfer&quot; bankid=&quot;&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;125000/100&quot; id=&quot;S0001&quot; account=&quot;A000087&quot; />\n"
                     "       &#60;SPLIT payee=&quot;P000010&quot; reconciledate=&quot;&quot; shares=&quot;-125000/100&quot; action=&quot;&quot; bankid=&quot;A000076-2010-03-05-b6850c0-1&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;-125000/100&quot; id=&quot;S0002&quot; account=&quot;A000076&quot; />\n"
                     "      &#60;/SPLITS>\n"
                     "      &#60;KEYVALUEPAIRS>\n"
                     "       &#60;PAIR key=&quot;Imported&quot; value=&quot;true&quot; />\n"
                     "      &#60;/KEYVALUEPAIRS>\n"
                     "     &#60;/TRANSACTION>\n"
                     "    &#60;/CONTAINER>\n"
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
    t = MyMoneyXmlContentHandler::readTransaction(node);
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

void MyMoneyXmlContentHandlerTest::writeTransaction()
{
  MyMoneyTransaction t("T000000000000000001");
  t.setPostDate(QDate(2001, 12, 28));
  t.setEntryDate(QDate(2003, 9, 29));
  t.setMemo("Wohnung:Miete");
  t.setCommodity("EUR");
  t.setValue("key", "value");

  MyMoneyTransaction matchedTx;
  matchedTx.setMemo(QLatin1String("TEST <TEST"));
  MyMoneySplit matchedSplit;
  matchedSplit.setMemo(matchedTx.memo());
  matchedSplit.setAccountId("A000076");
  matchedTx.addSplit(matchedSplit);

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
  s.setBankID("SPID");
  s.addMatch(matchedTx);
  QVERIFY(s.matchedTransaction() == matchedTx);
  t.addSplit(s);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("TRANSACTION-CONTAINER");
  doc.appendChild(el);
  MyMoneyXmlContentHandler::writeTransaction(t, doc, el);

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
  QCOMPARE(split.childNodes().size(), 2);

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

  auto tread = MyMoneyXmlContentHandler::readTransaction(transaction);
  QCOMPARE(tread.postDate(), t.postDate());
  QCOMPARE(tread.memo(), t.memo());
  QCOMPARE(tread.splits().count(), t.splits().count());
  // now check the parts that are important in the matched transaction
  tread = tread.splits().at(0).matchedTransaction();
  QCOMPARE(tread.postDate(), matchedTx.postDate());
  QCOMPARE(tread.memo(), matchedTx.memo());
  QCOMPARE(tread.splits().count(), matchedTx.splits().count());
  QCOMPARE(tread.splits().count(), 1);
  QCOMPARE(tread.splits().at(0).memo(), matchedSplit.memo());
}

void MyMoneyXmlContentHandlerTest::readSplit()
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
    s = MyMoneyXmlContentHandler::readSplit(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    s = MyMoneyXmlContentHandler::readSplit(node);
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
    QCOMPARE(s.action(), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit));
    QCOMPARE(s.accountId(), QLatin1String("A000076"));
    QCOMPARE(s.costCenterId(), QLatin1String("C000005"));
    QCOMPARE(s.memo(), QLatin1String("MyMemo"));
  } catch (const MyMoneyException &) {
  }

}

void MyMoneyXmlContentHandlerTest::writeSplit()
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
  s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit));
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("SPLIT-CONTAINER");
  doc.appendChild(el);
  MyMoneyXmlContentHandler::writeSplit(s, doc, el);

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
}

void MyMoneyXmlContentHandlerTest::testReplaceIDinSplit()
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
                     "    <PAIR key=\"kmm-matched-tx\" value=\"&#60;!DOCTYPE MATCH>\n"
                     "    &#60;CONTAINER>\n"
                     "     &#60;TRANSACTION postdate=&quot;2010-03-05&quot; memo=&quot;UMBUCHUNG&quot; id=&quot;&quot; commodity=&quot;EUR&quot; entrydate=&quot;2010-03-08&quot; >\n"
                     "      &#60;SPLITS>\n"
                     "       &#60;SPLIT payee=&quot;P000010&quot; reconciledate=&quot;&quot; shares=&quot;125000/100&quot; action=&quot;Transfer&quot; bankid=&quot;&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;125000/100&quot; id=&quot;S0001&quot; account=&quot;A000087&quot; />\n"
                     "       &#60;SPLIT payee=&quot;P000011&quot; reconciledate=&quot;&quot; shares=&quot;-125000/100&quot; action=&quot;&quot; bankid=&quot;A000076-2010-03-05-b6850c0-1&quot; number=&quot;&quot; reconcileflag=&quot;0&quot; memo=&quot;UMBUCHUNG&quot; value=&quot;-125000/100&quot; id=&quot;S0002&quot; account=&quot;A000076&quot; />\n"
                     "      &#60;/SPLITS>\n"
                     "      &#60;KEYVALUEPAIRS>\n"
                     "       &#60;PAIR key=&quot;Imported&quot; value=&quot;true&quot; />\n"
                     "      &#60;/KEYVALUEPAIRS>\n"
                     "     &#60;/TRANSACTION>\n"
                     "    &#60;/CONTAINER>\n"
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
    s = MyMoneyXmlContentHandler::readSplit(node);
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

void MyMoneyXmlContentHandlerTest::readAccount()
{
  MyMoneyAccount a;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<ACCOUNT-CONTAINER>\n"
                     " <ACCOUNT parentaccount=\"Parent\" lastmodified=\"%1\" lastreconciled=\"\" institution=\"B000001\" number=\"465500\" opened=\"%2\" type=\"9\" id=\"A000001\" name=\"AccountName\" description=\"Desc\" >\n"
                     "  <SUBACCOUNTS>\n"
                     "   <SUBACCOUNT id=\"A000002\" />\n"
                     "   <SUBACCOUNT id=\"A000003\" />\n"
                     "  </SUBACCOUNTS>\n"
                     "  <KEYVALUEPAIRS>\n"
                     "   <PAIR key=\"key\" value=\"value\" />\n"
                     "   <PAIR key=\"Key\" value=\"Value\" />\n"
                     "   <PAIR key=\"reconciliationHistory\" value=\"2011-01-01:123/100;2011-02-01:114/25\"/>\n"
                     "   <PAIR key=\"lastStatementDate\" value=\"2011-01-01\"/>\n"
                     "  </KEYVALUEPAIRS>\n"
                     " </ACCOUNT>\n"
                     "</ACCOUNT-CONTAINER>\n").
                   arg(QDate::currentDate().toString(Qt::ISODate)).arg(QDate::currentDate().toString(Qt::ISODate));

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<ACCOUNT-CONTAINER>\n"
                        " <KACCOUNT parentaccount=\"Parent\" lastmodified=\"%1\" lastreconciled=\"\" institution=\"B000001\" number=\"465500\" opened=\"%2\" type=\"9\" id=\"A000001\" name=\"AccountName\" description=\"Desc\" >\n"
                        "  <SUBACCOUNTS>\n"
                        "   <SUBACCOUNT id=\"A000002\" />\n"
                        "   <SUBACCOUNT id=\"A000003\" />\n"
                        "  </SUBACCOUNTS>\n"
                        "  <KEYVALUEPAIRS>\n"
                        "   <PAIR key=\"key\" value=\"value\" />\n"
                        "   <PAIR key=\"Key\" value=\"Value\" />\n"
                        "  </KEYVALUEPAIRS>\n"
                        " </KACCOUNT>\n"
                        "</ACCOUNT-CONTAINER>\n").
                      arg(QDate::currentDate().toString(Qt::ISODate)).arg(QDate::currentDate().toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  try {
    a = MyMoneyXmlContentHandler::readAccount(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  a.addAccountId("TEST");
  a.setValue("KEY", "VALUE");

  try {
    a = MyMoneyXmlContentHandler::readAccount(node);
    QCOMPARE(a.id(), QStringLiteral("A000001"));
    QCOMPARE(a.name(), QStringLiteral("AccountName"));
    QCOMPARE(a.parentAccountId(), QStringLiteral("Parent"));
    QCOMPARE(a.lastModified(), QDate::currentDate());
    QCOMPARE(a.lastReconciliationDate(), QDate());
    QCOMPARE(a.institutionId(), QStringLiteral("B000001"));
    QCOMPARE(a.number(), QStringLiteral("465500"));
    QCOMPARE(a.openingDate(), QDate::currentDate());
    QCOMPARE(a.accountType(), eMyMoney::Account::Type::Asset);
    QCOMPARE(a.description(), QStringLiteral("Desc"));
    QCOMPARE(a.accountList().count(), 2);
    QCOMPARE(a.accountList()[0], QStringLiteral("A000002"));
    QCOMPARE(a.accountList()[1], QStringLiteral("A000003"));
    QCOMPARE(a.pairs().count(), 3);
    QCOMPARE(a.value("key"), QStringLiteral("value"));
    QCOMPARE(a.value("Key"), QStringLiteral("Value"));
    QCOMPARE(a.pairs().contains("lastStatementDate"), false);
    QCOMPARE(a.reconciliationHistory().count(), 2);
    QCOMPARE(a.reconciliationHistory()[QDate(2011, 1, 1)].toString(), MyMoneyMoney(123, 100).toString());
    QCOMPARE(a.reconciliationHistory()[QDate(2011, 2, 1)].toString(), MyMoneyMoney(456, 100).toString());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::writeAccount()
{
  QString id = "A000001";
  QString institutionid = "B000001";
  QString parent = "Parent";

  MyMoneyAccount r;
  r.setAccountType(eMyMoney::Account::Type::Asset);
  r.setOpeningDate(QDate::currentDate());
  r.setLastModified(QDate::currentDate());
  r.setDescription("Desc");
  r.setName("AccountName");
  r.setNumber("465500");
  r.setParentAccountId(parent);
  r.setInstitutionId(institutionid);
  r.setValue(QString("key"), "value");
  r.addAccountId("A000002");
  r.addReconciliation(QDate(2011, 1, 1), MyMoneyMoney(123, 100));
  r.addReconciliation(QDate(2011, 2, 1), MyMoneyMoney(456, 100));

  QCOMPARE(r.pairs().count(), 2);
  QCOMPARE(r.value("key"), QLatin1String("value"));
  QCOMPARE(r.value("reconciliationHistory"), QLatin1String("2011-01-01:123/100;2011-02-01:114/25"));

  MyMoneyAccount a(id, r);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("ACCOUNT-CONTAINER");
  doc.appendChild(el);
  MyMoneyXmlContentHandler::writeAccount(a, doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement accountContainer = doc.documentElement();
  QVERIFY(accountContainer.isElement());
  QCOMPARE(accountContainer.tagName(), QLatin1String("ACCOUNT-CONTAINER"));
  QVERIFY(accountContainer.childNodes().size() == 1);
  QVERIFY(accountContainer.childNodes().at(0).isElement());

  QDomElement account = accountContainer.childNodes().at(0).toElement();
  QCOMPARE(account.tagName(), QLatin1String("ACCOUNT"));
  QCOMPARE(account.attribute("id"), QLatin1String("A000001"));
  QCOMPARE(account.attribute("lastreconciled"), QString());
  QCOMPARE(account.attribute("institution"), QLatin1String("B000001"));
  QCOMPARE(account.attribute("name"), QLatin1String("AccountName"));
  QCOMPARE(account.attribute("number"), QLatin1String("465500"));
  QCOMPARE(account.attribute("description"), QLatin1String("Desc"));
  QCOMPARE(account.attribute("parentaccount"), QLatin1String("Parent"));
  QCOMPARE(account.attribute("opened"), QDate::currentDate().toString(Qt::ISODate));
  QCOMPARE(account.attribute("type"), QLatin1String("9"));
  QCOMPARE(account.attribute("lastmodified"), QDate::currentDate().toString(Qt::ISODate));
  QCOMPARE(account.attribute("id"), QLatin1String("A000001"));
  QCOMPARE(account.childNodes().size(), 2);

  QVERIFY(account.childNodes().at(0).isElement());
  QDomElement subAccounts = account.childNodes().at(0).toElement();
  QCOMPARE(subAccounts.tagName(), QLatin1String("SUBACCOUNTS"));
  QCOMPARE(subAccounts.childNodes().size(), 1);
  QVERIFY(subAccounts.childNodes().at(0).isElement());
  QDomElement subAccount = subAccounts.childNodes().at(0).toElement();
  QCOMPARE(subAccount.tagName(), QLatin1String("SUBACCOUNT"));
  QCOMPARE(subAccount.attribute("id"), QLatin1String("A000002"));
  QCOMPARE(subAccount.childNodes().size(), 0);

  QDomElement keyValuePairs = account.childNodes().at(1).toElement();
  QCOMPARE(keyValuePairs.tagName(), QLatin1String("KEYVALUEPAIRS"));
  QCOMPARE(keyValuePairs.childNodes().size(), 2);

  QVERIFY(keyValuePairs.childNodes().at(0).isElement());
  QDomElement keyValuePair1 = keyValuePairs.childNodes().at(0).toElement();
  QCOMPARE(keyValuePair1.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair1.attribute("key"), QLatin1String("key"));
  QCOMPARE(keyValuePair1.attribute("value"), QLatin1String("value"));
  QCOMPARE(keyValuePair1.childNodes().size(), 0);

  QVERIFY(keyValuePairs.childNodes().at(1).isElement());
  QDomElement keyValuePair2 = keyValuePairs.childNodes().at(1).toElement();
  QCOMPARE(keyValuePair2.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair2.attribute("key"), QLatin1String("reconciliationHistory"));
  QCOMPARE(keyValuePair2.attribute("value"), QLatin1String("2011-01-01:123/100;2011-02-01:114/25"));
  QCOMPARE(keyValuePair2.childNodes().size(), 0);
}

void MyMoneyXmlContentHandlerTest::readWritePayee()
{
  QDomDocument doc;
  QDomElement parent = doc.createElement("Test");
  doc.appendChild(parent);
  MyMoneyPayee payee1("some random id"); //if the ID isn't set, w ethrow an exception
  MyMoneyXmlContentHandler::writePayee(payee1, doc, parent);
  QString temp1 = "Account1";
  payee1.setDefaultAccountId(temp1);
  MyMoneyXmlContentHandler::writePayee(payee1, doc, parent);
  QString temp2 = "Account2";
  payee1.setDefaultAccountId(temp2);
  MyMoneyXmlContentHandler::writePayee(payee1, doc, parent);
  payee1.setDefaultAccountId();
  MyMoneyXmlContentHandler::writePayee(payee1, doc, parent);
  QDomElement el = parent.firstChild().toElement();
  QVERIFY(!el.isNull());
  auto payee2 = MyMoneyXmlContentHandler::readPayee(el);
  QVERIFY(payee2.defaultAccountId().isEmpty());
  el = el.nextSibling().toElement();
  QVERIFY(!el.isNull());
  auto payee3 = MyMoneyXmlContentHandler::readPayee(el);
  QVERIFY(!payee3.defaultAccountId().isEmpty());
  QVERIFY(payee3.defaultAccountId() == temp1);
  el = el.nextSibling().toElement();
  QVERIFY(!el.isNull());
  auto payee4 = MyMoneyXmlContentHandler::readPayee(el);
  QVERIFY(!payee4.defaultAccountId().isEmpty());
  QVERIFY(payee4.defaultAccountId() == temp2);
  el = el.nextSibling().toElement();
  QVERIFY(!el.isNull());
  auto payee5 = MyMoneyXmlContentHandler::readPayee(el);
  QVERIFY(payee5.defaultAccountId().isEmpty());
}

void MyMoneyXmlContentHandlerTest::readWriteTag()
{
  QDomDocument doc;
  QDomElement parent = doc.createElement("Test");
  doc.appendChild(parent);
  MyMoneyTag tag1("some random id"); //if the ID isn't set, w ethrow an exception
  tag1.setName(QStringLiteral("MyTagName"));
  tag1.setNamedTagColor(QStringLiteral("red"));
  tag1.setNotes(QStringLiteral("Notes for a tag"));

  MyMoneyXmlContentHandler::writeTag(tag1, doc, parent);
  QDomElement el = parent.firstChild().toElement();
  QVERIFY(!el.isNull());
  auto tag2 = MyMoneyXmlContentHandler::readTag(el);
  QCOMPARE(tag1.name(), tag2.name());
  QCOMPARE(tag1.notes(), tag2.notes());
  QCOMPARE(tag1.tagColor(), tag2.tagColor());
}

void MyMoneyXmlContentHandlerTest::readInstitution()
{
  MyMoneyInstitution i;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<INSTITUTION-CONTAINER>\n"
                     " <INSTITUTION sortcode=\"sortcode\" id=\"I00001\" manager=\"manager\" name=\"name\" >\n"
                     "  <ADDRESS street=\"street\" zip=\"postcode\" city=\"town\" telephone=\"telephone\" />\n"
                     "  <ACCOUNTIDS>\n"
                     "   <ACCOUNTID id=\"A000001\" />\n"
                     "   <ACCOUNTID id=\"A000003\" />\n"
                     "  </ACCOUNTIDS>\n"
                     "  <KEYVALUEPAIRS>\n"
                     "   <PAIR key=\"key\" value=\"value\" />\n"
                     "  </KEYVALUEPAIRS>\n"
                     " </INSTITUTION>\n"
                     "</INSTITUTION-CONTAINER>\n");

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<INSTITUTION-CONTAINER>\n"
                        " <KINSTITUTION sortcode=\"sortcode\" id=\"I00001\" manager=\"manager\" name=\"name\" >\n"
                        "  <ADDRESS street=\"street\" zip=\"postcode\" city=\"town\" telephone=\"telephone\" />\n"
                        "  <ACCOUNTIDS>\n"
                        "   <ACCOUNTID id=\"A000001\" />\n"
                        "   <ACCOUNTID id=\"A000003\" />\n"
                        "  </ACCOUNTIDS>\n"
                        " </KINSTITUTION>\n"
                        "</INSTITUTION-CONTAINER>\n");

  QDomDocument doc;
  QDomElement node;

  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();
  try {
    i = MyMoneyXmlContentHandler::readInstitution(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  i.addAccountId("TEST");

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();
  try {
    QStringList alist;
    alist << "A000001" << "A000003";
    i = MyMoneyXmlContentHandler::readInstitution(node);

    QVERIFY(i.bankcode() == "sortcode");
    QVERIFY(i.id() == "I00001");
    QVERIFY(i.manager() == "manager");
    QVERIFY(i.name() == "name");
    QVERIFY(i.street() == "street");
    QVERIFY(i.postcode() == "postcode");
    QVERIFY(i.city() == "town");
    QVERIFY(i.telephone() == "telephone");
    QVERIFY(i.accountList() == alist);
    QVERIFY(i.value(QString("key")) == "value");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::writeInstitution()
{
  MyMoneyInstitution n("name", "town", "street", "postcode",
                       "telephone", "manager", "sortcode");;

  n.addAccountId("A000001");
  n.addAccountId("A000003");
  n.setValue(QString("key"), "value");

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("INSTITUTION-CONTAINER");
  doc.appendChild(el);

  MyMoneyInstitution i("I00001", n);
  MyMoneyXmlContentHandler::writeInstitution(i, doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement institutionContainer = doc.documentElement();
  QVERIFY(institutionContainer.isElement());
  QCOMPARE(institutionContainer.tagName(), QLatin1String("INSTITUTION-CONTAINER"));
  QVERIFY(institutionContainer.childNodes().size() == 1);
  QVERIFY(institutionContainer.elementsByTagName("INSTITUTION").at(0).isElement());

  QDomElement institution = institutionContainer.elementsByTagName("INSTITUTION").at(0).toElement();
  QCOMPARE(institution.tagName(), QLatin1String("INSTITUTION"));
  QCOMPARE(institution.attribute("id"), QLatin1String("I00001"));
  QCOMPARE(institution.attribute("manager"), QLatin1String("manager"));
  QCOMPARE(institution.attribute("name"), QLatin1String("name"));
  QCOMPARE(institution.attribute("sortcode"), QLatin1String("sortcode"));
  QCOMPARE(institution.childNodes().size(), 3);

  QVERIFY(institution.childNodes().at(0).isElement());
  QDomElement address = institution.childNodes().at(0).toElement();
  QCOMPARE(address.tagName(), QLatin1String("ADDRESS"));
  QCOMPARE(address.attribute("street"), QLatin1String("street"));
  QCOMPARE(address.attribute("telephone"), QLatin1String("telephone"));
  QCOMPARE(address.attribute("zip"), QLatin1String("postcode"));
  QCOMPARE(address.attribute("city"), QLatin1String("town"));
  QCOMPARE(address.childNodes().size(), 0);

  QVERIFY(institution.childNodes().at(1).isElement());
  QDomElement accountIds = institution.childNodes().at(1).toElement();
  QCOMPARE(accountIds.tagName(), QLatin1String("ACCOUNTIDS"));
  QCOMPARE(accountIds.childNodes().size(), 2);

  QVERIFY(accountIds.childNodes().at(0).isElement());
  QDomElement account1 = accountIds.childNodes().at(0).toElement();
  QCOMPARE(account1.tagName(), QLatin1String("ACCOUNTID"));
  QCOMPARE(account1.attribute("id"), QLatin1String("A000001"));
  QCOMPARE(account1.childNodes().size(), 0);

  QVERIFY(accountIds.childNodes().at(1).isElement());
  QDomElement account2 = accountIds.childNodes().at(1).toElement();
  QCOMPARE(account2.tagName(), QLatin1String("ACCOUNTID"));
  QCOMPARE(account2.attribute("id"), QLatin1String("A000003"));
  QCOMPARE(account2.childNodes().size(), 0);

  QVERIFY(institution.childNodes().at(2).isElement());
  QDomElement keyValuePairs = institution.childNodes().at(2).toElement();
  QCOMPARE(keyValuePairs.tagName(), QLatin1String("KEYVALUEPAIRS"));
  QCOMPARE(keyValuePairs.childNodes().size(), 1);

  QVERIFY(keyValuePairs.childNodes().at(0).isElement());
  QDomElement keyValuePair1 = keyValuePairs.childNodes().at(0).toElement();
  QCOMPARE(keyValuePair1.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair1.attribute("key"), QLatin1String("key"));
  QCOMPARE(keyValuePair1.attribute("value"), QLatin1String("value"));
  QCOMPARE(keyValuePair1.childNodes().size(), 0);
}

void MyMoneyXmlContentHandlerTest::readSchedule()
{
  MyMoneySchedule sch;

  QString ref_ok1 = QString(
                      "<!DOCTYPE TEST>\n"
                      "<SCHEDULE-CONTAINER>\n"
                      " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                      "  <PAYMENTS>\n"
                      "   <PAYMENT date=\"%3\" />\n"
                      "  </PAYMENTS>\n"
                      "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                      "   <SPLITS>\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                      "   </SPLITS>\n"
                      "   <KEYVALUEPAIRS>\n"
                      "    <PAIR key=\"key\" value=\"value\" />\n"
                      "   </KEYVALUEPAIRS>\n"
                      "  </TRANSACTION>\n"
                      " </SCHEDULED_TX>\n"
                      "</SCHEDULE-CONTAINER>\n"
                    ).arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate));

  // diff to ref_ok1 is that we now have an empty entrydate
  // in the transaction parameters
  QString ref_ok2 = QString(
                      "<!DOCTYPE TEST>\n"
                      "<SCHEDULE-CONTAINER>\n"
                      " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                      "  <PAYMENTS>\n"
                      "   <PAYMENT date=\"%3\" />\n"
                      "  </PAYMENTS>\n"
                      "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
                      "   <SPLITS>\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                      "   </SPLITS>\n"
                      "   <KEYVALUEPAIRS>\n"
                      "    <PAIR key=\"key\" value=\"value\" />\n"
                      "   </KEYVALUEPAIRS>\n"
                      "  </TRANSACTION>\n"
                      " </SCHEDULED_TX>\n"
                      "</SCHEDULE-CONTAINER>\n"
                    ).arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate));

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<SCHEDULE-CONTAINER>\n"
                        " <SCHEDULE startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                        "  <PAYMENTS count=\"1\" >\n"
                        "   <PAYMENT date=\"%3\" />\n"
                        "  </PAYMENTS>\n"
                        "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                        "   <SPLITS>\n"
                        "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                        "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                        "   </SPLITS>\n"
                        "   <KEYVALUEPAIRS>\n"
                        "    <PAIR key=\"key\" value=\"value\" />\n"
                        "   </KEYVALUEPAIRS>\n"
                        "  </TRANSACTION>\n"
                        " </SCHEDULED_TX>\n"
                        "</SCHEDULE-CONTAINER>\n"
                      ).arg(QDate::currentDate().toString(Qt::ISODate))
                      .arg(QDate::currentDate().toString(Qt::ISODate))
                      .arg(QDate::currentDate().toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok1);
  node = doc.documentElement().firstChild().toElement();

  setupAccounts();
  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QCOMPARE(sch.id(), QLatin1String("SCH0002"));
    QCOMPARE(sch.nextDueDate(), QDate::currentDate().addDays(7));
    QCOMPARE(sch.startDate(), QDate::currentDate());
    QCOMPARE(sch.endDate(), QDate());
    QCOMPARE(sch.autoEnter(), true);
    QCOMPARE(sch.isFixed(), true);
    QCOMPARE(sch.weekendOption(), Schedule::WeekendOption::MoveNothing);
    QCOMPARE(sch.lastPayment(), QDate::currentDate());
    QCOMPARE(sch.paymentType(), Schedule::PaymentType::DirectDebit);
    QCOMPARE(sch.type(), Schedule::Type::Bill);
    QCOMPARE(sch.name(), QLatin1String("A Name"));
    QCOMPARE(sch.baseOccurrence(), Schedule::Occurrence::Weekly);
    QCOMPARE(sch.occurrenceMultiplier(), 1);
    QCOMPARE(sch.nextDueDate(), sch.lastPayment().addDays(7));
    QCOMPARE(sch.recordedPayments().count(), 1);
    QCOMPARE(sch.recordedPayments()[0], QDate::currentDate());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  doc.setContent(ref_ok2);
  node = doc.documentElement().firstChild().toElement();


  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QCOMPARE(sch.id(), QLatin1String("SCH0002"));
    QCOMPARE(sch.nextDueDate(), QDate::currentDate().addDays(7));
    QCOMPARE(sch.startDate(), QDate::currentDate());
    QCOMPARE(sch.endDate(), QDate());
    QCOMPARE(sch.autoEnter(), true);
    QCOMPARE(sch.isFixed(), true);
    QCOMPARE(sch.weekendOption(), Schedule::WeekendOption::MoveNothing);
    QCOMPARE(sch.lastPayment(), QDate::currentDate());
    QCOMPARE(sch.paymentType(), Schedule::PaymentType::DirectDebit);
    QCOMPARE(sch.type(), Schedule::Type::Bill);
    QCOMPARE(sch.name(), QLatin1String("A Name"));
    QCOMPARE(sch.baseOccurrence(), Schedule::Occurrence::Weekly);
    QCOMPARE(sch.occurrenceMultiplier(), 1);
    QCOMPARE(sch.nextDueDate(), sch.lastPayment().addDays(7));
    QCOMPARE(sch.recordedPayments().count(), 1);
    QCOMPARE(sch.recordedPayments()[0], QDate::currentDate());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::writeSchedule()
{
  setupAccounts();
  MyMoneySchedule tempSch("A Name",
                      Schedule::Type::Bill,
                      Schedule::Occurrence::Weekly, 123,
                      Schedule::PaymentType::DirectDebit,
                      QDate::currentDate(),
                      QDate(),
                      true,
                      true);

  MyMoneySchedule sch("SCH0001", tempSch);
  sch.setLastPayment(QDate::currentDate());
  sch.recordPayment(QDate::currentDate());

  MyMoneyTransaction t("T000000000000000001");
  t.setPostDate(QDate(2001, 12, 28));
  t.setEntryDate(QDate(2003, 9, 29));
  t.setMemo("Wohnung:Miete");
  t.setCommodity("EUR");
  t.setValue("key", "value");

  MyMoneySplit s;
  s.setPayeeId("P000001");
  s.setShares(MyMoneyMoney(96379, 100));
  s.setValue(MyMoneyMoney(96379, 100));
  s.setAccountId("A000076");
  s.setBankID("SPID1");
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  t.addSplit(s);

  s.setPayeeId("P000001");
  s.setShares(MyMoneyMoney(-96379, 100));
  s.setValue(MyMoneyMoney(-96379, 100));
  s.setAccountId("A000276");
  s.setBankID("SPID2");
  s.setReconcileFlag(eMyMoney::Split::State::Cleared);
  s.clearId();
  t.addSplit(s);

  sch.setTransaction(t);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("SCHEDULE-CONTAINER");
  doc.appendChild(el);
  MyMoneyXmlContentHandler::writeSchedule(sch, doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement scheduleContainer = doc.documentElement();
  QVERIFY(scheduleContainer.isElement());
  QCOMPARE(scheduleContainer.tagName(), QLatin1String("SCHEDULE-CONTAINER"));
  QCOMPARE(scheduleContainer.childNodes().size(), 1);
  QVERIFY(scheduleContainer.childNodes().at(0).isElement());

  QDomElement schedule = scheduleContainer.childNodes().at(0).toElement();
  QCOMPARE(schedule.tagName(), QLatin1String("SCHEDULED_TX"));
  QCOMPARE(schedule.attribute("id"), QLatin1String("SCH0001"));
  QCOMPARE(schedule.attribute("paymentType"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("autoEnter"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("occurenceMultiplier"), QLatin1String("123")); // krazy:exclude=spelling
  QCOMPARE(schedule.attribute("startDate"), QDate::currentDate().toString(Qt::ISODate));
  QCOMPARE(schedule.attribute("lastPayment"), QDate::currentDate().toString(Qt::ISODate));
  QCOMPARE(schedule.attribute("occurenceMultiplier"), QLatin1String("123")); // krazy:exclude=spelling
  QCOMPARE(schedule.attribute("occurence"), QLatin1String("4")); // krazy:exclude=spelling
  QCOMPARE(schedule.attribute("type"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("name"), QLatin1String("A Name"));
  QCOMPARE(schedule.attribute("fixed"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("endDate"), QString());
  QCOMPARE(schedule.childNodes().size(), 2);

  QVERIFY(schedule.childNodes().at(0).isElement());
  QDomElement payments = schedule.childNodes().at(0).toElement();

  QVERIFY(schedule.childNodes().at(1).isElement());
  QDomElement transaction = schedule.childNodes().at(1).toElement();
  QCOMPARE(transaction.tagName(), QLatin1String("TRANSACTION"));
  QCOMPARE(transaction.attribute("id"), QString());
  QCOMPARE(transaction.attribute("postdate"), QLatin1String("2001-12-28"));
  QCOMPARE(transaction.attribute("commodity"), QLatin1String("EUR"));
  QCOMPARE(transaction.attribute("memo"), QLatin1String("Wohnung:Miete"));
  QCOMPARE(transaction.attribute("entrydate"), QLatin1String("2003-09-29"));
  QCOMPARE(transaction.childNodes().size(), 2);

  QVERIFY(transaction.childNodes().at(0).isElement());
  QDomElement splits = transaction.childNodes().at(0).toElement();
  QCOMPARE(splits.tagName(), QLatin1String("SPLITS"));
  QCOMPARE(splits.childNodes().size(), 2);
  QVERIFY(splits.childNodes().at(0).isElement());
  QDomElement split1 = splits.childNodes().at(0).toElement();
  QCOMPARE(split1.tagName(), QLatin1String("SPLIT"));
  QCOMPARE(split1.attribute("id"), QLatin1String("S0001"));
  QCOMPARE(split1.attribute("payee"), QLatin1String("P000001"));
  QCOMPARE(split1.attribute("reconcileflag"), QLatin1String("2"));
  QCOMPARE(split1.attribute("shares"), QLatin1String("96379/100"));
  QCOMPARE(split1.attribute("reconciledate"), QString());
  QCOMPARE(split1.attribute("action"), QString());
  QCOMPARE(split1.attribute("bankid"), QString());
  QCOMPARE(split1.attribute("account"), QLatin1String("A000076"));
  QCOMPARE(split1.attribute("number"), QString());
  QCOMPARE(split1.attribute("value"), QLatin1String("96379/100"));
  QCOMPARE(split1.attribute("memo"), QString());
  QCOMPARE(split1.childNodes().size(), 0);

  QVERIFY(splits.childNodes().at(1).isElement());
  QDomElement split2 = splits.childNodes().at(1).toElement();
  QCOMPARE(split2.tagName(), QLatin1String("SPLIT"));
  QCOMPARE(split2.attribute("id"), QLatin1String("S0002"));
  QCOMPARE(split2.attribute("payee"), QLatin1String("P000001"));
  QCOMPARE(split2.attribute("reconcileflag"), QLatin1String("1"));
  QCOMPARE(split2.attribute("shares"), QLatin1String("-96379/100"));
  QCOMPARE(split2.attribute("reconciledate"), QString());
  QCOMPARE(split2.attribute("action"), QString());
  QCOMPARE(split2.attribute("bankid"), QString());
  QCOMPARE(split2.attribute("account"), QLatin1String("A000276"));
  QCOMPARE(split2.attribute("number"), QString());
  QCOMPARE(split2.attribute("value"), QLatin1String("-96379/100"));
  QCOMPARE(split2.attribute("memo"), QString());
  QCOMPARE(split2.childNodes().size(), 0);

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

void MyMoneyXmlContentHandlerTest::testOverdue()
{
  MyMoneySchedule sch_overdue;
  MyMoneySchedule sch_intime;

  // the following checks only work correctly, if currentDate() is
  // between the 1st and 27th. If it is between 28th and 31st
  // we don't perform them. Note: this should be fixed.
  if (QDate::currentDate().day() > 27 || QDate::currentDate().day() == 1) {
    qDebug() << "testOverdue() skipped because current day is between 28th and 2nd";
    return;
  }

  QDate startDate = QDate::currentDate().addDays(-1).addMonths(-23);
  QDate lastPaymentDate = QDate::currentDate().addDays(-1).addMonths(-1);

  QString ref = QString(
                  "<!DOCTYPE TEST>\n"
                  "<SCHEDULE-CONTAINER>\n"
                  " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"0\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"8\" endDate=\"\" type=\"5\" id=\"SCH0002\" name=\"A Name\" fixed=\"0\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                  "  <PAYMENTS>\n"
                  "   <PAYMENT date=\"%3\" />\n"
                  "  </PAYMENTS>\n"
                  "  <TRANSACTION postdate=\"\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
                  "   <SPLITS>\n"
                  "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                  "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                  "   </SPLITS>\n"
                  "   <KEYVALUEPAIRS>\n"
                  "    <PAIR key=\"key\" value=\"value\" />\n"
                  "   </KEYVALUEPAIRS>\n"
                  "  </TRANSACTION>\n"
                  " </SCHEDULED_TX>\n"
                  "</SCHEDULE-CONTAINER>\n");
  QString ref_overdue = ref.arg(startDate.toString(Qt::ISODate))
                        .arg(lastPaymentDate.toString(Qt::ISODate))
                        .arg(lastPaymentDate.toString(Qt::ISODate));

  QString ref_intime = ref.arg(startDate.addDays(1).toString(Qt::ISODate))
                       .arg(lastPaymentDate.addDays(1).toString(Qt::ISODate))
                       .arg(lastPaymentDate.addDays(1).toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;

  setupAccounts();
  // std::cout << ref_intime << std::endl;
  try {
    doc.setContent(ref_overdue);
    node = doc.documentElement().firstChild().toElement();
    sch_overdue = MyMoneyXmlContentHandler::readSchedule(node);
    doc.setContent(ref_intime);
    node = doc.documentElement().firstChild().toElement();
    sch_intime = MyMoneyXmlContentHandler::readSchedule(node);

    QCOMPARE(sch_overdue.isOverdue(), true);
    QCOMPARE(sch_intime.isOverdue(), false);

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::testNextPayment()
/*
 * Test for a schedule where a payment hasn't yet been made.
 * First payment is in the future.
*/
{
  MyMoneySchedule sch;
  QString future_sched = QString(
                           "<!DOCTYPE TEST>\n"
                           "<SCHEDULE-CONTAINER>\n"
                           "<SCHEDULED_TX startDate=\"2007-02-17\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH000058\" name=\"Car Tax\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"16384\" >\n" // krazy:exclude=spelling
                           "  <PAYMENTS/>\n"
                           "  <TRANSACTION postdate=\"\" memo=\"\" id=\"\" commodity=\"GBP\" entrydate=\"\" >\n"
                           "  <SPLITS>\n"
                           "    <SPLIT payee=\"P000044\" reconciledate=\"\" shares=\"-15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"-15000/100\" account=\"A000155\" />\n"
                           "    <SPLIT payee=\"\" reconciledate=\"\" shares=\"15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"15000/100\" account=\"A000182\" />\n"
                           "  </SPLITS>\n"
                           "  <KEYVALUEPAIRS/>\n"
                           "  </TRANSACTION>\n"
                           "</SCHEDULED_TX>\n"
                           "</SCHEDULE-CONTAINER>\n"
                         );

  QDomDocument doc;
  QDomElement node;
  doc.setContent(future_sched);
  node = doc.documentElement().firstChild().toElement();

  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QCOMPARE(sch.nextPayment(QDate(2007, 2, 14)), QDate(2007, 2, 17));
    QCOMPARE(sch.nextPayment(QDate(2007, 2, 17)), QDate(2008, 2, 17));
    QCOMPARE(sch.nextPayment(QDate(2007, 2, 18)), QDate(2008, 2, 17));

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::testNextPaymentOnLastDayOfMonth()
{
  MyMoneySchedule sch;
  QString future_sched = QString(
                           "<!DOCTYPE TEST>\n"
                           "<SCHEDULE-CONTAINER>\n"
                           "<SCHEDULED_TX startDate=\"2014-10-31\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH000058\" name=\"Car Tax\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                           "  <PAYMENTS/>\n"
                           "  <TRANSACTION postdate=\"\" memo=\"\" id=\"\" commodity=\"GBP\" entrydate=\"\" >\n"
                           "  <SPLITS>\n"
                           "    <SPLIT payee=\"P000044\" reconciledate=\"\" shares=\"-15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"-15000/100\" account=\"A000155\" />\n"
                           "    <SPLIT payee=\"\" reconciledate=\"\" shares=\"15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"15000/100\" account=\"A000182\" />\n"
                           "  </SPLITS>\n"
                           "  <KEYVALUEPAIRS/>\n"
                           "  </TRANSACTION>\n"
                           "</SCHEDULED_TX>\n"
                           "</SCHEDULE-CONTAINER>\n"
                         );

  QDomDocument doc;
  QDomElement node;
  doc.setContent(future_sched);
  node = doc.documentElement().firstChild().toElement();

  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QDate nextPayment;

    // check for the first payment to happen
    nextPayment = sch.nextPayment(QDate(2014, 10, 1));
    QCOMPARE(nextPayment, QDate(2014, 10, 31));
    sch.setLastPayment(nextPayment);

    QCOMPARE(sch.nextPayment(QDate(2014, 11, 1)), QDate(2014, 11, 30));
    QCOMPARE(sch.nextPayment(QDate(2014, 12, 1)), QDate(2014, 12, 31));
    QCOMPARE(sch.nextPayment(QDate(2015, 1, 1)), QDate(2015, 1, 31));
    QCOMPARE(sch.nextPayment(QDate(2015, 2, 1)), QDate(2015, 2, 28));
    QCOMPARE(sch.nextPayment(QDate(2015, 3, 1)), QDate(2015, 3, 31));

    // now check that we also cover leap years
    QCOMPARE(sch.nextPayment(QDate(2016, 2, 1)), QDate(2016, 2, 29));
    QCOMPARE(sch.nextPayment(QDate(2016, 3, 1)), QDate(2016, 3, 31));

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::testPaymentDates()
{
  MyMoneySchedule sch;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SCHEDULE-CONTAINER>\n"

                     "<SCHEDULED_TX startDate=\"2003-12-31\" autoEnter=\"1\" weekendOption=\"0\" lastPayment=\"2006-01-31\" paymentType=\"2\" endDate=\"\" type=\"2\" id=\"SCH000032\" name=\"DSL\" fixed=\"0\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                     " <PAYMENTS/>\n"
                     " <TRANSACTION postdate=\"2006-02-28\" memo=\"\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
                     "  <SPLITS>\n"
                     "   <SPLIT payee=\"P000076\" reconciledate=\"\" shares=\"1200/100\" action=\"Deposit\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"1200/100\" account=\"A000076\" />\n"
                     "   <SPLIT payee=\"\" reconciledate=\"\" shares=\"-1200/100\" action=\"Deposit\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"-1200/100\" account=\"A000009\" />\n"
                     "  </SPLITS>\n"
                     "  <KEYVALUEPAIRS/>\n"
                     " </TRANSACTION>\n"
                     "</SCHEDULED_TX>\n"

                     "</SCHEDULE-CONTAINER>\n"
                   );

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  QDate startDate(2006, 1, 28);
  QDate endDate(2006, 5, 30);

  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QDate nextPayment = sch.nextPayment(startDate);
    QList<QDate> list = sch.paymentDates(nextPayment, endDate);
    QCOMPARE(list.count(), 3);
    QCOMPARE(list[0], QDate(2006, 2, 28));
    QCOMPARE(list[1], QDate(2006, 3, 31));
    // Would fall on a Sunday so gets moved back to 28th.
    QCOMPARE(list[2], QDate(2006, 4, 28));

    // Add tests for each possible occurrence.
    // Check how paymentDates is meant to work
    // Build a list of expected dates and compare
    // Schedule::Occurrence::Once
    sch.setOccurrence(Schedule::Occurrence::Once);
    startDate.setDate(2009, 1, 1);
    endDate.setDate(2009, 12, 31);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 1);
    QCOMPARE(list[0], QDate(2009, 1, 1));
    // Schedule::Occurrence::Daily
    sch.setOccurrence(Schedule::Occurrence::Daily);
    startDate.setDate(2009, 1, 1);
    endDate.setDate(2009, 1, 5);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 1, 1));
    QCOMPARE(list[1], QDate(2009, 1, 2));
    // Would fall on Saturday so gets moved to 2nd.
    QCOMPARE(list[2], QDate(2009, 1, 2));
    // Would fall on Sunday so gets moved to 2nd.
    QCOMPARE(list[3], QDate(2009, 1, 2));
    QCOMPARE(list[4], QDate(2009, 1, 5));
    // Schedule::Occurrence::Daily with multiplier 2
    sch.setOccurrenceMultiplier(2);
    list = sch.paymentDates(startDate.addDays(1), endDate);
    QCOMPARE(list.count(), 2);
    // Would fall on Sunday so gets moved to 2nd.
    QCOMPARE(list[0], QDate(2009, 1, 2));
    QCOMPARE(list[1], QDate(2009, 1, 5));
    sch.setOccurrenceMultiplier(1);
    // Schedule::Occurrence::Weekly
    sch.setOccurrence(Schedule::Occurrence::Weekly);
    startDate.setDate(2009, 1, 6);
    endDate.setDate(2009, 2, 4);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 1, 6));
    QCOMPARE(list[1], QDate(2009, 1, 13));
    QCOMPARE(list[2], QDate(2009, 1, 20));
    QCOMPARE(list[3], QDate(2009, 1, 27));
    QCOMPARE(list[4], QDate(2009, 2, 3));
    // Schedule::Occurrence::EveryOtherWeek
    sch.setOccurrence(Schedule::Occurrence::EveryOtherWeek);
    startDate.setDate(2009, 2, 5);
    endDate.setDate(2009, 4, 3);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 2, 5));
    QCOMPARE(list[1], QDate(2009, 2, 19));
    QCOMPARE(list[2], QDate(2009, 3, 5));
    QCOMPARE(list[3], QDate(2009, 3, 19));
    QCOMPARE(list[4], QDate(2009, 4, 2));
    // Schedule::Occurrence::Fortnightly
    sch.setOccurrence(Schedule::Occurrence::Fortnightly);
    startDate.setDate(2009, 4, 4);
    endDate.setDate(2009, 5, 31);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 4);
    // First one would fall on a Saturday and would get moved
    // to 3rd which is before start date so is not in list.
    // Would fall on a Saturday so gets moved to 17th.
    QCOMPARE(list[0], QDate(2009, 4, 17));
    // Would fall on a Saturday so gets moved to 1st.
    QCOMPARE(list[1], QDate(2009, 5, 1));
    // Would fall on a Saturday so gets moved to 15th.
    QCOMPARE(list[2], QDate(2009, 5, 15));
    // Would fall on a Saturday so gets moved to 29th.
    QCOMPARE(list[3], QDate(2009, 5, 29));
    // Schedule::Occurrence::EveryHalfMonth
    sch.setOccurrence(Schedule::Occurrence::EveryHalfMonth);
    startDate.setDate(2009, 6, 1);
    endDate.setDate(2009, 8, 11);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 6, 1));
    QCOMPARE(list[1], QDate(2009, 6, 16));
    QCOMPARE(list[2], QDate(2009, 7, 1));
    QCOMPARE(list[3], QDate(2009, 7, 16));
    // Would fall on a Saturday so gets moved to 31st.
    QCOMPARE(list[4], QDate(2009, 7, 31));
    // Schedule::Occurrence::EveryThreeWeeks
    sch.setOccurrence(Schedule::Occurrence::EveryThreeWeeks);
    startDate.setDate(2009, 8, 12);
    endDate.setDate(2009, 11, 12);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 8, 12));
    QCOMPARE(list[1], QDate(2009, 9, 2));
    QCOMPARE(list[2], QDate(2009, 9, 23));
    QCOMPARE(list[3], QDate(2009, 10, 14));
    QCOMPARE(list[4], QDate(2009, 11, 4));
    // Schedule::Occurrence::EveryFourWeeks
    sch.setOccurrence(Schedule::Occurrence::EveryFourWeeks);
    startDate.setDate(2009, 11, 13);
    endDate.setDate(2010, 3, 13);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 11, 13));
    QCOMPARE(list[1], QDate(2009, 12, 11));
    QCOMPARE(list[2], QDate(2010, 1, 8));
    QCOMPARE(list[3], QDate(2010, 2, 5));
    QCOMPARE(list[4], QDate(2010, 3, 5));
    // Schedule::Occurrence::EveryThirtyDays
    sch.setOccurrence(Schedule::Occurrence::EveryThirtyDays);
    startDate.setDate(2010, 3, 19);
    endDate.setDate(2010, 7, 19);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2010, 3, 19));
    // Would fall on a Sunday so gets moved to 16th.
    QCOMPARE(list[1], QDate(2010, 4, 16));
    QCOMPARE(list[2], QDate(2010, 5, 18));
    QCOMPARE(list[3], QDate(2010, 6, 17));
    // Would fall on a Saturday so gets moved to 16th.
    QCOMPARE(list[4], QDate(2010, 7, 16));
    // Schedule::Occurrence::EveryEightWeeks
    sch.setOccurrence(Schedule::Occurrence::EveryEightWeeks);
    startDate.setDate(2010, 7, 26);
    endDate.setDate(2011, 3, 26);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2010, 7, 26));
    QCOMPARE(list[1], QDate(2010, 9, 20));
    QCOMPARE(list[2], QDate(2010, 11, 15));
    QCOMPARE(list[3], QDate(2011, 1, 10));
    QCOMPARE(list[4], QDate(2011, 3, 7));
    // Schedule::Occurrence::EveryOtherMonth
    sch.setOccurrence(Schedule::Occurrence::EveryOtherMonth);
    startDate.setDate(2011, 3, 14);
    endDate.setDate(2011, 11, 20);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2011, 3, 14));
    // Would fall on a Saturday so gets moved to 13th.
    QCOMPARE(list[1], QDate(2011, 5, 13));
    QCOMPARE(list[2], QDate(2011, 7, 14));
    QCOMPARE(list[3], QDate(2011, 9, 14));
    QCOMPARE(list[4], QDate(2011, 11, 14));
    // Schedule::Occurrence::EveryThreeMonths
    sch.setOccurrence(Schedule::Occurrence::EveryThreeMonths);
    startDate.setDate(2011, 11, 15);
    endDate.setDate(2012, 11, 19);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2011, 11, 15));
    QCOMPARE(list[1], QDate(2012, 2, 15));
    QCOMPARE(list[2], QDate(2012, 5, 15));
    QCOMPARE(list[3], QDate(2012, 8, 15));
    QCOMPARE(list[4], QDate(2012, 11, 15));
    // Schedule::Occurrence::Quarterly
    sch.setOccurrence(Schedule::Occurrence::Quarterly);
    startDate.setDate(2012, 11, 20);
    endDate.setDate(2013, 11, 23);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2012, 11, 20));
    QCOMPARE(list[1], QDate(2013, 2, 20));
    QCOMPARE(list[2], QDate(2013, 5, 20));
    QCOMPARE(list[3], QDate(2013, 8, 20));
    QCOMPARE(list[4], QDate(2013, 11, 20));
    // Schedule::Occurrence::EveryFourMonths
    sch.setOccurrence(Schedule::Occurrence::EveryFourMonths);
    startDate.setDate(2013, 11, 21);
    endDate.setDate(2015, 3, 23);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2013, 11, 21));
    QCOMPARE(list[1], QDate(2014, 3, 21));
    QCOMPARE(list[2], QDate(2014, 7, 21));
    QCOMPARE(list[3], QDate(2014, 11, 21));
    // Would fall on a Saturday so gets moved to 20th.
    QCOMPARE(list[4], QDate(2015, 3, 20));
    // Schedule::Occurrence::TwiceYearly
    sch.setOccurrence(Schedule::Occurrence::TwiceYearly);
    startDate.setDate(2015, 3, 22);
    endDate.setDate(2017, 3, 29);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 4);
    // First date would fall on a Sunday which would get moved
    // to 20th which is before start date so not in list.
    QCOMPARE(list[0], QDate(2015, 9, 22));
    QCOMPARE(list[1], QDate(2016, 3, 22));
    QCOMPARE(list[2], QDate(2016, 9, 22));
    QCOMPARE(list[3], QDate(2017, 3, 22));
    // Schedule::Occurrence::Yearly
    sch.setOccurrence(Schedule::Occurrence::Yearly);
    startDate.setDate(2017, 3, 23);
    endDate.setDate(2021, 3, 29);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2017, 3, 23));
    QCOMPARE(list[1], QDate(2018, 3, 23));
    // Would fall on a Saturday so gets moved to 22nd.
    QCOMPARE(list[2], QDate(2019, 3, 22));
    QCOMPARE(list[3], QDate(2020, 3, 23));
    QCOMPARE(list[4], QDate(2021, 3, 23));
    // Schedule::Occurrence::EveryOtherYear
    sch.setOccurrence(Schedule::Occurrence::EveryOtherYear);
    startDate.setDate(2021, 3, 24);
    endDate.setDate(2029, 3, 30);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2021, 3, 24));
    QCOMPARE(list[1], QDate(2023, 3, 24));
    QCOMPARE(list[2], QDate(2025, 3, 24));
    QCOMPARE(list[3], QDate(2027, 3, 24));
    // Would fall on a Saturday so gets moved to 23rd.
    QCOMPARE(list[4], QDate(2029, 3, 23));
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::testHasReferenceTo()
{
  MyMoneySchedule sch;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SCHEDULE-CONTAINER>\n"
                     " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                     "  <PAYMENTS>\n"
                     "   <PAYMENT date=\"%3\" />\n"
                     "  </PAYMENTS>\n"
                     "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                     "   <SPLITS>\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                     "   </SPLITS>\n"
                     "   <KEYVALUEPAIRS>\n"
                     "    <PAIR key=\"key\" value=\"value\" />\n"
                     "   </KEYVALUEPAIRS>\n"
                     "  </TRANSACTION>\n"
                     " </SCHEDULED_TX>\n"
                     "</SCHEDULE-CONTAINER>\n"
                   ).arg(QDate::currentDate().toString(Qt::ISODate))
                   .arg(QDate::currentDate().toString(Qt::ISODate))
                   .arg(QDate::currentDate().toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  setupAccounts();
  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  QCOMPARE(sch.hasReferenceTo(QLatin1String("P000001")), true);
  QCOMPARE(sch.hasReferenceTo(QLatin1String("A000276")), true);
  QCOMPARE(sch.hasReferenceTo(QLatin1String("A000076")), true);
  QCOMPARE(sch.hasReferenceTo(QLatin1String("EUR")), true);
}

void MyMoneyXmlContentHandlerTest::testPaidEarlyOneTime()
{
// this tries to figure out what's wrong with
// https://bugs.kde.org/show_bug.cgi?id=231029

  MyMoneySchedule sch;
  QDate paymentInFuture = QDate::currentDate().addDays(7);

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SCHEDULE-CONTAINER>\n"
                     " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"0\" weekendOption=\"1\" lastPayment=\"%2\" paymentType=\"2\" endDate=\"%3\" type=\"4\" id=\"SCH0042\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                     "  <PAYMENTS/>\n"
                     "  <TRANSACTION postdate=\"\" memo=\"\" id=\"\" commodity=\"GBP\" entrydate=\"\" >\n"
                     "   <SPLITS>\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Transfer\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" id=\"S0001\" account=\"A000076\" />\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"Transfer\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"-96379/100\" id=\"S0002\" account=\"A000276\" />\n"
                     "   </SPLITS>\n"
                     "  </TRANSACTION>\n"
                     " </SCHEDULED_TX>\n"
                     "</SCHEDULE-CONTAINER>\n"
                   ).arg(paymentInFuture.toString(Qt::ISODate))
                   .arg(paymentInFuture.toString(Qt::ISODate))
                   .arg(paymentInFuture.toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  setupAccounts();
  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QCOMPARE(sch.isFinished(), true);
    QCOMPARE(sch.occurrence(), Schedule::Occurrence::Monthly);
    QCOMPARE(sch.paymentDates(QDate::currentDate(), QDate::currentDate().addDays(21)).count(), 0);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyXmlContentHandlerTest::testReplaceId()
{
  MyMoneySchedule sch;
  QDate paymentInFuture = QDate::currentDate().addDays(7);

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SCHEDULE-CONTAINER>\n"
                     " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"0\" weekendOption=\"1\" lastPayment=\"%2\" paymentType=\"2\" endDate=\"%3\" type=\"4\" id=\"SCH0042\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                     "  <PAYMENTS/>\n"
                     "  <TRANSACTION postdate=\"\" memo=\"\" id=\"\" commodity=\"GBP\" entrydate=\"\" >\n"
                     "   <SPLITS>\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Transfer\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" id=\"S0001\" account=\"A000076\" />\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"Transfer\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"-96379/100\" id=\"S0002\" account=\"A000276\" />\n"
                     "   </SPLITS>\n"
                     "  </TRANSACTION>\n"
                     " </SCHEDULED_TX>\n"
                     "</SCHEDULE-CONTAINER>\n"
                   ).arg(paymentInFuture.toString(Qt::ISODate))
                   .arg(paymentInFuture.toString(Qt::ISODate))
                   .arg(paymentInFuture.toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();
  setupAccounts();

  try {
    sch = MyMoneyXmlContentHandler::readSchedule(node);
    QCOMPARE(sch.transaction().postDate().isValid(), false);
    QCOMPARE(sch.transaction().splits()[0].accountId(), QLatin1String("A000076"));
    QCOMPARE(sch.transaction().splits()[1].accountId(), QLatin1String("A000276"));
    QCOMPARE(sch.replaceId(QLatin1String("A000079"), QLatin1String("A000076")), true);
    QCOMPARE(sch.transaction().splits()[0].accountId(), QLatin1String("A000079"));
    QCOMPARE(sch.transaction().splits()[1].accountId(), QLatin1String("A000276"));

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

}
