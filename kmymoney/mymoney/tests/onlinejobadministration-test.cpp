/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#include "mymoney/storage/mymoneyseqaccessmgr.h"
#include "onlinetasks/dummy/tasks/dummytask.h"

QTEST_GUILESS_MAIN(onlineJobAdministrationTest)

void onlineJobAdministrationTest::initTestCase()
{
  file = MyMoneyFile::instance();
  storage = new MyMoneySeqAccessMgr;
  file->attachStorage(storage);

  try {
    MyMoneyAccount account = MyMoneyAccount();
    account.setName("Test Account");
    account.setAccountType(eMyMoney::Account::Type::Savings);
    MyMoneyAccount asset = file->asset();
    MyMoneyFileTransaction transaction;
    file->addAccount(account , asset);
    accountId = account.id();
    transaction.commit();
  } catch (const MyMoneyException& ex) {
    QFAIL(qPrintable("Unexpected exception " + ex.what()));
  }
}

void onlineJobAdministrationTest::cleanupTestCase()
{
  file->detachStorage(storage);
  delete storage;
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
