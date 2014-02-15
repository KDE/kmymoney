#include "onlinejobtest.h"

#include <QtTest/QTest>
#include "onlinejob.h"

#include "onlinetasks/dummy/tasks/dummytask.h"

QTEST_MAIN(onlineJobTest)

void onlineJobTest::testDefaultConstructor()
{
  const onlineJob job = onlineJob();
  QVERIFY( job.id() == MyMoneyObject::emptyId() );
  QVERIFY( job.isNull() );
  QVERIFY( job.sendDate().isNull() );
  QVERIFY( job.bankAnswerDate().isNull() );
  QVERIFY( job.bankAnswerState() == onlineJob::noBankAnswer );
  QVERIFY( job.jobMessageList().isEmpty() );
  QVERIFY( job.isLocked() == false );

}

void onlineJobTest::testCopyConstructor()
{
    onlineJob originalJob = onlineJob( new dummyTask, "O000001" );
    QVERIFY( !originalJob.isNull() );
    QVERIFY( originalJob.task() );

    onlineJob jobCopy = onlineJob(originalJob);
    QVERIFY( !jobCopy.isNull() );
    QCOMPARE( jobCopy.id(), QString("O000001"));
    QVERIFY( originalJob.task() != jobCopy.task() );

}

void onlineJobTest::testCopyAssignment()
{
    onlineJob originalJob = onlineJob( new dummyTask, "O000001" );
    QVERIFY( !originalJob.isNull() );
    QVERIFY( originalJob.task() );

    onlineJob jobCopy;
    jobCopy = originalJob;
    QVERIFY( !jobCopy.isNull() );
    QCOMPARE( jobCopy.id(), QString("O000001"));
    QVERIFY( originalJob.task() != jobCopy.task() );
}

void onlineJobTest::testCopyConstructorWithNewId()
{
    onlineJob originalJob = onlineJob( new dummyTask, "O000001" );
    QVERIFY( !originalJob.isNull() );

    onlineJob jobCopy = onlineJob("O000002", originalJob);
    QVERIFY( !jobCopy.isNull() );
    QCOMPARE( jobCopy.id(), QString("O000002"));
    QVERIFY( originalJob.task() != jobCopy.task() );
}
