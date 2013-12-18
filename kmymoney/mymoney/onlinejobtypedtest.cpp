#include "onlinejobtypedtest.h"

#include <QtTest/QTest>

#include "onlinejobtyped.h"
#include "germanonlinetransfer.h"
#include "sepaonlinetransfer.h"

QTEST_MAIN(onlineJobTypedTest)

void onlineJobTypedTest::initTestCase()
{

}

void onlineJobTypedTest::cleanupTestCase()
{

}

void onlineJobTypedTest::copyContructor()
{
  sepaOnlineTransfer* task = new sepaOnlineTransfer;
  onlineJobTyped<sepaOnlineTransfer> job( task );
  QVERIFY( !job.isNull() );
  QVERIFY( job.m_taskSubType == task );
}

void onlineJobTypedTest::copyContructorFailure()
{
  try {
    onlineJobTyped<germanOnlineTransfer> job( new sepaOnlineTransfer );
    QFAIL("Missing expected exception");
  } catch ( onlineJob::badTaskCast* ) {

  }
}

void onlineJobTypedTest::copyByAssignment()
{
  sepaOnlineTransfer* task = new sepaOnlineTransfer;
  task->setPurpose( "Test string" );
  onlineJobTyped<sepaOnlineTransfer> job( new sepaOnlineTransfer );
  job = onlineJobTyped<sepaOnlineTransfer>( task );

  QVERIFY( !job.isNull() );
  QVERIFY( dynamic_cast<sepaOnlineTransfer*>(job.task()));
  QVERIFY( job.task()->purpose() == "Test string" );
}
