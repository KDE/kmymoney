/*
    SPDX-FileCopyrightText: 2002-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyfile-test.h"
#include <iostream>

#include <memory>
#include <QFile>
#include <QDataStream>
#include <QList>
#include <QTest>

#include "mymoneytestutils.h"
#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneysplit.h"
#include "mymoneyprice.h"
#include "mymoneypayee.h"
#include "mymoneyenums.h"
#include "onlinejob.h"
#include "payeesmodel.h"
#include "accountsmodel.h"

#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifiertyped.h"

QTEST_GUILESS_MAIN(MyMoneyFileTest)

MyMoneyFileTest::MyMoneyFileTest()
    : m(nullptr)
{
}

void MyMoneyFileTest::objectAdded(eMyMoney::File::Object type, const QString& id)
{
    Q_UNUSED(type);
    m_objectsAdded += id;
}

void MyMoneyFileTest::objectRemoved(eMyMoney::File::Object type, const QString& id)
{
    Q_UNUSED(type);
    m_objectsRemoved += id;
}

void MyMoneyFileTest::objectModified(eMyMoney::File::Object type, const QString& id)
{
    Q_UNUSED(type);
    m_objectsModified += id;
}

void MyMoneyFileTest::clearObjectLists()
{
    m_objectsAdded.clear();
    m_objectsModified.clear();
    m_objectsRemoved.clear();
    m_balanceChanged.clear();
    m_valueChanged.clear();
}

void MyMoneyFileTest::balanceChanged(const MyMoneyAccount& account)
{
    m_balanceChanged += account.id();
}

void MyMoneyFileTest::valueChanged(const MyMoneyAccount& account)
{
    m_valueChanged += account.id();
}

void MyMoneyFileTest::setupBaseCurrency()
{
    MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
    MyMoneyFileTransaction ft;
    try {
        m->currency(base.id());
    } catch (const MyMoneyException &e) {
        m->addCurrency(base);
    }
    m->setBaseCurrency(base);
    ft.commit();
}

// this method will be called once at the beginning of the test
void MyMoneyFileTest::initTestCase()
{
    m = MyMoneyFile::instance();

    connect(m, &MyMoneyFile::objectAdded, this, &MyMoneyFileTest::objectAdded);
    connect(m, &MyMoneyFile::objectRemoved, this, &MyMoneyFileTest::objectRemoved);
    connect(m, &MyMoneyFile::objectModified, this, &MyMoneyFileTest::objectModified);
    connect(m, &MyMoneyFile::balanceChanged, this, &MyMoneyFileTest::balanceChanged);
    connect(m, &MyMoneyFile::valueChanged, this, &MyMoneyFileTest::valueChanged);
}

// this method will be called before each testfunction
void MyMoneyFileTest::init()
{
    clearObjectLists();
}

// this method will be called after each testfunction
void MyMoneyFileTest::cleanup()
{
    m->unload();
}

void MyMoneyFileTest::testEmptyConstructor()
{
    MyMoneyPayee user = m->user();

    QCOMPARE(user.name(), QString());
    QCOMPARE(user.address(), QString());
    QCOMPARE(user.city(), QString());
    QCOMPARE(user.state(), QString());
    QCOMPARE(user.postcode(), QString());
    QCOMPARE(user.telephone(), QString());
    QCOMPARE(user.email(), QString());

    QCOMPARE(m->institutionCount(), static_cast<unsigned>(0));
    QCOMPARE(m->dirty(), false);
    QCOMPARE(m->accountsModel()->itemList().count(), 0);
}

void MyMoneyFileTest::testAddOneInstitution()
{
    MyMoneyInstitution institution;

    institution.setName("institution1");
    institution.setTown("town");
    institution.setStreet("street");
    institution.setPostcode("postcode");
    institution.setTelephone("telephone");
    institution.setManager("manager");
    institution.setBankCode("sortcode");

    // MyMoneyInstitution institution_file("", institution);
    MyMoneyInstitution institution_id("I000002", institution);
    MyMoneyInstitution institution_noname(institution);
    institution_noname.setName(QString());

    QCOMPARE(m->institutionCount(), static_cast<unsigned>(0));

    m->setDirty(false);

    clearObjectLists();
    MyMoneyFileTransaction ft;
    try {
        m->addInstitution(institution);
        ft.commit();
        QCOMPARE(institution.id(), QLatin1String("I000001"));
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
        QCOMPARE(m->dirty(), true);

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 1);
        QCOMPARE(m_objectsModified.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QCOMPARE(m_objectsAdded[0], QLatin1String("I000001"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    clearObjectLists();
    ft.restart();
    try {
        m->addInstitution(institution_id);
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
        ft.commit();
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
    }

    ft.restart();
    try {
        m->addInstitution(institution_noname);
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
        ft.commit();
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
    }
    QCOMPARE(m_objectsRemoved.count(), 0);
    QCOMPARE(m_objectsAdded.count(), 0);
    QCOMPARE(m_objectsModified.count(), 0);
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 0);
}

void MyMoneyFileTest::testAddTwoInstitutions()
{
    testAddOneInstitution();
    MyMoneyInstitution institution;
    institution.setName("institution2");
    institution.setTown("town");
    institution.setStreet("street");
    institution.setPostcode("postcode");
    institution.setTelephone("telephone");
    institution.setManager("manager");
    institution.setBankCode("sortcode");

    m->setDirty(false);

    MyMoneyFileTransaction ft;
    try {
        m->addInstitution(institution);
        ft.commit();

        QCOMPARE(institution.id(), QLatin1String("I000002"));
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(2));
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->setDirty(false);

    try {
        institution = m->institution("I000001");
        QCOMPARE(institution.id(), QLatin1String("I000001"));
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(2));
        QCOMPARE(m->dirty(), false);

        institution = m->institution("I000002");
        QCOMPARE(institution.id(), QLatin1String("I000002"));
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(2));
        QCOMPARE(m->dirty(), false);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyFileTest::testRemoveInstitution()
{
    testAddTwoInstitutions();

    MyMoneyInstitution i;

    QCOMPARE(m->institutionCount(), static_cast<unsigned>(2));

    i = m->institution("I000001");
    QCOMPARE(i.id(), QLatin1String("I000001"));
    QCOMPARE(i.accountCount(), static_cast<unsigned>(0));

    clearObjectLists();

    m->setDirty(false);
    MyMoneyFileTransaction ft;
    try {
        m->removeInstitution(i);
        QCOMPARE(m_objectsRemoved.count(), 0);
        ft.commit();
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m_objectsRemoved.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_objectsModified.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QCOMPARE(m_objectsRemoved[0], QLatin1String("I000001"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->setDirty(false);

    try {
        m->institution("I000001");
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
        QCOMPARE(m->dirty(), false);
    }

    clearObjectLists();
    ft.restart();
    try {
        m->removeInstitution(i);
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
        ft.commit();
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
        QCOMPARE(m->dirty(), false);
        QCOMPARE(m_objectsRemoved.count(), 0);
    }
}

void MyMoneyFileTest::testInstitutionRetrieval()
{

    testAddOneInstitution();

    m->setDirty(false);

    MyMoneyInstitution institution;

    QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));

    try {
        institution = m->institution("I000001");
        QCOMPARE(institution.id(), QLatin1String("I000001"));
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    try {
        institution = m->institution("I000002");
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(1));
    }

    QCOMPARE(m->dirty(), false);
}

void MyMoneyFileTest::testInstitutionListRetrieval()
{
    QList<MyMoneyInstitution> list;

    m->setDirty(false);
    list = m->institutionList();
    QCOMPARE(m->dirty(), false);
    QCOMPARE(list.count(), 0);

    testAddTwoInstitutions();

    m->setDirty(false);
    list = m->institutionList();
    QCOMPARE(m->dirty(), false);
    QCOMPARE(list.count(), 2);

    QList<MyMoneyInstitution>::ConstIterator it;
    it = list.constBegin();

    QVERIFY((*it).name() == "institution1" || (*it).name() == "institution2");
    ++it;
    QVERIFY((*it).name() == "institution2"  || (*it).name() == "institution1");
    ++it;
    QVERIFY(it == list.constEnd());
}

void MyMoneyFileTest::testInstitutionModify()
{
    testAddTwoInstitutions();
    MyMoneyInstitution institution;

    institution = m->institution("I000001");
    institution.setStreet("new street");
    institution.setTown("new town");
    institution.setPostcode("new postcode");
    institution.setTelephone("new telephone");
    institution.setManager("new manager");
    institution.setName("new name");
    institution.setBankCode("new sortcode");

    m->setDirty(false);

    clearObjectLists();
    MyMoneyFileTransaction ft;
    try {
        m->modifyInstitution(institution);
        ft.commit();
        QCOMPARE(institution.id(), QLatin1String("I000001"));
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(2));
        QCOMPARE(m->dirty(), true);

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QCOMPARE(m_objectsModified[0], QLatin1String("I000001"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    MyMoneyInstitution newInstitution;
    newInstitution = m->institution("I000001");

    QCOMPARE(newInstitution.id(), QLatin1String("I000001"));
    QCOMPARE(newInstitution.street(), QLatin1String("new street"));
    QCOMPARE(newInstitution.town(), QLatin1String("new town"));
    QCOMPARE(newInstitution.postcode(), QLatin1String("new postcode"));
    QCOMPARE(newInstitution.telephone(), QLatin1String("new telephone"));
    QCOMPARE(newInstitution.manager(), QLatin1String("new manager"));
    QCOMPARE(newInstitution.name(), QLatin1String("new name"));
    QCOMPARE(newInstitution.bankcode(), QLatin1String("new sortcode"));

    m->setDirty(false);

    ft.restart();
    MyMoneyInstitution failInstitution2("I000003", newInstitution);
    try {
        m->modifyInstitution(failInstitution2);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
        ft.commit();
        QCOMPARE(failInstitution2.id(), QLatin1String("I000003"));
        QCOMPARE(m->institutionCount(), static_cast<unsigned>(2));
        QCOMPARE(m->dirty(), false);
    }
}

void MyMoneyFileTest::testSetFunctions()
{
    MyMoneyPayee user = m->user();

    QCOMPARE(user.name(), QString());
    QCOMPARE(user.address(), QString());
    QCOMPARE(user.city(), QString());
    QCOMPARE(user.state(), QString());
    QCOMPARE(user.postcode(), QString());
    QCOMPARE(user.telephone(), QString());
    QCOMPARE(user.email(), QString());

    MyMoneyFileTransaction ft;
    m->setDirty(false);
    user.setName("Name");
    m->setUser(user);
    QCOMPARE(m->dirty(), true);
    m->setDirty(false);
    user.setAddress("Street");
    m->setUser(user);
    QCOMPARE(m->dirty(), true);
    m->setDirty(false);
    user.setCity("Town");
    m->setUser(user);
    QCOMPARE(m->dirty(), true);
    m->setDirty(false);
    user.setState("County");
    m->setUser(user);
    QCOMPARE(m->dirty(), true);
    m->setDirty(false);
    user.setPostcode("Postcode");
    m->setUser(user);
    QCOMPARE(m->dirty(), true);
    m->setDirty(false);
    user.setTelephone("Telephone");
    m->setUser(user);
    QCOMPARE(m->dirty(), true);
    m->setDirty(false);
    user.setEmail("Email");
    m->setUser(user);
    QCOMPARE(m->dirty(), true);
    m->setDirty(false);

    ft.commit();
    user = m->user();
    QCOMPARE(user.name(), QLatin1String("Name"));
    QCOMPARE(user.address(), QLatin1String("Street"));
    QCOMPARE(user.city(), QLatin1String("Town"));
    QCOMPARE(user.state(), QLatin1String("County"));
    QCOMPARE(user.postcode(), QLatin1String("Postcode"));
    QCOMPARE(user.telephone(), QLatin1String("Telephone"));
    QCOMPARE(user.email(), QLatin1String("Email"));
}

void MyMoneyFileTest::testAddAccounts()
{
    testAddTwoInstitutions();
    setupBaseCurrency();
    MyMoneyAccount  a, b, c;
    a.setAccountType(eMyMoney::Account::Type::Checkings);
    b.setAccountType(eMyMoney::Account::Type::Checkings);

    MyMoneyInstitution institution;

    m->setDirty(false);

    QCOMPARE(m->accountsModel()->itemList().count(), 0);

    institution = m->institution("I000001");
    QCOMPARE(institution.id(), QLatin1String("I000001"));

    a.setName("Account1");
    a.setInstitutionId(institution.id());
    a.setCurrencyId("EUR");

    clearObjectLists();
    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->asset();
        m->addAccount(a, parent);
        ft.commit();
        QCOMPARE(m->accountsModel()->itemList().count(), 1);
        QCOMPARE(a.parentAccountId(), QLatin1String("AStd::Asset"));
        QCOMPARE(a.id(), QLatin1String("A000001"));
        QCOMPARE(a.institutionId(), QLatin1String("I000001"));
        QCOMPARE(a.currencyId(), QLatin1String("EUR"));
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->asset().accountList().count(), 1);
        QCOMPARE(m->asset().accountList()[0], QLatin1String("A000001"));

        institution = m->institution("I000001");
        QCOMPARE(institution.accountCount(), static_cast<unsigned>(1));
        QCOMPARE(institution.accountList()[0], QLatin1String("A000001"));

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 1);
        QCOMPARE(m_objectsModified.count(), 2);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsAdded.contains(QLatin1String("A000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("I000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // try to add this account again, should not work
    ft.restart();
    try {
        MyMoneyAccount parent = m->asset();
        m->addAccount(a, parent);
        QFAIL("Expecting exception!");
    } catch (const MyMoneyException &) {
        ft.commit();
    }

    // check that we can modify the local object and
    // reload it from the file
    a.setName("AccountX");
    a = m->account("A000001");
    QCOMPARE(a.name(), QLatin1String("Account1"));

    m->setDirty(false);

    // check if we can get the same info to a different object
    c = m->account("A000001");
    QCOMPARE(c.accountType(), eMyMoney::Account::Type::Checkings);
    QCOMPARE(c.id(), QLatin1String("A000001"));
    QCOMPARE(c.name(), QLatin1String("Account1"));
    QCOMPARE(c.institutionId(), QLatin1String("I000001"));

    QCOMPARE(m->dirty(), false);

    // add a second account
    institution = m->institution("I000002");
    b.setName("Account2");
    b.setInstitutionId(institution.id());
    b.setCurrencyId("EUR");
    clearObjectLists();
    ft.restart();
    try {
        MyMoneyAccount parent = m->asset();
        m->addAccount(b, parent);
        ft.commit();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(b.id(), QLatin1String("A000002"));
        QCOMPARE(b.currencyId(), QLatin1String("EUR"));
        QCOMPARE(b.parentAccountId(), QLatin1String("AStd::Asset"));
        QCOMPARE(m->accountsModel()->itemList().count(), 2);

        institution = m->institution("I000001");
        QCOMPARE(institution.accountCount(), static_cast<unsigned>(1));
        QCOMPARE(institution.accountList()[0], QLatin1String("A000001"));

        institution = m->institution("I000002");
        QCOMPARE(institution.accountCount(), static_cast<unsigned>(1));
        QCOMPARE(institution.accountList()[0], QLatin1String("A000002"));

        QCOMPARE(m->asset().accountList().count(), 2);
        QCOMPARE(m->asset().accountList()[0], QLatin1String("A000001"));
        QCOMPARE(m->asset().accountList()[1], QLatin1String("A000002"));

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 1);
        QCOMPARE(m_objectsModified.count(), 2);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsAdded.contains(QLatin1String("A000002")));
        QVERIFY(m_objectsModified.contains(QLatin1String("I000002")));
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    MyMoneyAccount p;

    p = m->account("A000002");
    QCOMPARE(p.accountType(), eMyMoney::Account::Type::Checkings);
    QCOMPARE(p.id(), QLatin1String("A000002"));
    QCOMPARE(p.name(), QLatin1String("Account2"));
    QCOMPARE(p.institutionId(), QLatin1String("I000002"));
    QCOMPARE(p.currencyId(), QLatin1String("EUR"));
}

void MyMoneyFileTest::testAddCategories()
{
    setupBaseCurrency();

    MyMoneyAccount  a, b, c;
    a.setAccountType(eMyMoney::Account::Type::Income);
    a.setOpeningDate(QDate::currentDate());
    b.setAccountType(eMyMoney::Account::Type::Expense);

    m->setDirty(false);

    QCOMPARE(m->accountsModel()->itemList().count(), 0);
    QCOMPARE(a.openingDate(), QDate::currentDate());
    QVERIFY(!b.openingDate().isValid());

    a.setName("Account1");
    a.setCurrencyId("EUR");

    clearObjectLists();
    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->income();
        m->addAccount(a, parent);
        ft.commit();
        QCOMPARE(m->accountsModel()->itemList().count(), 1);
        QCOMPARE(a.parentAccountId(), QLatin1String("AStd::Income"));
        QCOMPARE(a.id(), QLatin1String("A000001"));
        QCOMPARE(a.institutionId(), QString());
        QCOMPARE(a.currencyId(), QLatin1String("EUR"));
        QCOMPARE(a.openingDate(), QDate(1900, 1, 1));
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->income().accountList().count(), 1);
        QCOMPARE(m->income().accountList()[0], QLatin1String("A000001"));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // add a second category, expense this time
    b.setName("Account2");
    b.setCurrencyId("EUR");
    clearObjectLists();
    ft.restart();
    try {
        MyMoneyAccount parent = m->expense();
        m->addAccount(b, parent);
        ft.commit();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(b.id(), QLatin1String("A000002"));
        QCOMPARE(a.institutionId(), QString());
        QCOMPARE(b.currencyId(), QLatin1String("EUR"));
        QCOMPARE(b.openingDate(), QDate(1900, 1, 1));
        QCOMPARE(b.parentAccountId(), QLatin1String("AStd::Expense"));
        QCOMPARE(m->accountsModel()->itemList().count(), 2);

        QCOMPARE(m->income().accountList().count(), 1);
        QCOMPARE(m->expense().accountList().count(), 1);
        QCOMPARE(m->income().accountList()[0], QLatin1String("A000001"));
        QCOMPARE(m->expense().accountList()[0], QLatin1String("A000002"));

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 1);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsAdded.contains(QLatin1String("A000002")));
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Expense")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testModifyAccount()
{
    testAddAccounts();
    m->setDirty(false);

    MyMoneyAccount p = m->account("A000001");
    MyMoneyInstitution institution;

    QCOMPARE(p.accountType(), eMyMoney::Account::Type::Checkings);
    QCOMPARE(p.name(), QLatin1String("Account1"));

    p.setName("New account name");
    MyMoneyFileTransaction ft;
    clearObjectLists();
    try {
        m->modifyAccount(p);
        ft.commit();

        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->accountsModel()->itemList().count(), 2);
        QCOMPARE(p.accountType(), eMyMoney::Account::Type::Checkings);
        QCOMPARE(p.name(), QLatin1String("New account name"));

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
    m->setDirty(false);

    // try to move account to new institution
    p.setInstitutionId("I000002");
    ft.restart();
    clearObjectLists();
    try {
        m->modifyAccount(p);
        ft.commit();

        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->accountsModel()->itemList().count(), 2);
        QCOMPARE(p.accountType(), eMyMoney::Account::Type::Checkings);
        QCOMPARE(p.name(), QLatin1String("New account name"));
        QCOMPARE(p.institutionId(), QLatin1String("I000002"));

        institution = m->institution("I000001");
        QCOMPARE(institution.accountCount(), static_cast<unsigned>(0));

        institution = m->institution("I000002");
        QCOMPARE(institution.accountCount(), static_cast<unsigned>(2));
        QCOMPARE(institution.accountList()[0], QLatin1String("A000002"));
        QCOMPARE(institution.accountList()[1], QLatin1String("A000001"));

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_objectsModified.count(), 3);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("I000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("I000002")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
    m->setDirty(false);

    // try to change to an account type that is allowed
    p.setAccountType(eMyMoney::Account::Type::Savings);
    ft.restart();
    try {
        m->modifyAccount(p);
        ft.commit();

        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->accountsModel()->itemList().count(), 2);
        QCOMPARE(p.accountType(), eMyMoney::Account::Type::Savings);
        QCOMPARE(p.name(), QLatin1String("New account name"));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
    m->setDirty(false);

    // try to change to an account type that is not allowed
    p.setAccountType(eMyMoney::Account::Type::CreditCard);
    ft.restart();
    try {
        m->modifyAccount(p);
        QFAIL("Expecting exception!");
    } catch (const MyMoneyException &) {
        ft.commit();
    }
    m->setDirty(false);

    // try to fool engine a bit
    p.setParentAccountId("A000001");
    ft.restart();
    try {
        m->modifyAccount(p);
        QFAIL("Expecting exception!");
    } catch (const MyMoneyException &) {
        ft.commit();
    }
}

void MyMoneyFileTest::testReparentAccount()
{
    testAddAccounts();
    m->setDirty(false);

    MyMoneyAccount p = m->account("A000001");
    MyMoneyAccount q = m->account("A000002");
    MyMoneyAccount o = m->account(p.parentAccountId());

    // make A000001 a child of A000002
    clearObjectLists();
    MyMoneyFileTransaction ft;
    try {
        QVERIFY(p.parentAccountId() != q.id());
        QCOMPARE(o.accountCount(), 2);
        QCOMPARE(q.accountCount(), 0);
        m->reparentAccount(p, q);
        ft.commit();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(p.parentAccountId(), q.id());
        QCOMPARE(q.accountCount(), 1);
        QCOMPARE(q.id(), QLatin1String("A000002"));
        QCOMPARE(p.id(), QLatin1String("A000001"));
        QCOMPARE(q.accountList()[0], p.id());

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_objectsModified.count(), 3);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("A000002")));
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

        o = m->account(o.id());
        QCOMPARE(o.accountCount(), 1);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testRemoveStdAccount(const MyMoneyAccount& acc)
{
    QString txt("Exception expected while removing account ");
    txt += acc.id();
    MyMoneyFileTransaction ft;
    try {
        m->removeAccount(acc);
        QFAIL(qPrintable(txt));
    } catch (const MyMoneyException &) {
        ft.commit();
    }
}

void MyMoneyFileTest::testRemoveAccount()
{
    MyMoneyInstitution institution;

    testAddAccounts();
    QCOMPARE(m->accountsModel()->itemList().count(), 2);
    m->setDirty(false);

    MyMoneyAccount p = m->account("A000001");

    clearObjectLists();

    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount q("Ainvalid", p);
        m->removeAccount(q);
        QFAIL("Exception expected!");
    } catch (const MyMoneyException &) {
        ft.commit();
    }

    ft.restart();
    try {
        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_objectsModified.count(), 0);

        m->removeAccount(p);
        ft.commit();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->accountsModel()->itemList().count(), 1);
        institution = m->institution("I000001");
        QCOMPARE(institution.accountCount(), static_cast<unsigned>(0));
        QCOMPARE(m->asset().accountList().count(), 1);

        QCOMPARE(m_objectsRemoved.count(), 1);
        QCOMPARE(m_objectsModified.count(), 2);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);

        QVERIFY(m_objectsRemoved.contains(QLatin1String("A000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("I000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

        institution = m->institution("I000002");
        QCOMPARE(institution.accountCount(), static_cast<unsigned>(1));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // Check that the standard account-groups cannot be removed
    testRemoveStdAccount(m->liability());
    testRemoveStdAccount(m->asset());
    testRemoveStdAccount(m->expense());
    testRemoveStdAccount(m->income());
}

void MyMoneyFileTest::testRemoveAccountTree()
{
    testReparentAccount();
    MyMoneyAccount a = m->account("A000002");

    clearObjectLists();
    MyMoneyFileTransaction ft;
    // remove the account
    try {
        m->removeAccount(a);
        ft.commit();

        QCOMPARE(m_objectsRemoved.count(), 1);
        QCOMPARE(m_objectsModified.count(), 5);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);

        QVERIFY(m_objectsRemoved.contains(QLatin1String("A000002")));
        QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));
        QVERIFY(m_objectsModified.contains(QLatin1String("I000002")));
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
    QCOMPARE(m->accountsModel()->itemList().count(), 1);

    // make sure it's gone
    try {
        m->account("A000002");
        QFAIL("Exception expected!");
    } catch (const MyMoneyException &) {
    }

    // make sure that children are re-parented to parent account
    try {
        a = m->account("A000001");
        QCOMPARE(a.parentAccountId(), m->asset().id());
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testAccountListRetrieval()
{
    QList<MyMoneyAccount> list;

    m->setDirty(false);
    m->accountList(list);
    QCOMPARE(m->dirty(), false);
    QCOMPARE(list.count(), 0);

    testAddAccounts();

    m->setDirty(false);
    list.clear();
    m->accountList(list);
    QCOMPARE(m->dirty(), false);
    QCOMPARE(list.count(), 2);

    QCOMPARE(list[0].accountType(), eMyMoney::Account::Type::Checkings);
    QCOMPARE(list[1].accountType(), eMyMoney::Account::Type::Checkings);
}

void MyMoneyFileTest::testAddTransaction()
{
    testAddAccounts();
    setupBaseCurrency();

    MyMoneyTransaction t, p;

    MyMoneyAccount exp1;
    exp1.setAccountType(eMyMoney::Account::Type::Expense);
    exp1.setName("Expense1");
    MyMoneyAccount exp2;
    exp2.setAccountType(eMyMoney::Account::Type::Expense);
    exp2.setName("Expense2");

    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->expense();
        m->addAccount(exp1, parent);
        m->addAccount(exp2, parent);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // fake the last modified flag to check that the
    // date is updated when we add the transaction
    MyMoneyAccount a = m->account("A000001");
    a.setLastModified(QDate(1, 2, 3));
    ft.restart();
    try {
        m->modifyAccount(a);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
    ft.restart();

    QCOMPARE(m->accountsModel()->itemList().count(), 4);
    a = m->account("A000001");
    QCOMPARE(a.lastModified(), QDate(1, 2, 3));

    // construct a transaction and add it to the pool
    t.setPostDate(QDate(2002, 2, 1));
    t.setMemo("Memotext");

    MyMoneySplit split1;
    MyMoneySplit split2;

    split1.setAccountId("A000001");
    split1.setShares(MyMoneyMoney(-1000, 100));
    split1.setValue(MyMoneyMoney(-1000, 100));
    split2.setAccountId("A000003");
    split2.setValue(MyMoneyMoney(1000, 100));
    split2.setShares(MyMoneyMoney(1000, 100));
    try {
        t.addSplit(split1);
        t.addSplit(split2);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    /*
            // FIXME: we don't have a payee and a number field right now
            // guess we should have a number field per split, don't know
            // about the payee
            t.setMethod(MyMoneyCheckingTransaction::Withdrawal);
            t.setPayee("Thomas Baumgart");
            t.setNumber("1234");
            t.setState(MyMoneyCheckingTransaction::Cleared);
    */
    m->setDirty(false);

    ft.restart();
    clearObjectLists();
    try {
        m->addTransaction(t);
        ft.commit();
        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 1);
        QCOMPARE(m_balanceChanged.count(), 2);
        QCOMPARE(m_balanceChanged.count("A000001"), 1);
        QCOMPARE(m_balanceChanged.count("A000003"), 1);
        QCOMPARE(m_valueChanged.count(), 0);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
    ft.restart();
    clearObjectLists();

    QCOMPARE(t.id(), QLatin1String("T000000000000000001"));
    QCOMPARE(t.postDate(), QDate(2002, 2, 1));
    QCOMPARE(t.entryDate(), QDate::currentDate());
    QCOMPARE(m->dirty(), true);

    // check the balance of the accounts
    a = m->account("A000001");
    QCOMPARE(a.lastModified(), QDate::currentDate());
    QCOMPARE(a.balance().toDouble(), MyMoneyMoney(-1000, 100).toDouble());

    MyMoneyAccount b = m->account("A000003");
    QCOMPARE(b.lastModified(), QDate::currentDate());
    QCOMPARE(b.balance(), MyMoneyMoney(1000, 100));

    m->setDirty(false);

    // locate transaction in MyMoneyFile via id

    try {
        p = m->transaction("T000000000000000001");
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QCOMPARE(p.splitCount(), static_cast<unsigned>(2));
        QCOMPARE(p.memo(), QLatin1String("Memotext"));
        QCOMPARE(p.splits()[0].accountId(), QLatin1String("A000001"));
        QCOMPARE(p.splits()[1].accountId(), QLatin1String("A000003"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // check if it's in the account(s) as well

    try {
        p = m->transaction("A000001", 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QCOMPARE(p.id(), QLatin1String("T000000000000000001"));
        QCOMPARE(p.splitCount(), static_cast<unsigned>(2));
        QCOMPARE(p.memo(), QLatin1String("Memotext"));
        QCOMPARE(p.splits()[0].accountId(), QLatin1String("A000001"));
        QCOMPARE(p.splits()[1].accountId(), QLatin1String("A000003"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    try {
        p = m->transaction("A000003", 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QCOMPARE(p.id(), QLatin1String("T000000000000000001"));
        QCOMPARE(p.splitCount(), static_cast<unsigned>(2));
        QCOMPARE(p.memo(), QLatin1String("Memotext"));
        QCOMPARE(p.splits()[0].accountId(), QLatin1String("A000001"));
        QCOMPARE(p.splits()[1].accountId(), QLatin1String("A000003"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testIsStandardAccount()
{
    QCOMPARE(m->isStandardAccount(m->liability().id()), true);
    QCOMPARE(m->isStandardAccount(m->asset().id()), true);
    QCOMPARE(m->isStandardAccount(m->expense().id()), true);
    QCOMPARE(m->isStandardAccount(m->income().id()), true);
    QCOMPARE(m->isStandardAccount("A00001"), false);
}

void MyMoneyFileTest::testHasActiveSplits()
{
    testAddTransaction();

    QCOMPARE(m->hasActiveSplits("A000001"), true);
    QCOMPARE(m->hasActiveSplits("A000002"), false);
}

void MyMoneyFileTest::testModifyTransactionSimple()
{
    // this will test that we can modify the basic attributes
    // of a transaction
    testAddTransaction();

    MyMoneyTransaction t = m->transaction("T000000000000000001");
    t.setMemo("New Memotext");
    m->setDirty(false);

    MyMoneyFileTransaction ft;
    clearObjectLists();
    try {
        m->modifyTransaction(t);
        ft.commit();
        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 2);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000001")), 1);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000003")), 1);
        QCOMPARE(m_valueChanged.count(), 0);
        t = m->transaction("T000000000000000001");
        QCOMPARE(t.memo(), QLatin1String("New Memotext"));
        QCOMPARE(m->dirty(), true);

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testModifyTransactionNewPostDate()
{
    // this will test that we can modify the basic attributes
    // of a transaction
    testAddTransaction();

    MyMoneyTransaction t = m->transaction("T000000000000000001");
    t.setPostDate(QDate(2004, 2, 1));
    m->setDirty(false);

    MyMoneyFileTransaction ft;
    clearObjectLists();
    try {
        m->modifyTransaction(t);
        ft.commit();
        t = m->transaction("T000000000000000001");
        QCOMPARE(t.postDate(), QDate(2004, 2, 1));
        t = m->transaction("A000001", 0);
        QCOMPARE(t.id(), QLatin1String("T000000000000000001"));
        QCOMPARE(m->dirty(), true);

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 2);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000001")), 1);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000003")), 1);
        QCOMPARE(m_valueChanged.count(), 0);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testModifyTransactionNewAccount()
{
    // this will test that we can modify the basic attributes
    // of a transaction
    testAddTransaction();

    MyMoneyTransaction t = m->transaction("T000000000000000001");
    MyMoneySplit s;
    s = t.splits()[0];
    s.setAccountId("A000002");
    t.modifySplit(s);

    m->setDirty(false);
    QList<MyMoneyTransaction> list;
    MyMoneyFileTransaction ft;
    clearObjectLists();
    try {
        MyMoneyTransactionFilter f1("A000001");
        MyMoneyTransactionFilter f2("A000002");
        MyMoneyTransactionFilter f3("A000003");
        m->transactionList(list, f1);
        QCOMPARE(list.count(), 1);
        m->transactionList(list, f2);
        QCOMPARE(list.count(), 0);
        m->transactionList(list, f3);
        QCOMPARE(list.count(), 1);

        m->modifyTransaction(t);
        ft.commit();
        t = m->transaction("T000000000000000001");
        QCOMPARE(t.postDate(), QDate(2002, 2, 1));
        t = m->transaction("A000002", 0);
        QCOMPARE(m->dirty(), true);
        m->transactionList(list, f1);
        QCOMPARE(list.count(), 0);
        m->transactionList(list, f2);
        QCOMPARE(list.count(), 1);
        m->transactionList(list, f3);
        QCOMPARE(list.count(), 1);

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 3);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000001")), 1);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000002")), 1);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000003")), 1);
        QCOMPARE(m_valueChanged.count(), 0);


    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testRemoveTransaction()
{
    testModifyTransactionNewPostDate();

    MyMoneyTransaction t;
    t = m->transaction("T000000000000000001");

    m->setDirty(false);
    MyMoneyFileTransaction ft;
    QList<MyMoneyTransaction> list;
    clearObjectLists();
    try {
        m->removeTransaction(t);
        ft.commit();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->transactionCount(), static_cast<unsigned>(0));
        MyMoneyTransactionFilter f1("A000001");
        MyMoneyTransactionFilter f2("A000002");
        MyMoneyTransactionFilter f3("A000003");
        m->transactionList(list, f1);
        QCOMPARE(list.count(), 0);
        m->transactionList(list, f2);
        QCOMPARE(list.count(), 0);
        m->transactionList(list, f3);
        QCOMPARE(list.count(), 0);

        QCOMPARE(m_objectsRemoved.count(), 1);
        QCOMPARE(m_objectsModified.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 2);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000001")), 1);
        QCOMPARE(m_balanceChanged.count(QLatin1String("A000003")), 1);
        QCOMPARE(m_valueChanged.count(), 0);

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

/*
 * This function is currently not implemented. It's kind of tricky
 * because it modifies a lot of objects in a single call. This might
 * be a problem for the undo/redo stuff. That's why I left it out in
 * the first run. We migh add it, if we need it.
 * /
void testMoveSplits() {
        testModifyTransactionNewPostDate();

        QCOMPARE(m->account("A000001").transactionCount(), 1);
        QCOMPARE(m->account("A000002").transactionCount(), 0);
        QCOMPARE(m->account("A000003").transactionCount(), 1);

        try {
                m->moveSplits("A000001", "A000002");
                QCOMPARE(m->account("A000001").transactionCount(), 0);
                QCOMPARE(m->account("A000002").transactionCount(), 1);
                QCOMPARE(m->account("A000003").transactionCount(), 1);
        } catch (const MyMoneyException &e) {
                QFAIL("Unexpected exception!");
        }
}
*/

void MyMoneyFileTest::testBalanceTotal()
{
    testAddTransaction();
    MyMoneyTransaction t;

    // construct a transaction and add it to the pool
    t.setPostDate(QDate(2002, 2, 1));
    t.setMemo("Memotext");

    MyMoneySplit split1;
    MyMoneySplit split2;

    MyMoneyFileTransaction ft;
    try {
        split1.setAccountId("A000002");
        split1.setShares(MyMoneyMoney(-1000, 100));
        split1.setValue(MyMoneyMoney(-1000, 100));
        split2.setAccountId("A000004");
        split2.setValue(MyMoneyMoney(1000, 100));
        split2.setShares(MyMoneyMoney(1000, 100));
        t.addSplit(split1);
        t.addSplit(split2);
        m->addTransaction(t);
        ft.commit();
        ft.restart();
        QCOMPARE(t.id(), QLatin1String("T000000000000000002"));
        QCOMPARE(m->totalBalance("A000001"), MyMoneyMoney(-1000, 100));
        QCOMPARE(m->totalBalance("A000002"), MyMoneyMoney(-1000, 100));

        MyMoneyAccount p = m->account("A000001");
        MyMoneyAccount q = m->account("A000002");
        m->reparentAccount(p, q);
        ft.commit();
        // check totalBalance() and balance() with combinations of parameters
        QCOMPARE(m->totalBalance("A000001"), MyMoneyMoney(-1000, 100));
        QCOMPARE(m->totalBalance("A000002"), MyMoneyMoney(-2000, 100));
        QVERIFY(m->totalBalance("A000002", QDate(2002, 1, 15)).isZero());

        QCOMPARE(m->balance("A000001"), MyMoneyMoney(-1000, 100));
        QCOMPARE(m->balance("A000002"), MyMoneyMoney(-1000, 100));
        // Date of a transaction
        QCOMPARE(m->balance("A000001", QDate(2002, 2, 1)), MyMoneyMoney(-1000, 100));
        QCOMPARE(m->balance("A000002", QDate(2002, 2, 1)), MyMoneyMoney(-1000, 100));
        // Date after last transaction
        QCOMPARE(m->balance("A000001", QDate(2002, 2, 1)), MyMoneyMoney(-1000, 100));
        QCOMPARE(m->balance("A000002", QDate(2002, 2, 1)), MyMoneyMoney(-1000, 100));
        // Date before first transaction
        QVERIFY(m->balance("A000001", QDate(2002, 1, 15)).isZero());
        QVERIFY(m->balance("A000002", QDate(2002, 1, 15)).isZero());

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // Now check for exceptions
    try {
        // Account not found for balance()
        QVERIFY(m->balance("A000005").isZero());
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    try {
        // Account not found for totalBalance()
        QVERIFY(m->totalBalance("A000005").isZero());
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

}

/// @todo cleanup
#if 0
void MyMoneyFileTest::testSetAccountName()
{
    MyMoneyFileTransaction ft;
    clearObjectLists();
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Liability), "Verbindlichkeiten");
        ft.commit();
        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Liability")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
    ft.restart();
    clearObjectLists();
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset), QString::fromUtf8("Vermögen"));
        ft.commit();
        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
    ft.restart();
    clearObjectLists();
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense), "Ausgaben");
        ft.commit();
        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Expense")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
    ft.restart();
    clearObjectLists();
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Income), "Einnahmen");
        ft.commit();
        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Income")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
    ft.restart();

    QCOMPARE(m->liability().name(), QLatin1String("Verbindlichkeiten"));
    QCOMPARE(m->asset().name(), QString::fromUtf8("Vermögen"));
    QCOMPARE(m->expense().name(), QLatin1String("Ausgaben"));
    QCOMPARE(m->income().name(), QLatin1String("Einnahmen"));

    try {
        m->setAccountName("A000001", "New account name");
        ft.commit();
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}
#endif

void MyMoneyFileTest::testAddPayee()
{
    MyMoneyPayee p;

    p.setName("THB");
    QCOMPARE(m->dirty(), false);
    MyMoneyFileTransaction ft;
    try {
        m->addPayee(p);
        ft.commit();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(p.id(), QLatin1String("P000001"));

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 1);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);

        QVERIFY(m_objectsAdded.contains(QLatin1String("P000001")));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyFileTest::testModifyPayee()
{
    MyMoneyPayee p;

    testAddPayee();
    clearObjectLists();

    p = m->payee("P000001");
    p.setName("New name");
    MyMoneyFileTransaction ft;
    try {
        m->modifyPayee(p);
        ft.commit();
        p = m->payee("P000001");
        QCOMPARE(p.name(), QLatin1String("New name"));

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);

        QVERIFY(m_objectsModified.contains(QLatin1String("P000001")));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyFileTest::testRemovePayee()
{
    MyMoneyPayee p;

    testAddPayee();
    clearObjectLists();
    QCOMPARE(m->payeeList().count(), 1);

    p = m->payee("P000001");
    MyMoneyFileTransaction ft;
    try {
        m->removePayee(p);
        ft.commit();
        QCOMPARE(m->payeeList().count(), 0);

        QCOMPARE(m_objectsRemoved.count(), 1);
        QCOMPARE(m_objectsModified.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 0);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);

        QVERIFY(m_objectsRemoved.contains(QLatin1String("P000001")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyFileTest::testPayeeWithIdentifier()
{
    MyMoneyPayee p;
    try {
        MyMoneyFileTransaction ft;
        m->addPayee(p);
        ft.commit();

        p = m->payee(p.id());

        payeeIdentifier ident = payeeIdentifier(new payeeIdentifiers::ibanBic());
        payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban(ident);
        iban->setIban(QLatin1String("DE82 2007 0024 0066 6446 00"));

        ft.restart();
        p.addPayeeIdentifier(iban);
        m->modifyPayee(p);
        ft.commit();

        p = m->payee(p.id());
        QCOMPARE(p.payeeIdentifiers().count(), 1);

        ident = p.payeeIdentifiers().first();
        try {
            iban = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(ident);
        } catch (...) {
            QFAIL("Unexpected exception");
        }
        QCOMPARE(iban->electronicIban(), QLatin1String("DE82200700240066644600"));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testAddTransactionStd()
{
    testAddAccounts();
    MyMoneyTransaction t, p;
    MyMoneyAccount a;

    a = m->account("A000001");

    // construct a transaction and add it to the pool
    t.setPostDate(QDate(2002, 2, 1));
    t.setMemo("Memotext");

    MyMoneySplit split1;
    MyMoneySplit split2;

    split1.setAccountId("A000001");
    split1.setShares(MyMoneyMoney(-1000, 100));
    split1.setValue(MyMoneyMoney(-1000, 100));
    split2.setAccountId(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense));
    split2.setValue(MyMoneyMoney(1000, 100));
    split2.setShares(MyMoneyMoney(1000, 100));
    try {
        t.addSplit(split1);
        t.addSplit(split2);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    /*
            // FIXME: we don't have a payee and a number field right now
            // guess we should have a number field per split, don't know
            // about the payee
            t.setMethod(MyMoneyCheckingTransaction::Withdrawal);
            t.setPayee("Thomas Baumgart");
            t.setNumber("1234");
            t.setState(MyMoneyCheckingTransaction::Cleared);
    */
    m->setDirty(false);

    MyMoneyFileTransaction ft;
    try {
        m->addTransaction(t);
        ft.commit();
        QFAIL("Missing expected exception!");
    } catch (const MyMoneyException &) {
    }

    QCOMPARE(m->dirty(), false);
}


void MyMoneyFileTest::testAccount2Category()
{
    testReparentAccount();
    QCOMPARE(m->accountToCategory("A000001"), QLatin1String("Account2:Account1"));
    QCOMPARE(m->accountToCategory("A000002"), QLatin1String("Account2"));
}

void MyMoneyFileTest::testCategory2Account()
{
    testAddTransaction();
    MyMoneyAccount a = m->account("A000003");
    MyMoneyAccount b = m->account("A000004");

    MyMoneyFileTransaction ft;
    try {
        m->reparentAccount(b, a);
        ft.commit();
        QCOMPARE(m->categoryToAccount("Expense1"), QLatin1String("A000003"));
        QCOMPARE(m->categoryToAccount("Expense1:Expense2"), QLatin1String("A000004"));
        QVERIFY(m->categoryToAccount("Acc2").isEmpty());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testHasAccount()
{
    testAddAccounts();

    MyMoneyAccount a, b;
    a.setAccountType(eMyMoney::Account::Type::Checkings);
    a.setName("Account3");
    b = m->account("A000001");
    MyMoneyFileTransaction ft;
    try {
        m->addAccount(a, b);
        ft.commit();
        QCOMPARE(m->accountsModel()->itemList().count(), 3);
        QCOMPARE(a.parentAccountId(), QLatin1String("A000001"));
        QCOMPARE(m->hasAccount("A000001", "Account3"), true);
        QCOMPARE(m->hasAccount("A000001", "Account2"), false);
        QCOMPARE(m->hasAccount("A000002", "Account3"), false);
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testAddEquityAccount()
{
    MyMoneyAccount i;
    i.setName("Investment");
    i.setAccountType(eMyMoney::Account::Type::Investment);

    setupBaseCurrency();

    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->asset();
        m->addAccount(i, parent);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
    // keep a copy for later use
    m_inv = i;

    // make sure, that only equity accounts can be children to it
    MyMoneyAccount a;
    a.setName("Testaccount");
    QList<eMyMoney::Account::Type> list;
    list << eMyMoney::Account::Type::Checkings;
    list << eMyMoney::Account::Type::Savings;
    list << eMyMoney::Account::Type::Cash;
    list << eMyMoney::Account::Type::CreditCard;
    list << eMyMoney::Account::Type::Loan;
    list << eMyMoney::Account::Type::CertificateDep;
    list << eMyMoney::Account::Type::Investment;
    list << eMyMoney::Account::Type::MoneyMarket;
    list << eMyMoney::Account::Type::Asset;
    list << eMyMoney::Account::Type::Liability;
    list << eMyMoney::Account::Type::Currency;
    list << eMyMoney::Account::Type::Income;
    list << eMyMoney::Account::Type::Expense;
    list << eMyMoney::Account::Type::AssetLoan;

    QList<eMyMoney::Account::Type>::Iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        a.setAccountType(*it);
        ft.restart();
        try {
            char    msg[100];
            m->addAccount(a, i);
            sprintf(msg, "Can add non-equity type %d to investment", (int)*it);
            QFAIL(msg);
        } catch (const MyMoneyException &) {
            ft.commit();
        }
    }
    ft.restart();
    try {
        a.setName("Teststock");
        a.setAccountType(eMyMoney::Account::Type::Stock);
        m->addAccount(a, i);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testReparentEquity()
{
    testAddEquityAccount();
    testAddEquityAccount();
    MyMoneyAccount parent;

    // check the bad cases
    QList<eMyMoney::Account::Type> list;
    list << eMyMoney::Account::Type::Checkings;
    list << eMyMoney::Account::Type::Savings;
    list << eMyMoney::Account::Type::Cash;
    list << eMyMoney::Account::Type::CertificateDep;
    list << eMyMoney::Account::Type::MoneyMarket;
    list << eMyMoney::Account::Type::Asset;
    list << eMyMoney::Account::Type::AssetLoan;
    list << eMyMoney::Account::Type::Currency;
    parent = m->asset();
    testReparentEquity(list, parent);

    list.clear();
    list << eMyMoney::Account::Type::CreditCard;
    list << eMyMoney::Account::Type::Loan;
    list << eMyMoney::Account::Type::Liability;
    parent = m->liability();
    testReparentEquity(list, parent);

    list.clear();
    list << eMyMoney::Account::Type::Income;
    parent = m->income();
    testReparentEquity(list, parent);

    list.clear();
    list << eMyMoney::Account::Type::Expense;
    parent = m->expense();
    testReparentEquity(list, parent);

    // now check the good case
    MyMoneyAccount stock = m->account("A000002");
    MyMoneyAccount inv = m->account(m_inv.id());
    MyMoneyFileTransaction ft;
    try {
        m->reparentAccount(stock, inv);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testReparentEquity(QList<eMyMoney::Account::Type>& list, MyMoneyAccount& parent)
{
    MyMoneyAccount a;
    MyMoneyAccount stock = m->account("A000002");

    QList<eMyMoney::Account::Type>::Iterator it;
    MyMoneyFileTransaction ft;
    for (it = list.begin(); it != list.end(); ++it) {
        a.setName(QString("Testaccount %1").arg((int)*it));
        a.setAccountType(*it);
        try {
            m->addAccount(a, parent);
            char    msg[100];
            m->reparentAccount(stock, a);
            sprintf(msg, "Can reparent stock to non-investment type %d account", (int)*it);
            QFAIL(msg);
        } catch (const MyMoneyException &) {
            ft.commit();
        }
        ft.restart();
    }
}

void MyMoneyFileTest::testBaseCurrency()
{
    MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
    MyMoneySecurity ref;

    // make sure, no base currency is set
    try {
        ref = m->baseCurrency();
        QVERIFY(ref.id().isEmpty());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    // make sure, we cannot assign an unknown currency
    try {
        m->setBaseCurrency(base);
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
    }

    MyMoneyFileTransaction ft;
    // add the currency and try again
    try {
        m->addCurrency(base);
        m->setBaseCurrency(base);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
    ft.restart();

    // make sure, the base currency is set
    try {
        ref = m->baseCurrency();
        QCOMPARE(ref.id(), QLatin1String("EUR"));
        QCOMPARE(ref.name(), QLatin1String("Euro"));
        QVERIFY(ref.tradingSymbol() == QChar(0x20ac));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

/// @todo "Cleanup dead code"
#if 0 // invalid test with new model based backend
    // check if it gets reset when attaching a new storage
    m->detachStorage(storage);

    MyMoneyStorageMgr* newStorage = new MyMoneyStorageMgr;
    m->attachStorage(newStorage);

    ref = m->baseCurrency();
    QVERIFY(ref.id().isEmpty());

    m->detachStorage(newStorage);
    delete newStorage;

    m->attachStorage(storage);
    ref = m->baseCurrency();
    QCOMPARE(ref.id(), QLatin1String("EUR"));
    QCOMPARE(ref.name(), QLatin1String("Euro"));
    QVERIFY(ref.tradingSymbol() == QChar(0x20ac));
#endif
}

void MyMoneyFileTest::testOpeningBalanceNoBase()
{
    MyMoneyAccount openingAcc;
    MyMoneySecurity base;

    try {
        base = m->baseCurrency();
        openingAcc = m->openingBalanceAccount(base);
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyFileTest::testOpeningBalance()
{
    MyMoneyAccount openingAcc;
    MyMoneySecurity second("USD", "US Dollar", "$");
    setupBaseCurrency();

    try {
        openingAcc = m->openingBalanceAccount(m->baseCurrency());
        QCOMPARE(openingAcc.parentAccountId(), m->equity().id());
        QCOMPARE(openingAcc.name(), MyMoneyFile::openingBalancesPrefix());
        QCOMPARE(openingAcc.openingDate(), QDate::currentDate());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    // add a second currency
    MyMoneyFileTransaction ft;
    try {
        m->addCurrency(second);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    QString refName = QString("%1 (%2)").arg(MyMoneyFile::openingBalancesPrefix()).arg("USD");
    try {
        openingAcc = m->openingBalanceAccount(second);
        QCOMPARE(openingAcc.parentAccountId(), m->equity().id());
        QCOMPARE(openingAcc.name(), refName);
        QCOMPARE(openingAcc.openingDate(), QDate::currentDate());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testModifyStdAccount()
{
    QVERIFY(m->asset().currencyId().isEmpty());
    QCOMPARE(m->asset().name(), QLatin1String("Asset accounts"));
    setupBaseCurrency();
    // testBaseCurrency();
    QVERIFY(m->asset().currencyId().isEmpty());
    QVERIFY(!m->baseCurrency().id().isEmpty());

    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount acc = m->asset();
        acc.setName("Anlagen");
        acc.setCurrencyId(m->baseCurrency().id());
        m->modifyAccount(acc);
        ft.commit();

        QCOMPARE(m->asset().name(), QLatin1String("Anlagen"));
        QCOMPARE(m->asset().currencyId(), m->baseCurrency().id());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    ft.restart();
    try {
        MyMoneyAccount acc = m->asset();
        acc.setNumber("Test");
        m->modifyAccount(acc);
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
        ft.rollback();
    }

}

void MyMoneyFileTest::testAddPrice()
{
    testAddAccounts();
    setupBaseCurrency();
    MyMoneyAccount p;

    MyMoneyFileTransaction ft;
    try {
        p = m->account("A000002");
        p.setCurrencyId("RON");
        m->modifyAccount(p);
        ft.commit();

        QCOMPARE(m->account("A000002").currencyId(), QLatin1String("RON"));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    clearObjectLists();
    ft.restart();
    MyMoneyPrice price("EUR", "RON", QDate::currentDate(), MyMoneyMoney(4.1), "Test source");
    m->addPrice(price);
    ft.commit();
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 1);
    QCOMPARE(m_valueChanged.count("A000002"), 1);

    clearObjectLists();
    ft.restart();
    MyMoneyPrice priceReciprocal("RON", "EUR", QDate::currentDate(), MyMoneyMoney(1 / 4.1), "Test source reciprocal price");
    m->addPrice(priceReciprocal);
    ft.commit();
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 1);
    QCOMPARE(m_valueChanged.count("A000002"), 1);
}

void MyMoneyFileTest::testRemovePrice()
{
    testAddPrice();
    clearObjectLists();
    MyMoneyFileTransaction ft;
    MyMoneyPrice price("EUR", "RON", QDate::currentDate(), MyMoneyMoney(4.1), "Test source");
    m->removePrice(price);
    ft.commit();
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 1);
    QCOMPARE(m_valueChanged.count("A000002"), 1);
}

void MyMoneyFileTest::testGetPrice()
{
    testAddPrice();
    // the price for the current date is found
    QVERIFY(m->price("EUR", "RON", QDate::currentDate()).isValid());
    // the price for the current date is returned when asking for the next day with exact date set to false
    {
        const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(1), false);
        QVERIFY(price.isValid() && price.date() == QDate::currentDate());
    }
    // no price is returned while asking for the next day with exact date set to true
    QVERIFY(!m->price("EUR", "RON", QDate::currentDate().addDays(1), true).isValid());

    // no price is returned while asking for the previous day with exact date set to true/false because all prices are newer
    QVERIFY(!m->price("EUR", "RON", QDate::currentDate().addDays(-1), false).isValid());
    QVERIFY(!m->price("EUR", "RON", QDate::currentDate().addDays(-1), true).isValid());

    // add two more prices
    MyMoneyFileTransaction ft;
    m->addPrice(MyMoneyPrice("EUR", "RON", QDate::currentDate().addDays(3), MyMoneyMoney(4.1), "Test source"));
    m->addPrice(MyMoneyPrice("EUR", "RON", QDate::currentDate().addDays(5), MyMoneyMoney(4.1), "Test source"));
    ft.commit();
    clearObjectLists();

    // extra tests for the exactDate=false behavior
    {
        const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(2), false);
        QVERIFY(price.isValid() && price.date() == QDate::currentDate());
    }
    {
        const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(3), false);
        QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(3));
    }
    {
        const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(4), false);
        QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(3));
    }
    {
        const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(5), false);
        QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(5));
    }
    {
        const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(6), false);
        QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(5));
    }
}

void MyMoneyFileTest::testAddAccountMissingCurrency()
{
    testAddTwoInstitutions();
    MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
    MyMoneyAccount  a;
    a.setAccountType(eMyMoney::Account::Type::Checkings);

    MyMoneyInstitution institution;

    m->setDirty(false);

    QCOMPARE(m->accountsModel()->itemList().count(), 0);

    institution = m->institution("I000001");
    QCOMPARE(institution.id(), QLatin1String("I000001"));

    a.setName("Account1");
    a.setInstitutionId(institution.id());

    clearObjectLists();
    MyMoneyFileTransaction ft;
    try {
        m->addCurrency(base);
        m->setBaseCurrency(base);
        MyMoneyAccount parent = m->asset();
        m->addAccount(a, parent);
        ft.commit();
        QCOMPARE(m->account("A000001").currencyId(), QLatin1String("EUR"));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testAddTransactionToClosedAccount()
{
    QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyFileTest::testRemoveTransactionFromClosedAccount()
{
    QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyFileTest::testModifyTransactionInClosedAccount()
{
    QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyFileTest::testStorageId()
{
    // make sure id will be setup if it does not exist
    MyMoneyFileTransaction ft;
    try {
        m->setValue("kmm-id", "");
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    try {
        // check for a new id
        auto id = m->storageId();
        QVERIFY(!id.isNull());
        // check that it is the same if we ask again
        QCOMPARE(id, m->storageId());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyFileTest::testHasMatchingOnlineBalance_emptyAccountWithoutImportedBalance()
{
    addOneAccount();

    MyMoneyAccount a = m->account("A000001");

    QCOMPARE(m->hasMatchingOnlineBalance(a), false);
}

void MyMoneyFileTest::testHasMatchingOnlineBalance_emptyAccountWithEqualImportedBalance()
{
    addOneAccount();

    MyMoneyAccount a = m->account("A000001");

    a.setValue("lastImportedTransactionDate", QDate(2011, 12, 1).toString(Qt::ISODate));
    a.setValue("lastStatementBalance", MyMoneyMoney().toString());

    MyMoneyFileTransaction ft;
    m->modifyAccount(a);
    ft.commit();

    QCOMPARE(m->hasMatchingOnlineBalance(a), true);
}

void MyMoneyFileTest::testHasMatchingOnlineBalance_emptyAccountWithUnequalImportedBalance()
{
    addOneAccount();

    MyMoneyAccount a = m->account("A000001");

    a.setValue("lastImportedTransactionDate", QDate(2011, 12, 1).toString(Qt::ISODate));
    a.setValue("lastStatementBalance", MyMoneyMoney::ONE.toString());

    MyMoneyFileTransaction ft;
    m->modifyAccount(a);
    ft.commit();

    QCOMPARE(m->hasMatchingOnlineBalance(a), false);
}

void MyMoneyFileTest::testHasNewerTransaction_withoutAnyTransaction_afterLastImportedTransaction()
{
    addOneAccount();

    MyMoneyAccount a = m->account("A000001");

    QDate dateOfLastTransactionImport(2011, 12, 1);

    // There are no transactions at all:
    QCOMPARE(m->hasNewerTransaction(a.id(), dateOfLastTransactionImport), false);
}

void MyMoneyFileTest::testHasNewerTransaction_withoutNewerTransaction_afterLastImportedTransaction()
{

    addOneAccount();
    setupBaseCurrency();

    QString accId("A000001");
    QDate dateOfLastTransactionImport(2011, 12, 1);

    MyMoneyFileTransaction ft;
    MyMoneyTransaction t;

    // construct a transaction at the day of the last transaction import and add it to the pool
    t.setPostDate(dateOfLastTransactionImport);

    MyMoneySplit split1;

    split1.setAccountId(accId);
    split1.setShares(MyMoneyMoney(-1000, 100));
    split1.setValue(MyMoneyMoney(-1000, 100));
    t.addSplit(split1);

    ft.restart();
    m->addTransaction(t);
    ft.commit();

    QCOMPARE(m->hasNewerTransaction(accId, dateOfLastTransactionImport), false);
}

void MyMoneyFileTest::testHasNewerTransaction_withNewerTransaction_afterLastImportedTransaction()
{

    addOneAccount();
    setupBaseCurrency();

    QString accId("A000001");
    QDate dateOfLastTransactionImport(2011, 12, 1);
    QDate dateOfDayAfterLastTransactionImport(dateOfLastTransactionImport.addDays(1));

    MyMoneyFileTransaction ft;
    MyMoneyTransaction t;

    // construct a transaction a day after the last transaction import and add it to the pool
    t.setPostDate(dateOfDayAfterLastTransactionImport);

    MyMoneySplit split1;

    split1.setAccountId(accId);
    split1.setShares(MyMoneyMoney(-1000, 100));
    split1.setValue(MyMoneyMoney(-1000, 100));
    t.addSplit(split1);

    ft.restart();
    m->addTransaction(t);
    ft.commit();

    QCOMPARE(m->hasNewerTransaction(accId, dateOfLastTransactionImport), true);
}

void MyMoneyFileTest::addOneAccount()
{
    setupBaseCurrency();
    QString accountId = "A000001";
    MyMoneyAccount  a;
    a.setAccountType(eMyMoney::Account::Type::Checkings);

    m->setDirty(false);

    QCOMPARE(m->accountsModel()->itemList().count(), 0);

    a.setName("Account1");
    a.setCurrencyId("EUR");

    clearObjectLists();
    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->asset();
        m->addAccount(a, parent);
        ft.commit();
        QCOMPARE(m->accountsModel()->itemList().count(), 1);
        QCOMPARE(a.parentAccountId(), QLatin1String("AStd::Asset"));
        QCOMPARE(a.id(), accountId);
        QCOMPARE(a.currencyId(), QLatin1String("EUR"));
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->asset().accountList().count(), 1);
        QCOMPARE(m->asset().accountList()[0], accountId);

        QCOMPARE(m_objectsRemoved.count(), 0);
        QCOMPARE(m_objectsAdded.count(), 1);
        QCOMPARE(m_objectsModified.count(), 1);
        QCOMPARE(m_balanceChanged.count(), 0);
        QCOMPARE(m_valueChanged.count(), 0);
        QVERIFY(m_objectsAdded.contains(accountId.toLatin1()));
        QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testCountTransactionsWithSpecificReconciliationState_noTransactions()
{
    addOneAccount();
    QString accountId = "A000001";

    QCOMPARE(m->countTransactionsWithSpecificReconciliationState(accountId, eMyMoney::TransactionFilter::State::NotReconciled), 0);
}

void MyMoneyFileTest::testCountTransactionsWithSpecificReconciliationState_transactionWithWantedReconcileState()
{
    addOneAccount();
    setupBaseCurrency();

    QString accountId = "A000001";

    // construct split & transaction
    MyMoneySplit split;
    split.setAccountId(accountId);
    split.setShares(MyMoneyMoney(-1000, 100));
    split.setValue(MyMoneyMoney(-1000, 100));

    MyMoneyTransaction transaction;
    transaction.setPostDate(QDate(2013, 1, 1));
    transaction.addSplit(split);

    // add transaction
    MyMoneyFileTransaction ft;
    m->addTransaction(transaction);
    ft.commit();

    QCOMPARE(m->countTransactionsWithSpecificReconciliationState(accountId, eMyMoney::TransactionFilter::State::NotReconciled), 1);
}

void MyMoneyFileTest::testCountTransactionsWithSpecificReconciliationState_transactionWithUnwantedReconcileState()
{
    addOneAccount();
    setupBaseCurrency();

    QString accountId = "A000001";

    // construct split & transaction
    MyMoneySplit split;
    split.setAccountId(accountId);
    split.setShares(MyMoneyMoney(-1000, 100));
    split.setValue(MyMoneyMoney(-1000, 100));
    split.setReconcileFlag(eMyMoney::Split::State::Reconciled);

    MyMoneyTransaction transaction;
    transaction.setPostDate(QDate(2013, 1, 1));
    transaction.addSplit(split);

    // add transaction
    MyMoneyFileTransaction ft;
    m->addTransaction(transaction);
    ft.commit();

    QCOMPARE(m->countTransactionsWithSpecificReconciliationState(accountId, eMyMoney::TransactionFilter::State::NotReconciled), 0);
}

void MyMoneyFileTest::testAddOnlineJob()
{
    QSKIP("Need dummy task for this test", SkipAll);
#if 0
    // Add a onlineJob
    onlineJob job(new germanOnlineTransfer());

    MyMoneyFileTransaction ft;
    m->addOnlineJob(job);
    QCOMPARE(job.id(), QString("O000001"));
    ft.commit();

    QCOMPARE(m_objectsRemoved.count(), 0);
    QCOMPARE(m_objectsAdded.count(), 1);
    QCOMPARE(m_objectsModified.count(), 0);
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 0);
#endif
}

void MyMoneyFileTest::testGetOnlineJob()
{
    QSKIP("Need dummy task for this test", SkipAll);
#if 0
    testAddOnlineJob();

    const onlineJob requestedJob = m->getOnlineJob("O000001");
    QVERIFY(!requestedJob.isNull());
    QCOMPARE(requestedJob.id(), QString("O000001"));
#endif
}

void MyMoneyFileTest::testRemoveOnlineJob()
{
    QSKIP("Need dummy task for this test", SkipAll);
#if 0
    // Add a onlineJob
    onlineJob job(new germanOnlineTransfer());
    onlineJob job2(new germanOnlineTransfer());
    onlineJob job3(new germanOnlineTransfer());


    MyMoneyFileTransaction ft;
    m->addOnlineJob(job);
    m->addOnlineJob(job2);
    m->addOnlineJob(job3);
    ft.commit();

    clearObjectLists();

    ft.restart();
    m->removeOnlineJob(job);
    m->removeOnlineJob(job2);
    ft.commit();

    QCOMPARE(m_objectsRemoved.count(), 2);
    QCOMPARE(m_objectsAdded.count(), 0);
    QCOMPARE(m_objectsModified.count(), 0);
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 0);
#endif
}

void MyMoneyFileTest::testOnlineJobRollback()
{
    QSKIP("Need dummy task for this test", SkipAll);
#if 0
    // Add a onlineJob
    onlineJob job(new germanOnlineTransfer());
    onlineJob job2(new germanOnlineTransfer());
    onlineJob job3(new germanOnlineTransfer());

    MyMoneyFileTransaction ft;
    m->addOnlineJob(job);
    m->addOnlineJob(job2);
    m->addOnlineJob(job3);
    ft.rollback();

    QCOMPARE(m_objectsRemoved.count(), 0);
    QCOMPARE(m_objectsAdded.count(), 0);
    QCOMPARE(m_objectsModified.count(), 0);
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 0);
#endif
}

void MyMoneyFileTest::testRemoveLockedOnlineJob()
{
    QSKIP("Need dummy task for this test", SkipAll);
#if 0
    // Add a onlineJob
    onlineJob job(new germanOnlineTransfer());
    job.setLock(true);
    QVERIFY(job.isLocked());

    MyMoneyFileTransaction ft;
    m->addOnlineJob(job);
    ft.commit();

    clearObjectLists();

    // Try removing locked transfer
    ft.restart();
    m->removeOnlineJob(job);
    ft.commit();
    QVERIFY2(m_objectsRemoved.count() == 0, "Online Job was locked, removing is not allowed");
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);

#endif
}

/** @todo */
void MyMoneyFileTest::testModifyOnlineJob()
{
    QSKIP("Need dummy task for this test", SkipAll);
#if 0
    // Add a onlineJob
    onlineJob job(new germanOnlineTransfer());
    MyMoneyFileTransaction ft;
    m->addOnlineJob(job);
    ft.commit();

    clearObjectLists();

    // Modify online job
    job.setJobSend();
    ft.restart();
    m->modifyOnlineJob(job);
    ft.commit();

    QCOMPARE(m_objectsRemoved.count(), 0);
    QCOMPARE(m_objectsAdded.count(), 0);
    QCOMPARE(m_objectsModified.count(), 1);
    QCOMPARE(m_balanceChanged.count(), 0);
    QCOMPARE(m_valueChanged.count(), 0);

    //onlineJob modifiedJob = m->getOnlineJob( job.id() );
    //QCOMPARE(modifiedJob.responsibleAccount(), QString("Std::Assert"));
#endif
}

void MyMoneyFileTest::testClearedBalance()
{
    testAddTransaction();
    MyMoneyTransaction t1;
    MyMoneyTransaction t2;

    // construct a transaction and add it to the pool
    t1.setPostDate(QDate(2002, 2, 1));
    t1.setMemo("Memotext");

    t2.setPostDate(QDate(2002, 2, 4));
    t2.setMemo("Memotext");

    MyMoneySplit split1;
    MyMoneySplit split2;
    MyMoneySplit split3;
    MyMoneySplit split4;

    MyMoneyFileTransaction ft;
    try {
        split1.setAccountId("A000002");
        split1.setShares(MyMoneyMoney(-1000, 100));
        split1.setValue(MyMoneyMoney(-1000, 100));
        split1.setReconcileFlag(eMyMoney::Split::State::Cleared);
        split2.setAccountId("A000004");
        split2.setValue(MyMoneyMoney(1000, 100));
        split2.setShares(MyMoneyMoney(1000, 100));
        split2.setReconcileFlag(eMyMoney::Split::State::Cleared);
        t1.addSplit(split1);
        t1.addSplit(split2);
        m->addTransaction(t1);
        ft.commit();
        ft.restart();
        QCOMPARE(t1.id(), QLatin1String("T000000000000000002"));
        split3.setAccountId("A000002");
        split3.setShares(MyMoneyMoney(-2000, 100));
        split3.setValue(MyMoneyMoney(-2000, 100));
        split3.setReconcileFlag(eMyMoney::Split::State::Cleared);
        split4.setAccountId("A000004");
        split4.setValue(MyMoneyMoney(2000, 100));
        split4.setShares(MyMoneyMoney(2000, 100));
        split4.setReconcileFlag(eMyMoney::Split::State::Cleared);
        t2.addSplit(split3);
        t2.addSplit(split4);
        m->addTransaction(t2);
        ft.commit();
        ft.restart();

        QCOMPARE(m->balance("A000001", QDate(2002, 2, 4)), MyMoneyMoney(-1000, 100));
        QCOMPARE(m->balance("A000002", QDate(2002, 2, 4)), MyMoneyMoney(-3000, 100));
        // Date of last cleared transaction
        QCOMPARE(m->clearedBalance("A000002", QDate(2002, 2, 1)), MyMoneyMoney(-1000, 100));

        // Date of last transaction
        QCOMPARE(m->balance("A000002", QDate(2002, 2, 4)), MyMoneyMoney(-3000, 100));

        // Date before first transaction
        QVERIFY(m->clearedBalance("A000002", QDate(2002, 1, 15)).isZero());


    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testAdjustedValues()
{
    // create a checking account, an expense, an investment account and a stock
    addOneAccount();

    MyMoneyAccount exp1;
    exp1.setAccountType(eMyMoney::Account::Type::Expense);
    exp1.setName("Expense1");
    exp1.setCurrencyId("EUR");

    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->expense();
        m->addAccount(exp1, parent);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    testAddEquityAccount();

    MyMoneySecurity stockSecurity(QLatin1String("Blubber"), QLatin1String("TestStockSecurity"), QLatin1String("BLUB"), 1000, 1000, 1000);
    stockSecurity.setTradingCurrency(QLatin1String("BLUB"));
    MyMoneySecurity tradingCurrency("BLUB", "BlubCurrency");

    // add the security
    ft.restart();
    try {
        m->addCurrency(tradingCurrency);
        m->addSecurity(stockSecurity);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    MyMoneyAccount i = m->accountByName("Investment");
    MyMoneyAccount stock;
    ft.restart();
    try {
        stock.setName("Teststock");
        stock.setCurrencyId(stockSecurity.id());
        stock.setAccountType(eMyMoney::Account::Type::Stock);
        m->addAccount(stock, i);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    // values taken from real example on https://bugs.kde.org/show_bug.cgi?id=345655
    MyMoneySplit s1, s2, s3;
    s1.setAccountId(QLatin1String("A000001"));
    s1.setShares(MyMoneyMoney(QLatin1String("-99901/1000")));
    s1.setValue(MyMoneyMoney(QLatin1String("-999/10")));

    s2.setAccountId(exp1.id());
    s2.setShares(MyMoneyMoney(QLatin1String("-611/250")));
    s2.setValue(MyMoneyMoney(QLatin1String("-61/25")));

    s3.setAccountId(stock.id());
    s3.setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);
    s3.setShares(MyMoneyMoney(QLatin1String("64901/100000")));
    s3.setPrice(MyMoneyMoney(QLatin1String("157689/1000")));
    s3.setValue(MyMoneyMoney(QLatin1String("102340161/1000000")));

    MyMoneyTransaction t;
    t.setCommodity(QLatin1String("EUR"));
    t.setPostDate(QDate::currentDate());
    t.addSplit(s1);
    t.addSplit(s2);
    t.addSplit(s3);

    // make sure the split sum is not zero
    QVERIFY(!t.splitSum().isZero());

    ft.restart();
    try {
        m->addTransaction(t);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    QCOMPARE(t.splitById(s1.id()).shares(), MyMoneyMoney(QLatin1String("-999/10")));
    QCOMPARE(t.splitById(s1.id()).value(), MyMoneyMoney(QLatin1String("-999/10")));

    QCOMPARE(t.splitById(s2.id()).shares(), MyMoneyMoney(QLatin1String("-61/25")));
    QCOMPARE(t.splitById(s2.id()).value(), MyMoneyMoney(QLatin1String("-61/25")));

    QCOMPARE(t.splitById(s3.id()).shares(), MyMoneyMoney(QLatin1String("649/1000")));
    QCOMPARE(t.splitById(s3.id()).value(), MyMoneyMoney(QLatin1String("10234/100")));
    QCOMPARE(t.splitById(s3.id()).price(), MyMoneyMoney(QLatin1String("157689/1000")));
    QCOMPARE(t.splitSum(), MyMoneyMoney());

    // now reset and check if modify also works
    s1.setShares(MyMoneyMoney(QLatin1String("-999/10")));
    s1.setValue(MyMoneyMoney(QLatin1String("-999/10")));

    s2.setShares(MyMoneyMoney(QLatin1String("-61/25")));
    s2.setValue(MyMoneyMoney(QLatin1String("-61/25")));

    s3.setShares(MyMoneyMoney(QLatin1String("649/1000")));
    s3.setPrice(MyMoneyMoney(QLatin1String("157689/1000")));
    s3.setValue(MyMoneyMoney(QLatin1String("102340161/1000000")));

    t.modifySplit(s1);
    t.modifySplit(s2);
    t.modifySplit(s3);

    // make sure the split sum is not zero
    QVERIFY(!t.splitSum().isZero());

    ft.restart();
    try {
        m->modifyTransaction(t);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // we need to get the transaction from the engine, as modifyTransaction does
    // not return the modified values
    MyMoneyTransaction t2 = m->transaction(t.id());

    QCOMPARE(t2.splitById(s3.id()).shares(), MyMoneyMoney(QLatin1String("649/1000")));
    QCOMPARE(t2.splitById(s3.id()).value(), MyMoneyMoney(QLatin1String("10234/100")));
    QCOMPARE(t2.splitById(s3.id()).price(), MyMoneyMoney(QLatin1String("157689/1000")));
    QCOMPARE(t2.splitSum(), MyMoneyMoney());
}

void MyMoneyFileTest::testVatAssignment()
{
    MyMoneyAccount acc;
    MyMoneyAccount vat;
    MyMoneyAccount expense;

    testAddTransaction();

    vat.setName("VAT");
    vat.setCurrencyId("EUR");
    vat.setAccountType(eMyMoney::Account::Type::Expense);
    // make it a VAT account
    vat.setValue(QLatin1String("VatRate"), QLatin1String("20/100"));

    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->expense();
        m->addAccount(vat, parent);
        QVERIFY(!vat.id().isEmpty());
        acc = m->account(QLatin1String("A000001"));
        expense = m->account(QLatin1String("A000003"));
        QCOMPARE(acc.name(), QLatin1String("Account1"));
        QCOMPARE(expense.name(), QLatin1String("Expense1"));
        expense.setValue(QLatin1String("VatAccount"), vat.id());
        m->modifyAccount(expense);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // the categories are setup now for gross value entry
    MyMoneyTransaction tr;
    MyMoneySplit sp;
    MyMoneyMoney amount(1707, 100);

    // setup the transaction
    sp.setShares(amount);
    sp.setValue(amount);
    sp.setAccountId(acc.id());
    tr.addSplit(sp);
    sp.clearId();
    sp.setShares(-amount);
    sp.setValue(-amount);
    sp.setAccountId(expense.id());
    tr.addSplit(sp);

    QCOMPARE(m->addVATSplit(tr, acc, expense, amount), true);
    QCOMPARE(tr.splits().count(), 3);
    QCOMPARE(tr.splitByAccount(acc.id()).shares().toString(), MyMoneyMoney(1707, 100).toString());
    QCOMPARE(tr.splitByAccount(expense.id()).shares().toString(), MyMoneyMoney(-1422, 100).toString());
    QCOMPARE(tr.splitByAccount(vat.id()).shares().toString(), MyMoneyMoney(-285, 100).toString());
    QCOMPARE(tr.splitSum().toString(), MyMoneyMoney().toString());

    tr.removeSplits();
    ft.restart();
    try {
        expense.setValue(QLatin1String("VatAmount"), QLatin1String("net"));
        m->modifyAccount(expense);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    // the categories are setup now for net value entry
    amount = MyMoneyMoney(1422, 100);
    sp.clearId();
    sp.setShares(amount);
    sp.setValue(amount);
    sp.setAccountId(acc.id());
    tr.addSplit(sp);
    sp.clearId();
    sp.setShares(-amount);
    sp.setValue(-amount);
    sp.setAccountId(expense.id());
    tr.addSplit(sp);

    QCOMPARE(m->addVATSplit(tr, acc, expense, amount), true);
    QCOMPARE(tr.splits().count(), 3);
    QCOMPARE(tr.splitByAccount(acc.id()).shares().toString(), MyMoneyMoney(1706, 100).toString());
    QCOMPARE(tr.splitByAccount(expense.id()).shares().toString(), MyMoneyMoney(-1422, 100).toString());
    QCOMPARE(tr.splitByAccount(vat.id()).shares().toString(), MyMoneyMoney(-284, 100).toString());
    QCOMPARE(tr.splitSum().toString(), MyMoneyMoney().toString());
}

void MyMoneyFileTest::testEmptyFilter()
{
    testAddTransaction();

    try {
        QList<QPair<MyMoneyTransaction, MyMoneySplit> > tList;
        MyMoneyTransactionFilter filter;
        MyMoneyFile::instance()->transactionList(tList, filter);
        QCOMPARE(tList.count(), 2);

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }
}

void MyMoneyFileTest::testAddSecurity()
{
    // create a checking account, an expense, an investment account and a stock
    addOneAccount();

    MyMoneyAccount exp1;
    exp1.setAccountType(eMyMoney::Account::Type::Expense);
    exp1.setName("Expense1");
    exp1.setCurrencyId("EUR");

    MyMoneyFileTransaction ft;
    try {
        MyMoneyAccount parent = m->expense();
        m->addAccount(exp1, parent);
        ft.commit();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception!");
    }

    testAddEquityAccount();

    MyMoneySecurity stockSecurity(QLatin1String("Blubber"), QLatin1String("TestsockSecurity"), QLatin1String("BLUB"), 1000, 1000, 1000);
    stockSecurity.setTradingCurrency(QLatin1String("BLUB"));
    // add the security
    ft.restart();
    try {
        m->addSecurity(stockSecurity);
        ft.commit();
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    // check that we can get it via the security method
    try {
        MyMoneySecurity sec = m->security(stockSecurity.id());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    // and also via the currency method
    try {
        MyMoneySecurity sec = m->currency(stockSecurity.id());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}
