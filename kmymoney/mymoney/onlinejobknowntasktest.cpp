#include "onlinejobknowntasktest.h"

#include <QtTest/QTest>

#include "onlinejobknowntask.h"
#include "germanonlinetransfer.h"
#include "sepaonlinetransfer.h"

QTEST_MAIN(onlineJobKnownTaskTest)

void onlineJobKnownTaskTest::initTestCase()
{

}

void onlineJobKnownTaskTest::cleanupTestCase()
{

}

void onlineJobKnownTaskTest::copyContructor()
{
  sepaOnlineTransfer* task = new sepaOnlineTransfer;
  onlineJobKnownTask<sepaOnlineTransfer> job( task );
  QVERIFY( !job.isNull() );
  QVERIFY( job.m_taskSubType == task );
}

void onlineJobKnownTaskTest::copyContructorFailure()
{
  try {
    onlineJobKnownTask<germanOnlineTransfer> job( new sepaOnlineTransfer );
    QFAIL("Missing expected exception");
  } catch ( onlineJob::badTaskCast* ) {

  }
}

void onlineJobKnownTaskTest::copyByAssignment()
{
  sepaOnlineTransfer* task = new sepaOnlineTransfer;
  task->setPurpose( "Test string" );
  onlineJobKnownTask<sepaOnlineTransfer> job( new sepaOnlineTransfer );
  job = onlineJobKnownTask<sepaOnlineTransfer>( task );

  QVERIFY( !job.isNull() );
  QVERIFY( dynamic_cast<sepaOnlineTransfer*>(job.task()));
  QVERIFY( job.task()->purpose() == "Test string" );
}
