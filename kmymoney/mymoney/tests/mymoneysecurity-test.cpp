/*
    SPDX-FileCopyrightText: 2004-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneysecurity-test.h"

#include <QTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneySecurityTest;

#include "mymoneysecurity.h"
#include "mymoneysecurity_p.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

QTEST_GUILESS_MAIN(MyMoneySecurityTest)

void MyMoneySecurityTest::init()
{
    m = new MyMoneySecurity();
}

void MyMoneySecurityTest::cleanup()
{
    delete m;
}

void MyMoneySecurityTest::testEmptyConstructor()
{
    QVERIFY(m->id().isEmpty());
    QVERIFY(m->name().isEmpty());
    QVERIFY(m->tradingSymbol().isEmpty());
    QVERIFY(m->securityType() == eMyMoney::Security::Type::None);
    QVERIFY(m->tradingMarket().isEmpty());
    QVERIFY(m->tradingCurrency().isEmpty());
    QVERIFY(m->smallestCashFraction() == 100);
    QVERIFY(m->smallestAccountFraction() == 100);
}

void MyMoneySecurityTest::testCopyConstructor()
{
    MyMoneySecurity* n1 = new MyMoneySecurity("GUID1", *m);
    MyMoneySecurity n2(*n1);

    // QVERIFY(*n1 == n2);

    delete n1;
}

void MyMoneySecurityTest::testNonemptyConstructor()
{
    QDate date(2004, 4, 1);
    MyMoneyMoney val("1234/100");

    m->setName("name");
    m->setTradingSymbol("symbol");
    m->setSecurityType(eMyMoney::Security::Type::Currency);
    // m->addPriceHistory(date, val);

    MyMoneySecurity n("id", *m);

    QVERIFY(n.id() == QString("id"));
    QVERIFY(n.tradingSymbol() == QString("symbol"));
    QVERIFY(n.securityType() == eMyMoney::Security::Type::Currency);
    // QVERIFY(n.priceHistory().count() == 1);
}


void MyMoneySecurityTest::testSetFunctions()
{
    m->setName("Name");
    m->setTradingSymbol("Symbol");
    m->setTradingMarket("Market");
    m->setTradingCurrency("Currency");
    m->setSecurityType(eMyMoney::Security::Type::Stock);
    m->setSmallestAccountFraction(50);
    m->setSmallestCashFraction(2);

    QVERIFY(m->name() == "Name");
    QVERIFY(m->tradingSymbol() == "Symbol");
    QVERIFY(m->tradingMarket() == "Market");
    QVERIFY(m->tradingCurrency() == "Currency");
    QVERIFY(m->securityType() == eMyMoney::Security::Type::Stock);
    QVERIFY(m->smallestAccountFraction() == 50);
    QVERIFY(m->smallestCashFraction() == 2);
}

/*
void MyMoneySecurityTest::testMyMoneyFileConstructor() {
 MyMoneySecurity *t = new MyMoneySecurity("GUID", *n);

 QVERIFY(t->id() == "GUID");

 delete t;
}
*/

void MyMoneySecurityTest::testEquality()
{
    testSetFunctions();
    m->setValue("Key", "Value");

    MyMoneySecurity n;
    n = *m;

    QVERIFY(n == *m);
    n.setName("NewName");
    QVERIFY(!(n == *m));
    n = *m;
    n.setTradingSymbol("NewSymbol");
    QVERIFY(!(n == *m));
    n = *m;
    n.setTradingMarket("NewMarket");
    QVERIFY(!(n == *m));
    n = *m;
    n.setTradingCurrency("NewCurrency");
    QVERIFY(!(n == *m));
    n = *m;
    n.setSecurityType(eMyMoney::Security::Type::Currency);
    QVERIFY(!(n == *m));
    n = *m;
    n.setPricePrecision(8);
    QVERIFY(!(n == *m));
    n = *m;
    n.setSmallestCashFraction(20);
    QVERIFY(!(n == *m));
    n = *m;
    n.setValue("Key", "NewValue");
    QVERIFY(!(n == *m));
}

void MyMoneySecurityTest::testInequality()
{
    testSetFunctions();
    m->setValue("Key", "Value");

    MyMoneySecurity n;
    n = *m;

    QVERIFY(!(n != *m));
    n.setName("NewName");
    QVERIFY(n != *m);
    n = *m;
    n.setTradingSymbol("NewSymbol");
    QVERIFY(n != *m);
    n = *m;
    n.setTradingMarket("NewMarket");
    QVERIFY(n != *m);
    n = *m;
    n.setTradingCurrency("NewCurrency");
    QVERIFY(n != *m);
    n = *m;
    n.setSecurityType(eMyMoney::Security::Type::Currency);
    QVERIFY(n != *m);
    n = *m;
    n.setSmallestAccountFraction(40);
    QVERIFY(n != *m);
    n = *m;
    n.setSmallestCashFraction(20);
    QVERIFY(n != *m);
    n = *m;
    n.setValue("Key", "NewValue");
    QVERIFY(n != *m);
}

/*
void MyMoneySecurityTest::testAccountIDList () {
 MyMoneySecurity equity;
 QStringList list;
 QString id;

 // list must be empty
 list = institution.accountList();
 QVERIFY(list.count() == 0);

 // add one account
 institution.addAccountId("A000002");
 list = institution.accountList();
 QVERIFY(list.count() == 1);
 QVERIFY(list.contains("A000002") == 1);

 // adding same account shouldn't make a difference
 institution.addAccountId("A000002");
 list = institution.accountList();
 QVERIFY(list.count() == 1);
 QVERIFY(list.contains("A000002") == 1);

 // now add another account
 institution.addAccountId("A000001");
 list = institution.accountList();
 QVERIFY(list.count() == 2);
 QVERIFY(list.contains("A000002") == 1);
 QVERIFY(list.contains("A000001") == 1);

 id = institution.removeAccountId("A000001");
 QVERIFY(id == "A000001");
 list = institution.accountList();
 QVERIFY(list.count() == 1);
 QVERIFY(list.contains("A000002") == 1);

}
*/
