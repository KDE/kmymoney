/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#include "onlinejobtypedtest.h"

#include <QtTest/QTest>

#include "onlinejobtyped.h"
#include "onlinetasks/dummy/tasks/dummytask.h"

QTEST_MAIN(onlineJobTypedTest)

void onlineJobTypedTest::initTestCase()
{

}

void onlineJobTypedTest::cleanupTestCase()
{

}

void onlineJobTypedTest::copyContructor()
{
  dummyTask* task = new dummyTask;
  onlineJobTyped<dummyTask> job( task );
  QVERIFY( !job.isNull() );
  QVERIFY( job.m_taskSubType == task );
}

class dummyClass2 : public dummyTask {};

void onlineJobTypedTest::constructWithIncompatibleType()
{
  try {
    onlineJobTyped<dummyClass2> job( new dummyTask );
    QFAIL("Missing expected exception");
  } catch ( const onlineJob::badTaskCast& ) {
  }
}

void onlineJobTypedTest::constructWithNull()
{
  try {
    onlineJobTyped<dummyTask> job( 0 );
    QFAIL("Missing expected exception");
  } catch ( const onlineJob::emptyTask& ) {
  }
}

void onlineJobTypedTest::copyByAssignment()
{
  dummyTask* task = new dummyTask;
  task->setTestNumber( 8888 );
  onlineJobTyped<dummyTask> job( new dummyTask );
  job = onlineJobTyped<dummyTask>( task );

  QVERIFY( !job.isNull() );
  QVERIFY( dynamic_cast<dummyTask*>(job.task()));
  QCOMPARE(job.task()->testNumber(), 8888 );
}
