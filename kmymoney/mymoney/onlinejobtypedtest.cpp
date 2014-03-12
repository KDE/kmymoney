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
