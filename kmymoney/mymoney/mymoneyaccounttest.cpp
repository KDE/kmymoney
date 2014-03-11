/***************************************************************************
                          mymoneyaccounttest.cpp
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

#include "mymoneyaccounttest.h"

#include <QtTest/QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyAccountTest;

#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneysplit.h"

QTEST_MAIN(MyMoneyAccountTest)

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
  QVERIFY(a.accountType() == MyMoneyAccount::UnknownAccountType);
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
  r.setAccountType(MyMoneyAccount::Asset);
  r.setOpeningDate(QDate::currentDate());
  r.setLastModified(QDate::currentDate());
  r.setDescription("Desc");
  r.setNumber("465500");
  r.setParentAccountId(parent);
  r.setValue(QString("key"), "value");
  s.setShares(MyMoneyMoney(1, 1));
  r.adjustBalance(s);
  QVERIFY(r.m_kvp.count() == 1);
  QVERIFY(r.value("key") == "value");

  MyMoneyAccount a(id, r);

  QVERIFY(a.id() == id);
  QVERIFY(a.institutionId().isEmpty());
  QVERIFY(a.accountType() == MyMoneyAccount::Asset);
  QVERIFY(a.openingDate() == QDate::currentDate());
  QVERIFY(a.lastModified() == QDate::currentDate());
  QVERIFY(a.number() == "465500");
  QVERIFY(a.description() == "Desc");
  QVERIFY(a.accountList().count() == 0);
  QVERIFY(a.parentAccountId() == "Parent");
  QVERIFY(a.balance() == MyMoneyMoney(1, 1));

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
  a.setAccountType(MyMoneyAccount::MoneyMarket);

  QVERIFY(a.name() == "Account");
  QVERIFY(a.institutionId() == "Institution1");
  QVERIFY(a.lastModified() == today);
  QVERIFY(a.description() == "Desc");
  QVERIFY(a.number() == "123456");
  QVERIFY(a.accountType() == MyMoneyAccount::MoneyMarket);
}

void MyMoneyAccountTest::testCopyConstructor()
{
  QString id = "A000001";
  QString institutionid = "B000001";
  QString parent = "ParentAccount";
  MyMoneyAccount r;
  r.setAccountType(MyMoneyAccount::Expense);
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
  QVERIFY(b.accountType() == MyMoneyAccount::Expense);
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
  a.setAccountType(MyMoneyAccount::Checkings);
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
  QVERIFY(b.accountType() == MyMoneyAccount::Checkings);
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
  s.setAction(MyMoneySplit::ActionSplitShares);
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
  a.setAccountType(MyMoneyAccount::Checkings);

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
  a.setAccountType(MyMoneyAccount::Asset);
  a.setParentAccountId("P-ID");
  a.setId("A-ID");
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

  a.setAccountType(MyMoneyAccount::Liability);
  QVERIFY(!(b == a));
  b = a;

  a.setParentAccountId("P-noID");
  QVERIFY(!(b == a));
  b = a;

  a.setId("A-noID");
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
  r.setAccountType(MyMoneyAccount::Asset);
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

  // QVERIFY(r.m_kvp.count() == 1);
  // QVERIFY(r.value("key") == "value");

  MyMoneyAccount a(id, r);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("ACCOUNT-CONTAINER");
  doc.appendChild(el);
  a.writeXML(doc, el);

  QString ref = QString(
                  "<!DOCTYPE TEST>\n"
                  "<ACCOUNT-CONTAINER>\n"
                  " <ACCOUNT description=\"Desc\" parentaccount=\"Parent\" opened=\"%2\" number=\"465500\" lastmodified=\"%1\" type=\"9\" id=\"A000001\" lastreconciled=\"\" institution=\"B000001\" name=\"AccountName\" >\n"
                  "  <SUBACCOUNTS>\n"
                  "   <SUBACCOUNT id=\"A000002\" />\n"
                  "  </SUBACCOUNTS>\n"
                  "  <KEYVALUEPAIRS>\n"
                  "   <PAIR key=\"key\" value=\"value\" />\n"
                  "   <PAIR key=\"reconciliationHistory\" value=\"2011-01-01:123/100;2011-02-01:114/25\"/>\n"
                  "  </KEYVALUEPAIRS>\n"
                  " </ACCOUNT>\n"
                  "</ACCOUNT-CONTAINER>\n").
                arg(QDate::currentDate().toString(Qt::ISODate)).arg(QDate::currentDate().toString(Qt::ISODate));

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  ref.replace(QString(" />\n"), QString("/>\n"));
  ref.replace(QString(" >\n"), QString(">\n"));
#endif

  // qDebug("ref = '%s'", qPrintable(ref));
  // qDebug("doc = '%s'", qPrintable(doc.toString()));

  QVERIFY(doc.toString() == ref);
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
    QVERIFY(a.m_name == "AccountName");
    QVERIFY(a.m_parentAccount == "Parent");
    QVERIFY(a.m_lastModified == QDate::currentDate());
    QVERIFY(a.m_lastReconciliationDate == QDate());
    QVERIFY(a.m_institution == "B000001");
    QVERIFY(a.m_number == "465500");
    QVERIFY(a.m_openingDate == QDate::currentDate());
    QVERIFY(a.m_accountType == MyMoneyAccount::Asset);
    QVERIFY(a.m_description == "Desc");
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

void MyMoneyAccountTest::testHasReferenceTo(void)
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

void MyMoneyAccountTest::testSetClosed(void)
{
  MyMoneyAccount a;

  QVERIFY(a.isClosed() == false);
  a.setClosed(true);
  QVERIFY(a.isClosed() == true);
  a.setClosed(false);
  QVERIFY(a.isClosed() == false);
}

void MyMoneyAccountTest::testIsIncomeExpense(void)
{
  MyMoneyAccount a;

  a.setAccountType(MyMoneyAccount::UnknownAccountType);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Checkings);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Savings);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Cash);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::CreditCard);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Loan);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::CertificateDep);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Investment);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::MoneyMarket);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Asset);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Liability);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Currency);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Income);
  QVERIFY(a.isIncomeExpense() == true);

  a.setAccountType(MyMoneyAccount::Expense);
  QVERIFY(a.isIncomeExpense() == true);

  a.setAccountType(MyMoneyAccount::AssetLoan);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Stock);
  QVERIFY(a.isIncomeExpense() == false);

  a.setAccountType(MyMoneyAccount::Equity);
  QVERIFY(a.isIncomeExpense() == false);
}

void MyMoneyAccountTest::testIsAssetLiability(void)
{
  MyMoneyAccount a;

  a.setAccountType(MyMoneyAccount::UnknownAccountType);
  QVERIFY(a.isAssetLiability() == false);

  a.setAccountType(MyMoneyAccount::Checkings);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Savings);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Cash);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::CreditCard);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Loan);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::CertificateDep);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Investment);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::MoneyMarket);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Asset);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Liability);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Currency);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Income);
  QVERIFY(a.isAssetLiability() == false);

  a.setAccountType(MyMoneyAccount::Expense);
  QVERIFY(a.isAssetLiability() == false);

  a.setAccountType(MyMoneyAccount::AssetLoan);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Stock);
  QVERIFY(a.isAssetLiability() == true);

  a.setAccountType(MyMoneyAccount::Equity);
  QVERIFY(a.isAssetLiability() == false);
}

void MyMoneyAccountTest::testIsLoan(void)
{
  MyMoneyAccount a;

  a.setAccountType(MyMoneyAccount::UnknownAccountType);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Checkings);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Savings);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Cash);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::CreditCard);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Loan);
  QVERIFY(a.isLoan() == true);

  a.setAccountType(MyMoneyAccount::CertificateDep);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Investment);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::MoneyMarket);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Asset);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Liability);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Currency);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Income);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Expense);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::AssetLoan);
  QVERIFY(a.isLoan() == true);

  a.setAccountType(MyMoneyAccount::Stock);
  QVERIFY(a.isLoan() == false);

  a.setAccountType(MyMoneyAccount::Equity);
  QVERIFY(a.isLoan() == false);
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

#include "mymoneyaccounttest.moc"

