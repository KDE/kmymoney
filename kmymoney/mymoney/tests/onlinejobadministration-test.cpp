/*
 * Copyright 2013-2016  Christian Dávid <christian-david@web.de>
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
  } catch (const MyMoneyException &e) {
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
    file->addAccount(account , asset);
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
