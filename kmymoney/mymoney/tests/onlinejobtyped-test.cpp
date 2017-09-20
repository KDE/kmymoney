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

#include "onlinejobtyped-test.h"

#include <QtTest/QTest>

#include "onlinejobtyped.h"
#include "onlinetasks/dummy/tasks/dummytask.h"

QTEST_GUILESS_MAIN(onlineJobTypedTest)

class dummyTask2 : public dummyTask {};

class onlineTaskInterface {};
class onlineTaskDummy3 : public onlineTask, public onlineTaskInterface
{
public:
  ONLINETASK_META(dummyTask, "org.kmymoney.onlinetasks.dummytask");

  virtual bool isValid() const {
    return true;
  }
  virtual QString jobTypeName() const {
    return QLatin1String("Dummy credit transfer");
  }
  virtual QString storagePluginIid() const {
    return QString();
  }
  virtual bool sqlModify(QSqlDatabase, const QString&) const {
    return false;
  }
  virtual bool sqlSave(QSqlDatabase, const QString&) const {
    return false;
  }
  virtual bool sqlRemove(QSqlDatabase, const QString&) const {
    return false;
  }

protected:

  virtual onlineTaskDummy3* clone() const {
    return (new onlineTaskDummy3);
  }
  virtual bool hasReferenceTo(const QString&) const {
    return false;
  }
  virtual void writeXML(QDomDocument&, QDomElement&) const {}
  virtual onlineTaskDummy3* createFromXml(const QDomElement &) const {
    return (new onlineTaskDummy3);
  }
  virtual onlineTask* createFromSqlDatabase(QSqlDatabase, const QString&) const {
    return (new onlineTaskDummy3);
  }
  virtual QString responsibleAccount() const {
    return QString();
  };
};

void onlineJobTypedTest::initTestCase()
{

}

void onlineJobTypedTest::cleanupTestCase()
{

}

void onlineJobTypedTest::copyContructor()
{
  dummyTask* task = new dummyTask;
  onlineJobTyped<dummyTask> job(task);
  QVERIFY(!job.isNull());
  QVERIFY(job.m_task == task);
}

void onlineJobTypedTest::constructWithIncompatibleType()
{
  try {
    onlineJobTyped<dummyTask2> job(new dummyTask);
    QFAIL("Missing expected exception");
  } catch (const onlineJob::badTaskCast&) {
  } catch (...) {
    QFAIL("Wrong exception thrown");
  }
}

void onlineJobTypedTest::constructWithNull()
{
  try {
    onlineJobTyped<dummyTask> job(0);
    QFAIL("Missing expected exception");
  } catch (const onlineJob::emptyTask&) {
  } catch (...) {
    QFAIL("Wrong exception thrown");
  }
}

void onlineJobTypedTest::copyByAssignment()
{
  dummyTask* task = new dummyTask;
  task->setTestNumber(8888);
  onlineJobTyped<dummyTask> job(new dummyTask);
  job = onlineJobTyped<dummyTask>(task);

  QVERIFY(!job.isNull());
  QVERIFY(dynamic_cast<dummyTask*>(job.task()));
  QCOMPARE(job.task()->testNumber(), 8888);
}

void onlineJobTypedTest::constructWithManadtoryDynamicCast()
{
  onlineJob job(new onlineTaskDummy3);
  try {
    onlineJobTyped<onlineTaskInterface> jobTyped(job);
  } catch (...) {
    QFAIL("Unexpected exception");
  }
}
