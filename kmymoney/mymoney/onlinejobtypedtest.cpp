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

void onlineJobTypedTest::copyContructorFailure()
{
  QSKIP("Need second dummy task for testing", SkipAll);
  try {
    onlineJobTyped<dummyTask> job( new dummyTask );
    QFAIL("Missing expected exception");
  } catch ( onlineJob::badTaskCast* e ) {
    delete e;
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
