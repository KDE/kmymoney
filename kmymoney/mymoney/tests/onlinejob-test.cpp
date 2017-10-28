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

#include "onlinejob-test.h"

#include <QtTest/QTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class onlineJobTest;

#include "onlinejob.h"

#include "onlinetasks/dummy/tasks/dummytask.h"

QTEST_GUILESS_MAIN(onlineJobTest)

void onlineJobTest::testDefaultConstructor()
{
  const onlineJob job = onlineJob();
  QVERIFY(job.id() == MyMoneyObject::emptyId());
  QVERIFY(job.isNull());
  QVERIFY(job.sendDate().isNull());
  QVERIFY(job.bankAnswerDate().isNull());
  QVERIFY(job.bankAnswerState() == onlineJob::noBankAnswer);
  QVERIFY(job.jobMessageList().isEmpty());
  QVERIFY(job.isLocked() == false);

}

void onlineJobTest::testCopyConstructor()
{
  onlineJob originalJob = onlineJob(new dummyTask, "O000001");
  QVERIFY(!originalJob.isNull());
  QVERIFY(originalJob.task());

  onlineJob jobCopy = onlineJob(originalJob);
  QVERIFY(!jobCopy.isNull());
  QCOMPARE(jobCopy.id(), QString("O000001"));
  QVERIFY(originalJob.task() != jobCopy.task());

}

void onlineJobTest::testCopyAssignment()
{
  onlineJob originalJob = onlineJob(new dummyTask, "O000001");
  QVERIFY(!originalJob.isNull());
  QVERIFY(originalJob.task());

  onlineJob jobCopy;
  jobCopy = originalJob;
  QVERIFY(!jobCopy.isNull());
  QCOMPARE(jobCopy.id(), QString("O000001"));
  QVERIFY(originalJob.task() != jobCopy.task());
}

void onlineJobTest::testCopyConstructorWithNewId()
{
  onlineJob originalJob = onlineJob(new dummyTask, "O000001");
  originalJob.setBankAnswer(onlineJob::acceptedByBank);
  QVERIFY(!originalJob.isNull());

  onlineJob jobCopy = onlineJob("O000002", originalJob);
  QVERIFY(!jobCopy.isNull());
  QCOMPARE(jobCopy.id(), QString("O000002"));
  QVERIFY(originalJob.task() != jobCopy.task());
  QVERIFY(jobCopy.bankAnswerDate().isNull());
}

void onlineJobTest::testElementNames()
{
  for (auto i = (int)onlineJob::Element::OnlineTask; i <= (int)onlineJob::Element::OnlineTask; ++i) {
    auto isEmpty = onlineJob::getElName(static_cast<onlineJob::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void onlineJobTest::testAttributeNames()
{
  for (auto i = (int)onlineJob::Attribute::Send; i < (int)onlineJob::Attribute::LastAttribute; ++i) {
    auto isEmpty = onlineJob::getAttrName(static_cast<onlineJob::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
