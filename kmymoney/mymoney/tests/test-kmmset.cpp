/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#define KMM_MYMONEY_UNIT_TESTABLE friend class KMMSetTest;

#include "test-kmmset.h"

#include <string>

#include <QTest>

QTEST_GUILESS_MAIN(KMMSetTest)

void KMMSetTest::init()
{
}

void KMMSetTest::cleanup()
{
}

void KMMSetTest::testEmptyConstructor()
{
    KMMSet<int> intSet;
    KMMSet<QString> qStringSet;
    KMMSet<std::string> stdStringSet;

    QCOMPARE(intSet.size(), 0);
    QCOMPARE(qStringSet.size(), 0);
    QCOMPARE(stdStringSet.size(), 0);

    QCOMPARE(intSet.size(), 0);
    QCOMPARE(qStringSet.size(), 0);
    QCOMPARE(stdStringSet.size(), 0);

    QCOMPARE(intSet.empty(), true);
    QCOMPARE(qStringSet.empty(), true);
    QCOMPARE(stdStringSet.empty(), true);

    QCOMPARE(intSet.isEmpty(), true);
    QCOMPARE(qStringSet.isEmpty(), true);
    QCOMPARE(stdStringSet.isEmpty(), true);
}

void KMMSetTest::testConstructor()
{
    KMMSet<int> intSet({1, 2, 3});
    KMMSet<QString> qStringSet(QStringList({"1", "2", "3"}));
    KMMSet<std::string> stdStringSet({"1", "2", "3"});

    QCOMPARE(intSet.size(), 3);
    QCOMPARE(intSet.empty(), false);
    QCOMPARE(intSet.isEmpty(), false);
    QCOMPARE(intSet.count(1), 1);
    QCOMPARE(intSet.count(2), 1);
    QCOMPARE(intSet.count(3), 1);
    QCOMPARE(intSet.count(4), 0);

    QCOMPARE(qStringSet.size(), 3);
    QCOMPARE(qStringSet.empty(), false);
    QCOMPARE(qStringSet.isEmpty(), false);
    QCOMPARE(qStringSet.count("1"), 1);
    QCOMPARE(qStringSet.count("2"), 1);
    QCOMPARE(qStringSet.count("3"), 1);
    QCOMPARE(qStringSet.count("4"), 0);

    QCOMPARE(stdStringSet.size(), 3);
    QCOMPARE(stdStringSet.empty(), false);
    QCOMPARE(stdStringSet.isEmpty(), false);
    QCOMPARE(stdStringSet.count("1"), 1);
    QCOMPARE(stdStringSet.count("2"), 1);
    QCOMPARE(stdStringSet.count("3"), 1);
    QCOMPARE(stdStringSet.count("4"), 0);
}

void KMMSetTest::testCopyConstructor()
{
    KMMSet<int> intSet1({1, 2, 3});
    KMMSet<QString> qStringSet1(QStringList({"1", "2", "3"}));
    KMMSet<std::string> stdStringSet1({"1", "2", "3"});

    KMMSet<int> intSet(intSet1);
    KMMSet<QString> qStringSet(qStringSet1);
    KMMSet<std::string> stdStringSet(stdStringSet1);

    QCOMPARE(intSet1.size(), 3);
    QCOMPARE(qStringSet1.size(), 3);
    QCOMPARE(stdStringSet1.size(), 3);

    QCOMPARE(intSet.size(), 3);
    QCOMPARE(intSet.count(1), 1);
    QCOMPARE(intSet.count(2), 1);
    QCOMPARE(intSet.count(3), 1);
    QCOMPARE(intSet.count(4), 0);

    QCOMPARE(qStringSet.size(), 3);
    QCOMPARE(qStringSet.count("1"), 1);
    QCOMPARE(qStringSet.count("2"), 1);
    QCOMPARE(qStringSet.count("3"), 1);
    QCOMPARE(qStringSet.count("4"), 0);

    QCOMPARE(stdStringSet.size(), 3);
    QCOMPARE(stdStringSet.count("1"), 1);
    QCOMPARE(stdStringSet.count("2"), 1);
    QCOMPARE(stdStringSet.count("3"), 1);
    QCOMPARE(stdStringSet.count("4"), 0);
}

