/*
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004       Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include "mymoneyaccount-test.h"

#include <QtTest>
#include <QDomDocument>
#include <QDomElement>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyAccountTest;

#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyaccount_p.h"
#include "mymoneysplit.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

QTEST_GUILESS_MAIN(MyMoneyAccountTest)

void MyMoneyAccountTest::init()
{
}

void MyMoneyAccountTest::cleanup()
{
}

void MyMoneyAccountTest::testEmptyConstructor()
{
  MyMoneyAccount a;

  QVERIFY(a.id().isEmpty());
  QVERIFY(a.name().isEmpty());
  QVERIFY(a.accountType() == eMyMoney::Account::Type::Unknown);
  QVERIFY(a.openingDate() == QDate());
  QVERIFY(a.lastModified() == QDate());
  QVERIFY(a.lastReconciliationDate() == QDate());
  QVERIFY(a.accountList().count() == 0);
  QVERIFY(a.balance().isZero());
}

void MyMoneyAccountTest::testConstructor()
{
  QString id = "A000001";
  QString institutionid = "B000001";
  QString parent = "Parent";
  MyMoneyAccount r;
  MyMoneySplit s;
  r.setAccountType(eMyMoney::Account::Type::Asset);
  r.setOpeningDate(QDate::currentDate());
  r.setLastModified(QDate::currentDate());
  r.setDescription("Desc");
  r.setNumber("465500");
  r.setParentAccountId(parent);
  r.setValue(QString("key"), "value");
  s.setShares(MyMoneyMoney::ONE);
  r.adjustBalance(s);
  QVERIFY(r.pairs().count() == 1);
  QVERIFY(r.value("key") == "value");

  MyMoneyAccount a(id, r);

  QVERIFY(a.id() == id);
  QVERIFY(a.institutionId().isEmpty());
  QVERIFY(a.accountType() == eMyMoney::Account::Type::Asset);
  QVERIFY(a.openingDate() == QDate::currentDate());
  QVERIFY(a.lastModified() == QDate::currentDate());
  QVERIFY(a.number() == "465500");
  QVERIFY(a.description() == "Desc");
  QVERIFY(a.accountList().count() == 0);
  QVERIFY(a.parentAccountId() == "Parent");
  QVERIFY(a.balance() == MyMoneyMoney::ONE);

  QMap<QString, QString> copy;
  copy = r.pairs();
  QVERIFY(copy.count() == 1);
  QVERIFY(copy[QString("key")] == "value");
}

void MyMoneyAccountTest::testSetFunctions()
{
  MyMoneyAccount a;

  QDate today(QDate::currentDate());
  QVERIFY(a.name().isEmpty());
  QVERIFY(a.lastModified() == QDate());
  QVERIFY(a.description().isEmpty());

  a.setName("Account");
  a.setInstitutionId("Institution1");
  a.setLastModified(today);
  a.setDescription("Desc");
  a.setNumber("123456");
  a.setAccountType(eMyMoney::Account::Type::MoneyMarket);

  QVERIFY(a.name() == "Account");
  QVERIFY(a.institutionId() == "Institution1");
  QVERIFY(a.lastModified() == today);
  QVERIFY(a.description() == "Desc");
  QVERIFY(a.number() == "123456");
  QVERIFY(a.accountType() == eMyMoney::Account::Type::MoneyMarket);
}

void MyMoneyAccountTest::testCopyConstructor()
{
  QString id = "A000001";
  QString institutionid = "B000001";
  QString parent = "ParentAccount";
  MyMoneyAccount r;
  r.setAccountType(eMyMoney::Account::Type::Expense);
  r.setOpeningDate(QDate::currentDate());
  r.setLastModified(QDate::currentDate());
  r.setName("Account");
  r.setInstitutionId("Inst1");
  r.setDescription("Desc1");
  r.setNumber("Number");
  r.setParentAccountId(parent);
  r.setValue("Key", "Value");

  MyMoneyAccount a(id, r);
  a.setInstitutionId(institutionid);

  MyMoneyAccount b(a);

  QVERIFY(b.name() == "Account");
  QVERIFY(b.institutionId() == institutionid);
  QVERIFY(b.accountType() == eMyMoney::Account::Type::Expense);
  QVERIFY(b.lastModified() == QDate::currentDate());
  QVERIFY(b.openingDate() == QDate::currentDate());
  QVERIFY(b.description() == "Desc1");
  QVERIFY(b.number() == "Number");
  QVERIFY(b.parentAccountId() == "ParentAccount");

  QVERIFY(b.value("Key") == "Value");
}

void MyMoneyAccountTest::testAssignmentConstructor()
{
  MyMoneyAccount a;
  a.setAccountType(eMyMoney::Account::Type::Checkings);
  a.setName("Account");
  a.setInstitutionId("Inst1");
  a.setDescription("Bla");
  a.setNumber("assigned Number");
  a.setValue("Key", "Value");
  a.addAccountId("ChildAccount");

  MyMoneyAccount b;

  b.setLastModified(QDate::currentDate());

  b = a;

  QVERIFY(b.name() == "Account");
  QVERIFY(b.institutionId() == "Inst1");
  QVERIFY(b.accountType() == eMyMoney::Account::Type::Checkings);
  QVERIFY(b.lastModified() == QDate());
  QVERIFY(b.openingDate() == a.openingDate());
  QVERIFY(b.description() == "Bla");
  QVERIFY(b.number() == "assigned Number");
  QVERIFY(b.value("Key") == "Value");
  QVERIFY(b.accountList().count() == 1);
  QVERIFY(b.accountList()[0] == "ChildAccount");
}

void MyMoneyAccountTest::testAdjustBalance()
{
  MyMoneyAccount a;
  MyMoneySplit s;
  s.setShares(MyMoneyMoney(3, 1));
  a.adjustBalance(s);
  QVERIFY(a.balance() == MyMoneyMoney(3, 1));
  s.setShares(MyMoneyMoney(5, 1));
  a.adjustBalance(s, true);
  QVERIFY(a.balance() == MyMoneyMoney(-2, 1));
  s.setShares(MyMoneyMoney(2, 1));
  s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares));
  a.adjustBalance(s);
  QVERIFY(a.balance() == MyMoneyMoney(-4, 1));
  s.setShares(MyMoneyMoney(4, 1));
  s.setAction(QString());
  a.adjustBalance(s);
  QVERIFY(a.balance().isZero());
}

void MyMoneyAccountTest::testSubAccounts()
{
  MyMoneyAccount a;
  a.setAccountType(eMyMoney::Account::Type::Checkings);

  a.addAccountId("Subaccount1");
  QVERIFY(a.accountList().count() == 1);
  a.addAccountId("Subaccount1");
  QVERIFY(a.accountList().count() == 1);
  a.addAccountId("Subaccount2");
  QVERIFY(a.accountList().count() == 2);

}

void MyMoneyAccountTest::testEquality()
{
  MyMoneyAccount a;

  a.setLastModified(QDate::currentDate());
  a.setName("Name");
  a.setNumber("Number");
  a.setDescription("Desc");
  a.setInstitutionId("I-ID");
  a.setOpeningDate(QDate::currentDate());
  a.setLastReconciliationDate(QDate::currentDate());
  a.setAccountType(eMyMoney::Account::Type::Asset);
  a.setParentAccountId("P-ID");
  a.d_func()->setId("A-ID");
  a.setCurrencyId("C-ID");
  a.setValue("Key", "Value");

  MyMoneyAccount b;

  b = a;
  QVERIFY(b == a);

  a.setName("Noname");
  QVERIFY(!(b == a));
  b = a;

  a.setLastModified(QDate::currentDate().addDays(-1));
  QVERIFY(!(b == a));
  b = a;

  a.setNumber("Nonumber");
  QVERIFY(!(b == a));
  b = a;

  a.setDescription("NoDesc");
  QVERIFY(!(b == a));
  b = a;

  a.setInstitutionId("I-noID");
  QVERIFY(!(b == a));
  b = a;

  a.setOpeningDate(QDate::currentDate().addDays(-1));
  QVERIFY(!(b == a));
  b = a;

  a.setLastReconciliationDate(QDate::currentDate().addDays(-1));
  QVERIFY(!(b == a));
  b = a;

  a.setAccountType(eMyMoney::Account::Type::Liability);
  QVERIFY(!(b == a));
  b = a;

  a.setParentAccountId("P-noID");
  QVERIFY(!(b == a));
  b = a;

  a.d_func()->setId("A-noID");
  QVERIFY(!(b == a));
  b = a;

  a.setCurrencyId("C-noID");
  QVERIFY(!(b == a));
  b = a;

  a.setValue("Key", "noValue");
  QVERIFY(!(b == a));
  b = a;

  a.setValue("noKey", "Value");
  QVERIFY(!(b == a));
  b = a;

}

void MyMoneyAccountTest::testWriteXML()
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
  a.writeXML(doc, el);

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

void MyMoneyAccountTest::testReadXML()
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
    a = MyMoneyAccount(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  a.addAccountId("TEST");
  a.setValue("KEY", "VALUE");

  try {
    a = MyMoneyAccount(node);
    QVERIFY(a.id() == "A000001");
    QVERIFY(a.name() == "AccountName");
    QVERIFY(a.parentAccountId() == "Parent");
    QVERIFY(a.lastModified() == QDate::currentDate());
    QVERIFY(a.lastReconciliationDate() == QDate());
    QVERIFY(a.institutionId() == "B000001");
    QVERIFY(a.number() == "465500");
    QVERIFY(a.openingDate() == QDate::currentDate());
    QVERIFY(a.accountType() == eMyMoney::Account::Type::Asset);
    QVERIFY(a.description() == "Desc");
    QVERIFY(a.accountList().count() == 2);
    QVERIFY(a.accountList()[0] == "A000002");
    QVERIFY(a.accountList()[1] == "A000003");
    QVERIFY(a.pairs().count() == 4);
    QVERIFY(a.value("key") == "value");
    QVERIFY(a.value("Key") == "Value");
    QVERIFY(a.value("lastStatementDate").isEmpty());
    QVERIFY(a.reconciliationHistory().count() == 2);
    QVERIFY(a.reconciliationHistory()[QDate(2011, 1, 1)] == MyMoneyMoney(123, 100));
    QVERIFY(a.reconciliationHistory()[QDate(2011, 2, 1)] == MyMoneyMoney(456, 100));
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyAccountTest::testHasReferenceTo()
{
  MyMoneyAccount a;

  a.setInstitutionId("I0001");
  a.addAccountId("A_001");
  a.addAccountId("A_002");
  a.setParentAccountId("A_Parent");
  a.setCurrencyId("Currency");

  QVERIFY(a.hasReferenceTo("I0001") == true);
  QVERIFY(a.hasReferenceTo("I0002") == false);
  QVERIFY(a.hasReferenceTo("A_001") == false);
  QVERIFY(a.hasReferenceTo("A_Parent") == true);
  QVERIFY(a.hasReferenceTo("Currency") == true);
}

void MyMoneyAccountTest::testSetClosed()
{
  MyMoneyAccount a;

  QVERIFY(a.isClosed() == false);
  a.setClosed(true);
  QVERIFY(a.isClosed() == true);
  a.setClosed(false);
  QVERIFY(a.isClosed() == false);
}

void MyMoneyAccountTest::specialAccountTypes_data()
{
  QTest::addColumn<eMyMoney::Account::Type>("accountType");
  QTest::addColumn<bool>("incomeExpense");
  QTest::addColumn<bool>("assetLibility");
  QTest::addColumn<bool>("loan");

  // positive and null is debit
  QTest::newRow("unknown") << eMyMoney::Account::Type::Unknown << false << false << false;
  QTest::newRow("checking") << eMyMoney::Account::Type::Checkings << false << true << false;
  QTest::newRow("savings") << eMyMoney::Account::Type::Savings << false << true << false;
  QTest::newRow("cash") << eMyMoney::Account::Type::Cash << false << true << false;
  QTest::newRow("investment") << eMyMoney::Account::Type::Investment << false << true << false;
  QTest::newRow("asset") << eMyMoney::Account::Type::Asset << false << true << false;
  QTest::newRow("currency") << eMyMoney::Account::Type::Currency << false << true << false;
  QTest::newRow("expense") << eMyMoney::Account::Type::Expense << true << false << false;
  QTest::newRow("moneymarket") << eMyMoney::Account::Type::MoneyMarket << false << true << false;
  QTest::newRow("certificatedeposit") << eMyMoney::Account::Type::CertificateDep << false << true << false;
  QTest::newRow("assetloan") << eMyMoney::Account::Type::AssetLoan << false << true << true;
  QTest::newRow("stock") << eMyMoney::Account::Type::Stock << false << true << false;
  QTest::newRow("creditcard") << eMyMoney::Account::Type::CreditCard << false << true << false;
  QTest::newRow("loan") << eMyMoney::Account::Type::Loan << false << true << true;
  QTest::newRow("liability") << eMyMoney::Account::Type::Liability << false << true << false;
  QTest::newRow("income") << eMyMoney::Account::Type::Income << true << false << false;
  QTest::newRow("equity") << eMyMoney::Account::Type::Equity << false << false << false;
}

void MyMoneyAccountTest::specialAccountTypes()
{
  QFETCH(eMyMoney::Account::Type, accountType);
  QFETCH(bool, incomeExpense);
  QFETCH(bool, assetLibility);
  QFETCH(bool, loan);

  MyMoneyAccount a;
  a.setAccountType(accountType);
  QCOMPARE(a.isIncomeExpense(), incomeExpense);
  QCOMPARE(a.isAssetLiability(), assetLibility);
  QCOMPARE(a.isLoan(), loan);
}

void MyMoneyAccountTest::addReconciliation()
{
  MyMoneyAccount a;

  QVERIFY(a.addReconciliation(QDate(2011, 1, 2), MyMoneyMoney(123, 100)) == true);
  QVERIFY(a.reconciliationHistory().count() == 1);
  QVERIFY(a.addReconciliation(QDate(2011, 2, 1), MyMoneyMoney(456, 100)) == true);
  QVERIFY(a.reconciliationHistory().count() == 2);
  QVERIFY(a.addReconciliation(QDate(2011, 2, 1), MyMoneyMoney(789, 100)) == true);
  QVERIFY(a.reconciliationHistory().count() == 2);
  QVERIFY(a.reconciliationHistory().values().last() == MyMoneyMoney(789, 100));
}

void MyMoneyAccountTest::reconciliationHistory()
{
  MyMoneyAccount a;

  QVERIFY(a.reconciliationHistory().isEmpty() == true);
  QVERIFY(a.addReconciliation(QDate(2011, 1, 2), MyMoneyMoney(123, 100)) == true);
  QVERIFY(a.reconciliationHistory()[QDate(2011, 1, 2)] == MyMoneyMoney(123, 100));
  QVERIFY(a.reconciliationHistory()[QDate(2011, 1, 1)] == MyMoneyMoney());
  QVERIFY(a.reconciliationHistory()[QDate(2011, 1, 3)] == MyMoneyMoney());

  QVERIFY(a.addReconciliation(QDate(2011, 2, 1), MyMoneyMoney(456, 100)) == true);
  QVERIFY(a.reconciliationHistory()[QDate(2011, 1, 2)] == MyMoneyMoney(123, 100));
  QVERIFY(a.reconciliationHistory()[QDate(2011, 1, 1)] == MyMoneyMoney());
  QVERIFY(a.reconciliationHistory()[QDate(2011, 1, 3)] == MyMoneyMoney());
  QVERIFY(a.reconciliationHistory()[QDate(2011, 2, 1)] == MyMoneyMoney(456, 100));
  QVERIFY(a.reconciliationHistory().count() == 2);
}

void MyMoneyAccountTest::testElementNames()
{
  for (auto i = (int)Account::Element::SubAccount; i <= (int)Account::Element::OnlineBanking; ++i) {
    auto isEmpty = MyMoneyAccountPrivate::getElName(static_cast<Account::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyAccountTest::testAttributeNames()
{
  for (auto i = (int)Account::Attribute::ID; i < (int)Account::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyAccountPrivate::getAttrName(static_cast<Account::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyAccountTest::testHasOnlineMapping()
{
  MyMoneyAccount a;
  QCOMPARE(a.hasOnlineMapping(), false);
  MyMoneyKeyValueContainer kvp = a.onlineBankingSettings();
  kvp.setValue(QLatin1String("provider"), QLatin1String("bla"));
  a.setOnlineBankingSettings(kvp);
  QCOMPARE(a.hasOnlineMapping(), true);
}
