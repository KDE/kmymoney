/*
    SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejobadministration-test.h"

#include <QTest>

#include "onlinejobadministration.h"
#include "mymoney/mymoneyfile.h"
#include "mymoneyaccount.h"
#include "onlinetasks/dummy/tasks/dummytask.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"
#include "mymoneysecurity.h"

QTEST_GUILESS_MAIN(onlineJobAdministrationTest)

onlineJobAdministrationTest::onlineJobAdministrationTest()
    : file(nullptr)
{
}

void onlineJobAdministrationTest::setupBaseCurrency()
{
    file = MyMoneyFile::instance();

    MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
    MyMoneyFileTransaction ft;
    try {
        file->currency(base.id());
    } catch (const MyMoneyException&) {
        file->addCurrency(base);
    }
    file->setBaseCurrency(base);
    ft.commit();
}


void onlineJobAdministrationTest::initTestCase()
{
    setupBaseCurrency();
    file = MyMoneyFile::instance();

    try {
        MyMoneyAccount account = MyMoneyAccount();
        account.setName("Test Account");
        account.setAccountType(eMyMoney::Account::Type::Savings);
        MyMoneyAccount asset = file->asset();
        MyMoneyFileTransaction transaction;
        file->addAccount(account, asset);
        accountId = account.id();
        transaction.commit();
    } catch (const MyMoneyException &ex) {
        QFAIL(qPrintable(QString::fromLatin1("Unexpected exception %1").arg(ex.what())));
    }
}

void onlineJobAdministrationTest::cleanupTestCase()
{
}

void onlineJobAdministrationTest::init()
{
    qDeleteAll(onlineJobAdministration::instance()->m_onlineTasks);
    onlineJobAdministration::instance()->m_onlineTasks.clear();
}

void onlineJobAdministrationTest::getSettings()
{
}

void onlineJobAdministrationTest::registerOnlineTask()
{
    dummyTask *task = new dummyTask;
    onlineJobAdministration::instance()->registerOnlineTask(task);
    QCOMPARE(onlineJobAdministration::instance()->m_onlineTasks.count(), 1);
    QVERIFY(onlineJobAdministration::instance()->m_onlineTasks.value(task->taskName()));
}
