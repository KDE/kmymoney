/*
    SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejobtyped-test.h"

#include <QTest>

#include "onlinejob.h"
#include "onlinejobtyped.h"
#include "onlinetasks/dummy/tasks/dummytask.h"

QTEST_GUILESS_MAIN(onlineJobTypedTest)

class dummyTask2 : public dummyTask {};

class onlineTaskInterface {};
class onlineTaskDummy3 : public onlineTask, public onlineTaskInterface
{
public:
    ONLINETASK_META(dummyTask, "org.kmymoney.onlinetasks.dummytask");

    bool isValid() const final override
    {
        return true;
    }
    QString jobTypeName() const final override
    {
        return QLatin1String("Dummy credit transfer");
    }

    void writeXML(QXmlStreamWriter*) const final override
    {
    }

protected:

    onlineTaskDummy3* clone() const final override {
        return (new onlineTaskDummy3);
    }
    bool hasReferenceTo(const QString&) const final override {
        return false;
    }
    QSet<QString> referencedObjects() const final override {
        return {};
    }
    onlineTaskDummy3* createFromXml(QXmlStreamReader*) const final override
    {
        return (new onlineTaskDummy3);
    }
    QString responsibleAccount() const final override {
        return QString();
    }
    QString purpose() const final override
    {
        return QString();
    }
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