void KMMSetTest::testAssignmentConstructor()
{
    KMMSet<int> intSet1({1, 2, 3});
    KMMSet<QString> qStringSet1(QStringList({"1", "2", "3"}));
    KMMSet<std::string> stdStringSet1({"1", "2", "3"});

    KMMSet<int> intSet = intSet1;
    KMMSet<QString> qStringSet = qStringSet1;
    KMMSet<std::string> stdStringSet = stdStringSet1;

    QCOMPARE(intSet1.size(), 3);
    QCOMPARE(qStringSet1.size(), 3);
    QCOMPARE(stdStringSet1.size(), 3);

    QCOMPARE(intSet.size(), 3);
    QCOMPARE(intSet.count(1), 1);
    QCOMPARE(intSet.count(2), 1);
    QCOMPARE(intSet.count(3), 1);
    QCOMPARE(intSet.count(4), 0);

    QCOMPARE(qStringSet.size(), 3);
    QCOMPARE(qStringSet.count("1"), 1);
    QCOMPARE(qStringSet.count("2"), 1);
    QCOMPARE(qStringSet.count("3"), 1);
    QCOMPARE(qStringSet.count("4"), 0);

    QCOMPARE(stdStringSet.size(), 3);
    QCOMPARE(stdStringSet.count("1"), 1);
    QCOMPARE(stdStringSet.count("2"), 1);
    QCOMPARE(stdStringSet.count("3"), 1);
    QCOMPARE(stdStringSet.count("4"), 0);
}

void KMMSetTest::testDuplicateItems()
{
    KMMSet<int> intSet({1, 2, 3, 1});
    KMMSet<QString> qStringSet(QStringList({"1", "2", "3", "1"}));
    KMMSet<std::string> stdStringSet({"1", "2", "3", "1"});

    QCOMPARE(intSet.size(), 3);
    QCOMPARE(intSet.count(1), 1);
    QCOMPARE(intSet.count(2), 1);
    QCOMPARE(intSet.count(3), 1);
    QCOMPARE(intSet.count(4), 0);

    QCOMPARE(qStringSet.size(), 3);
    QCOMPARE(qStringSet.count("1"), 1);
    QCOMPARE(qStringSet.count("2"), 1);
    QCOMPARE(qStringSet.count("3"), 1);
    QCOMPARE(qStringSet.count("4"), 0);

    QCOMPARE(stdStringSet.size(), 3);
    QCOMPARE(stdStringSet.count("1"), 1);
    QCOMPARE(stdStringSet.count("2"), 1);
    QCOMPARE(stdStringSet.count("3"), 1);
    QCOMPARE(stdStringSet.count("4"), 0);
}

void KMMSetTest::testContains()
{
    KMMSet<int> intSet({1, 2, 3});
    KMMSet<QString> qStringSet(QStringList({"1", "2", "3"}));
    KMMSet<std::string> stdStringSet({"1", "2", "3"});

    QCOMPARE(intSet.contains(1), true);
    QCOMPARE(intSet.contains(2), true);
    QCOMPARE(intSet.contains(3), true);
    QCOMPARE(intSet.contains(4), false);

    QCOMPARE(qStringSet.contains("1"), true);
    QCOMPARE(qStringSet.contains("2"), true);
    QCOMPARE(qStringSet.contains("3"), true);
    QCOMPARE(qStringSet.contains("4"), false);

    QCOMPARE(stdStringSet.contains("1"), true);
    QCOMPARE(stdStringSet.contains("2"), true);
    QCOMPARE(stdStringSet.contains("3"), true);
    QCOMPARE(stdStringSet.contains("4"), false);
}

void KMMSetTest::testIntersect()
{
    KMMSet<int> intSet1({1, 2});
    KMMSet<QString> qStringSet1(QStringList({"1", "2"}));
    KMMSet<std::string> stdStringSet1({"1", "2"});

    KMMSet<int> intSet2({2, 3});
    KMMSet<QString> qStringSet2(QStringList({"2", "3"}));
    KMMSet<std::string> stdStringSet2({"2", "3"});

    intSet1 &= intSet2;
    qStringSet1 &= qStringSet2;
    stdStringSet1 &= stdStringSet2;

    QCOMPARE(intSet1.size(), 1);
    QCOMPARE(intSet1.contains(2), true);
    QCOMPARE(intSet1.contains(3), false);
    QCOMPARE(intSet2.size(), 2);

    QCOMPARE(qStringSet1.size(), 1);
    QCOMPARE(qStringSet1.contains("2"), true);
    QCOMPARE(qStringSet1.contains("3"), false);
    QCOMPARE(qStringSet2.size(), 2);

    QCOMPARE(stdStringSet1.size(), 1);
    QCOMPARE(stdStringSet1.contains("2"), true);
    QCOMPARE(stdStringSet1.contains("3"), false);
    QCOMPARE(stdStringSet2.size(), 2);
}

