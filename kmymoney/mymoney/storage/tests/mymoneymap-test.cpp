/*
    SPDX-FileCopyrightText: 2007-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneymap-test.h"
#include <iostream>
#include <QTest>

QTEST_GUILESS_MAIN(MyMoneyMapTest)

void MyMoneyMapTest::init()
{
    m = new MyMoneyMap<QString, QString>;
}

void MyMoneyMapTest::cleanup()
{
    delete m;
}

void MyMoneyMapTest::testArrayOperator()
{
    QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyMapTest::testModifyKey()
{
    // setup
    m->startTransaction();
    m->insert("a", "a");
    m->commitTransaction();
    QVERIFY((*m)["a"] == "a");

    // commit
    m->startTransaction();
    m->modify("a", "b");
    QVERIFY((*m)["a"] == "b");
    m->commitTransaction();
    QVERIFY((*m)["a"] == "b");

    // rollback
    m->startTransaction();
    m->modify("a", "c");
    QVERIFY((*m)["a"] == "c");
    m->rollbackTransaction();
    QVERIFY((*m)["a"] == "b");
}

void MyMoneyMapTest::testModifyKeyTwice()
{
    m->startTransaction();
    m->insert("a", "a");
    QVERIFY((*m)["a"] == "a");
    m->insert("b", "b");
    QVERIFY((*m)["a"] == "a");
    QVERIFY((*m)["b"] == "b");

    m->modify("a", "b");
    QVERIFY((*m)["a"] == "b");

    m->rollbackTransaction();

    QVERIFY(m->count() == 0);

    m->startTransaction();
    m->insert("a", "a");
    m->insert("b", "b");
    m->commitTransaction();
    QVERIFY((*m)["a"] == "a");
    QVERIFY((*m)["b"] == "b");

    m->startTransaction();
    m->modify("a", "b");
    m->modify("a", "c");
    QVERIFY((*m)["a"] == "c");

    m->rollbackTransaction();
    QVERIFY((*m)["a"] == "a");
}
