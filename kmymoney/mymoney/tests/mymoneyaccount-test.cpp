/*
    SPDX-FileCopyrightText: 2002-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyaccount-test.h"

#include <QTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyAccountTest;

#include "mymoneyaccount.h"
#include "mymoneyaccount_p.h"
#include "mymoneyaccountloan.h"
#include "mymoneyenums.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"

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

    QCOMPARE(b.name(), "Account");
    QCOMPARE(b.institutionId(), "Inst1");
    QCOMPARE(b.accountType(), eMyMoney::Account::Type::Checkings);
    QCOMPARE(b.lastModified(), QDate());
    QCOMPARE(b.openingDate(), a.openingDate());
    QCOMPARE(b.description(), "Bla");
    QCOMPARE(b.number(), "assigned Number");
    QCOMPARE(b.value("Key"), "Value");
    QCOMPARE(b.accountList().count(), 1);
    QCOMPARE(b.accountList().at(0), "ChildAccount");

    MyMoneyAccountLoan c;

    b.setLastModified(QDate::currentDate());

    c = a;

    QCOMPARE(c.name(), "Account");
    QCOMPARE(c.institutionId(), "Inst1");
    QCOMPARE(c.accountType(), eMyMoney::Account::Type::Checkings);
    QCOMPARE(c.lastModified(), QDate());
    QCOMPARE(c.openingDate(), a.openingDate());
    QCOMPARE(c.description(), "Bla");
    QCOMPARE(c.number(), "assigned Number");
    QCOMPARE(c.value("Key"), "Value");
    QCOMPARE(c.accountList().count(), 1);
    QCOMPARE(c.accountList().at(0), "ChildAccount");
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

    QCOMPARE(a.isClosed(), false);
    a.setClosed(true);
    QCOMPARE(a.isClosed(), true);
    a.setClosed(false);
    QCOMPARE(a.isClosed(), false);
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
    QVERIFY(a.reconciliationHistory().value(QDate(2011, 1, 2)) == MyMoneyMoney(123, 100));
    QVERIFY(a.reconciliationHistory().value(QDate(2011, 1, 1)) == MyMoneyMoney());
    QVERIFY(a.reconciliationHistory().value(QDate(2011, 1, 3)) == MyMoneyMoney());

    QVERIFY(a.addReconciliation(QDate(2011, 2, 1), MyMoneyMoney(456, 100)) == true);
    QVERIFY(a.reconciliationHistory().value(QDate(2011, 1, 2)) == MyMoneyMoney(123, 100));
    QVERIFY(a.reconciliationHistory().value(QDate(2011, 1, 1)) == MyMoneyMoney());
    QVERIFY(a.reconciliationHistory().value(QDate(2011, 1, 3)) == MyMoneyMoney());
    QVERIFY(a.reconciliationHistory().value(QDate(2011, 2, 1)) == MyMoneyMoney(456, 100));
    QVERIFY(a.reconciliationHistory().count() == 2);
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

void MyMoneyAccountTest::testBudgetOptions()
{
    MyMoneyAccount a;
    a.setAccountType(eMyMoney::Account::Type::Asset);
    QCOMPARE(a.accountType(), eMyMoney::Account::Type::Asset);
    QCOMPARE(a.budgetAccountType(), eMyMoney::Account::Type::Asset);

    a.setBudgetAccountType(eMyMoney::Account::Type::Income);
    QCOMPARE(a.accountType(), eMyMoney::Account::Type::Asset);
    QCOMPARE(a.budgetAccountType(), eMyMoney::Account::Type::Income);

    a.setBudgetAccountType(eMyMoney::Account::Type::Expense);
    QCOMPARE(a.accountType(), eMyMoney::Account::Type::Asset);
    QCOMPARE(a.budgetAccountType(), eMyMoney::Account::Type::Expense);

    a.setBudgetAccountType(eMyMoney::Account::Type::Investment);
    QCOMPARE(a.accountType(), eMyMoney::Account::Type::Asset);
    QCOMPARE(a.budgetAccountType(), eMyMoney::Account::Type::Asset);
}

void MyMoneyAccountTest::testBalanceFactor_data()
{
    QTest::addColumn<eMyMoney::Account::Type>("accountType");
    QTest::addColumn<MyMoneyMoney>("factor");

    // positive and null is debit
    QTest::newRow("unknown") << eMyMoney::Account::Type::Unknown << MyMoneyMoney::ONE;
    QTest::newRow("checking") << eMyMoney::Account::Type::Checkings << MyMoneyMoney::ONE;
    QTest::newRow("savings") << eMyMoney::Account::Type::Savings << MyMoneyMoney::ONE;
    QTest::newRow("cash") << eMyMoney::Account::Type::Cash << MyMoneyMoney::ONE;
    QTest::newRow("investment") << eMyMoney::Account::Type::Investment << MyMoneyMoney::ONE;
    QTest::newRow("asset") << eMyMoney::Account::Type::Asset << MyMoneyMoney::ONE;
    QTest::newRow("currency") << eMyMoney::Account::Type::Currency << MyMoneyMoney::ONE;
    QTest::newRow("moneymarket") << eMyMoney::Account::Type::MoneyMarket << MyMoneyMoney::ONE;
    QTest::newRow("certificatedeposit") << eMyMoney::Account::Type::CertificateDep << MyMoneyMoney::ONE;
    QTest::newRow("assetloan") << eMyMoney::Account::Type::AssetLoan << MyMoneyMoney::ONE;
    QTest::newRow("stock") << eMyMoney::Account::Type::Stock << MyMoneyMoney::ONE;
    QTest::newRow("income") << eMyMoney::Account::Type::Income << MyMoneyMoney::ONE;

    QTest::newRow("expense") << eMyMoney::Account::Type::Expense << MyMoneyMoney::MINUS_ONE;
    QTest::newRow("creditcard") << eMyMoney::Account::Type::CreditCard << MyMoneyMoney::MINUS_ONE;
    QTest::newRow("liability") << eMyMoney::Account::Type::Liability << MyMoneyMoney::MINUS_ONE;
    QTest::newRow("loan") << eMyMoney::Account::Type::Loan << MyMoneyMoney::MINUS_ONE;
    QTest::newRow("equity") << eMyMoney::Account::Type::Equity << MyMoneyMoney::MINUS_ONE;
}
void MyMoneyAccountTest::testBalanceFactor()
{
    QFETCH(eMyMoney::Account::Type, accountType);
    QFETCH(MyMoneyMoney, factor);

    QCOMPARE(MyMoneyAccount::balanceFactor(accountType), factor);

    MyMoneyAccount account;
    account.setAccountType(accountType);
    QCOMPARE(account.balanceFactor(), factor);
}

void MyMoneyAccountTest::testPayeeCreation_data()
{
    QTest::addColumn<eMyMoney::Account::PayeeCreation>("creationMode");

    QTest::newRow("AskUser") << eMyMoney::Account::PayeeCreation::AskUser;
    QTest::newRow("AutomaticCreaation") << eMyMoney::Account::PayeeCreation::AutomaticCreaation;
    QTest::newRow("NoCreation") << eMyMoney::Account::PayeeCreation::NoCreation;
    QTest::newRow("ApplicationDefault") << eMyMoney::Account::PayeeCreation::ApplicationDefault;
}

void MyMoneyAccountTest::testPayeeCreation()
{
    QFETCH(eMyMoney::Account::PayeeCreation, creationMode);

    MyMoneyAccount account;
    QCOMPARE(account.payeeCreation(), eMyMoney::Account::PayeeCreation::ApplicationDefault);
    account.setPayeeCreation(creationMode);
    QCOMPARE(account.payeeCreation(), creationMode);
}

void MyMoneyAccountTest::testPriceMode_data()
{
    QTest::addColumn<eMyMoney::Invest::PriceMode>("priceMode");

    QTest::newRow("Price") << eMyMoney::Invest::PriceMode::Price;
    QTest::newRow("PricePerShare") << eMyMoney::Invest::PriceMode::PricePerShare;
    QTest::newRow("PricePerTransaction") << eMyMoney::Invest::PriceMode::PricePerTransaction;
}

void MyMoneyAccountTest::testPriceMode()
{
    QFETCH(eMyMoney::Invest::PriceMode, priceMode);

    MyMoneyAccount account;
    QCOMPARE(account.priceMode(), eMyMoney::Invest::PriceMode::Price);
    account.setPriceMode(priceMode);
    QCOMPARE(account.priceMode(), priceMode);
}