void KMMSetTest::testUnite()
{
    KMMSet<int> intSet1({1, 2});
    KMMSet<QString> qStringSet1(QStringList({"1", "2"}));
    KMMSet<std::string> stdStringSet1({"1", "2"});

    KMMSet<int> intSet2({2, 3});
    KMMSet<QString> qStringSet2(QStringList({"2", "3"}));
    KMMSet<std::string> stdStringSet2({"2", "3"});

    intSet1.unite(intSet2);
    qStringSet1.unite(qStringSet2);
    stdStringSet1.unite(stdStringSet2);

    QCOMPARE(intSet1.size(), 3);
    QCOMPARE(intSet1.count(1), 1);
    QCOMPARE(intSet1.count(2), 1);
    QCOMPARE(intSet1.count(3), 1);
    QCOMPARE(intSet2.size(), 2);

    QCOMPARE(qStringSet1.size(), 3);
    QCOMPARE(qStringSet1.count("1"), 1);
    QCOMPARE(qStringSet1.count("2"), 1);
    QCOMPARE(qStringSet1.count("3"), 1);
    QCOMPARE(qStringSet2.size(), 2);

    QCOMPARE(stdStringSet1.size(), 3);
    QCOMPARE(stdStringSet1.count("1"), 1);
    QCOMPARE(stdStringSet1.count("2"), 1);
    QCOMPARE(stdStringSet1.count("3"), 1);
    QCOMPARE(stdStringSet2.size(), 2);
}

void KMMSetTest::testSubtract()
{
    KMMSet<int> intSet1({1, 2, 4, 3});
    KMMSet<QString> qStringSet1(QStringList({"1", "2", "4", "3"}));
    KMMSet<std::string> stdStringSet1({"1", "2", "4", "3"});

    KMMSet<int> intSet2({2, 3});
    KMMSet<QString> qStringSet2(QStringList({"2", "3"}));
    KMMSet<std::string> stdStringSet2({"2", "3"});

    intSet1 -= intSet2;
    qStringSet1 -= qStringSet2;
    stdStringSet1 -= stdStringSet2;

    QCOMPARE(intSet1.size(), 2);
    QCOMPARE(intSet1.contains(1), true);
    QCOMPARE(intSet1.contains(2), false);
    QCOMPARE(intSet1.contains(3), false);
    QCOMPARE(intSet1.contains(4), true);
    QCOMPARE(intSet2.size(), 2);

    QCOMPARE(qStringSet1.size(), 2);
    QCOMPARE(qStringSet1.contains("1"), true);
    QCOMPARE(qStringSet1.contains("2"), false);
    QCOMPARE(qStringSet1.contains("3"), false);
    QCOMPARE(qStringSet1.contains("4"), true);
    QCOMPARE(qStringSet2.size(), 2);

    QCOMPARE(stdStringSet1.size(), 2);
    QCOMPARE(stdStringSet1.contains("1"), true);
    QCOMPARE(stdStringSet1.contains("2"), false);
    QCOMPARE(stdStringSet1.contains("3"), false);
    QCOMPARE(stdStringSet1.contains("4"), true);
    QCOMPARE(stdStringSet2.size(), 2);
}

void KMMSetTest::testValues()
{
    KMMSet<int> intSet1({1, 2, 4, 3});
    KMMSet<QString> qStringSet1(QStringList({"1", "2", "4", "3"}));
    KMMSet<std::string> stdStringSet1({"1", "2", "4", "3"});

    const auto intList = intSet1.values();
    const auto qStringList = qStringSet1.values();
    const auto stdStringList = stdStringSet1.values();

    QCOMPARE(intList.size(), 4);
    QCOMPARE(intList.contains(1), true);
    QCOMPARE(intList.contains(2), true);
    QCOMPARE(intList.contains(3), true);
    QCOMPARE(intList.contains(4), true);

    QCOMPARE(qStringList.size(), 4);
    QCOMPARE(qStringList.contains("1"), true);
    QCOMPARE(qStringList.contains("2"), true);
    QCOMPARE(qStringList.contains("3"), true);
    QCOMPARE(qStringList.contains("4"), true);

    QCOMPARE(stdStringList.size(), 4);
    QCOMPARE(stdStringList.contains("1"), true);
    QCOMPARE(stdStringList.contains("2"), true);
    QCOMPARE(stdStringList.contains("3"), true);
    QCOMPARE(stdStringList.contains("4"), true);
}
