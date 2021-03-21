/*
    SPDX-FileCopyrightText: 2002-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneymoney-test.h"

#include <limits>
#include <cstdint>

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyMoneyTest;

#include <config-kmymoney.h>
#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

QTEST_GUILESS_MAIN(MyMoneyMoneyTest)

void MyMoneyMoneyTest::init()
{
    m_0 = new MyMoneyMoney(12, 100);
    m_1 = new MyMoneyMoney(-10, 100);
    m_2 = new MyMoneyMoney(2, 100);
    m_3 = new MyMoneyMoney(123, 1);
    m_4 = new MyMoneyMoney(1234, 1000);
    m_5 = new MyMoneyMoney(static_cast<qint64>(195883), 100000);
    m_6 = new MyMoneyMoney(1.247658435, 1000000000);

    MyMoneyMoney::setDecimalSeparator('.');
    MyMoneyMoney::setThousandSeparator(',');
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
}

void MyMoneyMoneyTest::cleanup()
{
    delete m_0;
    delete m_1;
    delete m_2;
    delete m_3;
    delete m_4;
    delete m_5;
    delete m_6;
}

void MyMoneyMoneyTest::testEmptyConstructor()
{
    //qDebug("testing %s", qPrintable(m_0->toString()));
    MyMoneyMoney *m = new MyMoneyMoney();
    QVERIFY(m->valueRef() == 0);
    QVERIFY(m->toString() == QString("0/1"));
    QVERIFY(m->valueRef().get_den() == 1);
    delete m;
}

void MyMoneyMoneyTest::testIntConstructor()
{
    //qDebug("Current value: %s",qPrintable( m_0->toString()) );
    //QVERIFY(m_0->valueRef().get_num() == 12);
    //QVERIFY(m_0->valueRef().get_den() == 100);
    QVERIFY(m_0->valueRef().get_num() == 3);
    QVERIFY(m_0->valueRef().get_den() == 25);

    QVERIFY(m_5->valueRef().get_num() == 195883);
    QVERIFY(m_5->valueRef().get_den() == 100000);

    MyMoneyMoney a(123, 10000);
    QVERIFY(a.valueRef().get_num() == 123);
    QVERIFY(a.valueRef().get_den() == 10000);
}

void MyMoneyMoneyTest::testAssignment()
{
    MyMoneyMoney *m = new MyMoneyMoney();
    *m = *m_1;
    //qDebug() << "Current value: "<< qPrintable( m->toString()) ;
    QVERIFY(m->valueRef().get_num() == -1);
    QVERIFY(m->valueRef().get_den() == 10);
    //QVERIFY(m->valueRef().get_num() == -10);
    //QVERIFY(m->valueRef().get_den() == 100);
#if 0
    *m = 0;
    QVERIFY(m->valueRef().get_num() == 0);
    QVERIFY(m->valueRef().get_den() == 100);

    *m = 777888999;
    QVERIFY(m->valueRef().get_num() == 777888999);
    QVERIFY(m->valueRef().get_den() == 100);

    *m = (int) - 5678;
    QVERIFY(m->valueRef().get_num() == -5678);
    QVERIFY(m->valueRef().get_den() == 100);

    *m = QString("-987");
    QVERIFY(m->valueRef().get_num() == -987);
    QVERIFY(m->valueRef().get_den() == 1);

    *m = QString("9998887776665554.44");
    QVERIFY(m->valueRef().get_num() == 999888777666555444LL);
    QVERIFY(m->valueRef().get_den() == 100);

    *m = QString("-99988877766655.444");
    QVERIFY(m->valueRef().get_num() == -99988877766655444LL);
    QVERIFY(m->valueRef().get_den() == 1000);

    *m = -666555444333222111LL;
    QVERIFY(m->valueRef().get_num() == -666555444333222111LL);
    QVERIFY(m->valueRef().get_den() == 100);
#endif
    delete m;
}

void MyMoneyMoneyTest::testStringConstructor()
{
    MyMoneyMoney *m1 = new MyMoneyMoney("-999666555444");

    mpz_class testnum = mpz_class("-999666555444");
    //qDebug("Created %s", qPrintable(m1->toString()));

    QVERIFY(m1->valueRef().get_num() == testnum);
    QVERIFY(m1->valueRef().get_den() == 1);

    testnum = mpz_class("444555666999");
    MyMoneyMoney *m2 = new MyMoneyMoney("4445556669.99");
    QVERIFY(m2->valueRef().get_num() == testnum);
    QVERIFY(m2->valueRef().get_den() == 100);

    delete m1;
    delete m2;

    //new tests
    m1 = new MyMoneyMoney("0.01");
    QVERIFY(m1->valueRef().get_num() == 1);
    QVERIFY(m1->valueRef().get_den() == 100);
    delete m1;

    m1 = new MyMoneyMoney("0.07");
    QVERIFY(m1->valueRef().get_num() == 7);
    QVERIFY(m1->valueRef().get_den() == 100);
    delete m1;

    m1 = new MyMoneyMoney("0.08");
    QVERIFY(m1->valueRef().get_num() == 2);
    QVERIFY(m1->valueRef().get_den() == 25);
    delete m1;

    m1 = new MyMoneyMoney(".");
    //qDebug("Created %s", qPrintable(m1->toString()));
    QVERIFY(m1->valueRef().get_num() == 0);
    QVERIFY(m1->valueRef().get_den() == 1);
    delete m1;

    m1 = new MyMoneyMoney("");
    QVERIFY(m1->valueRef().get_num() == 0);
    QVERIFY(m1->valueRef().get_den() == 1);
    delete m1;

    m1 = new MyMoneyMoney("1,123.");

    QVERIFY(m1->valueRef().get_num() == (1123));
    QVERIFY(m1->valueRef().get_den() == 1);
    delete m1;

    m1 = new MyMoneyMoney("123.1");
    QVERIFY(m1->valueRef().get_num() == (1231));
    QVERIFY(m1->valueRef().get_den() == 10);
    delete m1;

    m1 = new MyMoneyMoney("123.456");
    //qDebug("Created: %s", m1->valueRef().get_str().c_str());
    QVERIFY(m1->valueRef().get_num() == 15432);
    QVERIFY(m1->valueRef().get_den() == 125);
    //QVERIFY(m1->valueRef().get_num() == 123456);
    //QVERIFY(m1->valueRef().get_den() == 1000);
    delete m1;

    m1 = new MyMoneyMoney("12345/100");
    //qDebug("Created: %s", m1->valueRef().get_str().c_str());
    QVERIFY(m1->valueRef().get_num() == 2469);
    QVERIFY(m1->valueRef().get_den() == 20);
//  QVERIFY(m1->valueRef().get_num() == (12345));
//  QVERIFY(m1->valueRef().get_den() == 100);
    delete m1;

    m1 = new MyMoneyMoney("-54321/100");
//  qDebug("Created: %s", m1->valueRef().get_str().c_str());
    QVERIFY(m1->valueRef().get_num() == (-54321));
    QVERIFY(m1->valueRef().get_den() == 100);
    delete m1;


    MyMoneyMoney::setDecimalSeparator(',');
    MyMoneyMoney::setThousandSeparator('.');
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::ParensAround);
    m1 = new MyMoneyMoney("x1.234,567 EUR");
    QVERIFY(m1->valueRef().get_num() == (1234567));
    QVERIFY(m1->valueRef().get_den() == 1000);
    delete m1;

    m1 = new MyMoneyMoney("x(1.234,567) EUR");
    QVERIFY(m1->valueRef().get_num() == (-1234567));
    QVERIFY(m1->valueRef().get_den() == 1000);
    delete m1;

    m1 = new MyMoneyMoney("1 5/8");
    QVERIFY(m1->valueRef().get_num() == (13));
    QVERIFY(m1->valueRef().get_den() == 8);
    delete m1;

    m1 = new MyMoneyMoney("09");
    QVERIFY(m1->valueRef().get_num() == (9));
    QVERIFY(m1->valueRef().get_den() == 1);
    delete m1;
}

void MyMoneyMoneyTest::testConvert()
{
    MyMoneyMoney a(123.456);
    MyMoneyMoney b(a.convertDenominator(100));
    QVERIFY(b == MyMoneyMoney(12346, 100));

    a = QString("-123.456");

    b = a.convert(100);
    QVERIFY(b == MyMoneyMoney(-12346, 100));


    a = QString("123.1");
    b = a.convert(100);
    QVERIFY(b == MyMoneyMoney(12310, 100));

    a = QString("-73010.28");
    b = QString("1.95583");

    QVERIFY((a * b).convert(100) == QString("-142795.70"));
    QVERIFY((a * b).convert(100) == QString("-14279570/100"));
// QVERIFY((a * b).convert(100).toString() == QString("-14279570/100"));

    a = QString("-142795.69");
    QVERIFY((a / b).convert(100) == QString("-73010.28"));
    //QVERIFY((a / b).convert(100).toString() == QString("-7301028/100"));
}

void MyMoneyMoneyTest::testEquality()
{
    QVERIFY(*m_1 == *m_1);
    QVERIFY(!(*m_1 == *m_0));

    MyMoneyMoney m1(std::int64_t(999666555444), 100);
    MyMoneyMoney m2(std::int64_t(999666555444), 100);
    QVERIFY(m1 == m2);

    MyMoneyMoney m3(std::int64_t(-999666555444), 100);
    MyMoneyMoney m4(std::int64_t(-999666555444), 100);
    QVERIFY(m3 == m4);

    MyMoneyMoney m5(1230, 100);
    MyMoneyMoney m6(123, 10);
    MyMoneyMoney m7(246, 20);

    QVERIFY(m5 == m6);
    QVERIFY(m5 == m7);

    QVERIFY(m5 == QString("369/30"));

    QVERIFY(MyMoneyMoney::autoCalc == MyMoneyMoney::autoCalc);


    MyMoneyMoney mm1, mm2;
    mm1 = QLatin1String("-14279570/100");
    mm2 = QLatin1String("-1427957/10");
    QVERIFY(mm1 == mm2);
    QVERIFY(mm1 == QLatin1String("-14279570/100"));

    mm1 = QLatin1String("-7301028/100");
    mm2 = QLatin1String("-1825257/25");
    QVERIFY(mm1 == mm2);
}

void MyMoneyMoneyTest::testInequality()
{
    QVERIFY(*m_1 != *m_0);
    QVERIFY(!(*m_1 != *m_1));

    MyMoneyMoney m1(std::int64_t(999666555444), 100);
    MyMoneyMoney m2(std::int64_t(-999666555444), 100);
    QVERIFY(m1 != m2);

    MyMoneyMoney m3(std::int64_t(-999666555444), 100);
    MyMoneyMoney m4(std::int64_t(999666555444), 100);
    QVERIFY(m3 != m4);

    QVERIFY(m4 != QString("999666555444"));

    QVERIFY(MyMoneyMoney::autoCalc != MyMoneyMoney(1, 100));
    QVERIFY(MyMoneyMoney(1, 100) != MyMoneyMoney::autoCalc);
}


void MyMoneyMoneyTest::testAddition()
{
    QVERIFY(*m_0 + *m_1 == *m_2);

    MyMoneyMoney m1(100, 100);

    // QVERIFY((m1 + 50) == MyMoneyMoney(51,1));
    // QVERIFY((m1 + 1000000000) == MyMoneyMoney(1000000001,1));
    // QVERIFY((m1 + -50) == MyMoneyMoney(-49,1));

    QVERIFY((m1 += *m_0) == MyMoneyMoney(112, 100));
    // QVERIFY((m1 += -12) == MyMoneyMoney(100));

    // m1++;
    // QVERIFY(m1 == MyMoneyMoney(101));
    // QVERIFY((++m1) == MyMoneyMoney(102));

    m1 = QString("123.20");
    MyMoneyMoney m2(40, 1000);
    QVERIFY((m1 + m2) == QString("123.24"));

    m1 += m2;
    //FIXME check after deciding about normalization
    QVERIFY(m1.valueRef().get_num() == 3081);
    QVERIFY(m1.valueRef().get_den() == 25);
    //QVERIFY(m1.valueRef().get_num() == 123240);
    //QVERIFY(m1.valueRef().get_den() == 1000);
}

void MyMoneyMoneyTest::testSubtraction()
{
    QVERIFY(*m_2 - *m_1 == *m_0);

    MyMoneyMoney m1(100, 100);

    // QVERIFY((m1-50) == MyMoneyMoney(-49,1));
    // QVERIFY((m1-1000000000) == MyMoneyMoney(-999999999,1));
    // QVERIFY((m1 - -50) == MyMoneyMoney(51,1));

    QVERIFY((m1 -= *m_0) == MyMoneyMoney(88, 100));
    // QVERIFY((m1 -= -12) == MyMoneyMoney(100));

    // m1--;
    // QVERIFY(m1 == MyMoneyMoney(99));
    // QVERIFY((--m1) == MyMoneyMoney(98));

    m1 = QString("123.20");
    MyMoneyMoney m2(1, 5);
    QVERIFY((m1 - m2) == MyMoneyMoney(123, 1));

    m1 -= m2;
    //FIXME check after deciding about normalization
    QVERIFY(m1.valueRef().get_num() == 123);
    QVERIFY(m1.valueRef().get_den() == 1);
    //QVERIFY(m1.valueRef().get_num() == 12300);
    //QVERIFY(m1.valueRef().get_den() == 100);

}

void MyMoneyMoneyTest::testMultiplication()
{
    MyMoneyMoney m1(100, 1);

    QVERIFY((m1 * MyMoneyMoney(50, 1)) == MyMoneyMoney(5000, 1));
    QVERIFY((m1 * MyMoneyMoney(10000000, 1)) == MyMoneyMoney(1000000000, 1));
    QVERIFY((m1 *(*m_0)) == MyMoneyMoney(1200, 100));

    MyMoneyMoney m2(QString("-73010.28"));
    m1 = QString("1.95583");
    QVERIFY((m1 * m2) == QString("-142795.6959324"));

    MyMoneyMoney m3(100, 1);
    QVERIFY((m3 * 10) == MyMoneyMoney(1000, 1));
    //QVERIFY( (m3 *= (*m_0))  == MyMoneyMoney(1200));
    QVERIFY((m3 *= (*m_0))  == MyMoneyMoney(1200, 100));
}

void MyMoneyMoneyTest::testDivision()
{
    MyMoneyMoney m1(100, 100);
    QVERIFY((m1 / MyMoneyMoney(50, 100)) == MyMoneyMoney(2, 1));

    MyMoneyMoney m2(QString("-142795.69"));
    m1 = QString("1.95583");
    QVERIFY((m2 / m1).convert(100000000) == QString("-73010.27696681"));

    MyMoneyMoney m3 = MyMoneyMoney() / MyMoneyMoney(100, 100);
    QVERIFY(m3.valueRef().get_num() == 0);
    QVERIFY(m3.valueRef().get_den() != 0);
}

void MyMoneyMoneyTest::testSetDecimalSeparator()
{
    MyMoneyMoney m1(100000, 100);
    MyMoneyMoney m2(200000, 100);

    QVERIFY(m1.formatMoney("", 2) == QString("1,000.00"));
    QVERIFY(MyMoneyMoney::decimalSeparator() == '.');

    MyMoneyMoney::setDecimalSeparator(':');
    QVERIFY(m1.formatMoney("", 2) == QString("1,000:00"));
    QVERIFY(m2.formatMoney("", 2) == QString("2,000:00"));

    QVERIFY(MyMoneyMoney::decimalSeparator() == ':');
}

void MyMoneyMoneyTest::testSetThousandSeparator()
{
    MyMoneyMoney m1(100000, 100);
    MyMoneyMoney m2(200000, 100);

    QVERIFY(m1.formatMoney("", 2) == QString("1,000.00"));
    QVERIFY(MyMoneyMoney::thousandSeparator() == ',');

    MyMoneyMoney::setThousandSeparator(':');
    QVERIFY(m1.formatMoney("", 2) == QString("1:000.00"));
    QVERIFY(m2.formatMoney("", 2) == QString("2:000.00"));

    QVERIFY(MyMoneyMoney::thousandSeparator() == ':');
}

void MyMoneyMoneyTest::testFormatMoney()
{
    qDebug() << "Value:" << qPrintable(m_0->toString());
    qDebug() << "Converted: " << qPrintable(m_0->convert(100).toString());
    qDebug() << " Formatted: " << qPrintable(m_0->formatMoney("", 2));

    QVERIFY(m_0->formatMoney("", 2) == QString("0.12"));
    QVERIFY(m_1->formatMoney("", 2) == QString("-0.10"));

    MyMoneyMoney m1(10099, 100);
    qDebug() << "Value:" << qPrintable(m1.toString());
    qDebug() << "Converted: " << qPrintable(m1.convert(100).toString());
    qDebug() << " Formatted: " << qPrintable(m1.formatMoney("", 2));
    QVERIFY(m1.formatMoney("", 2) == QString("100.99"));

    m1 = MyMoneyMoney(100, 1);
    qDebug() << "Value:" << qPrintable(m1.toString());
    qDebug() << "Converted: " << qPrintable(m1.convert(100).toString());
    qDebug() << " Formatted: " << qPrintable(m1.formatMoney("", 2));
    QVERIFY(m1.formatMoney("", 2) == QString("100.00"));
    QVERIFY(m1.formatMoney("", -1) == QString("100"));

    MyMoneyMoney mTemp(100099, 100);

    m1 = m1 * MyMoneyMoney(10, 1);

    QVERIFY(m1 == MyMoneyMoney(1000, 1));

    QVERIFY(m1.formatMoney("", 2) == QString("1,000.00"));
    QVERIFY(m1.formatMoney("", -1) == QString("1,000"));
    QVERIFY(m1.formatMoney("", -1, false) == QString("1000"));
    QVERIFY(m1.formatMoney("", 3, false) == QString("1000.000"));

    m1 = MyMoneyMoney(std::numeric_limits<std::int64_t>::max(), 100);

    QVERIFY(m1.formatMoney("", 2) == QString("92,233,720,368,547,758.07"));
    QVERIFY(m1.formatMoney(100) == QString("92,233,720,368,547,758.07"));
    QVERIFY(m1.formatMoney("", 2, false) == QString("92233720368547758.07"));
    QVERIFY(m1.formatMoney(100, false) == QString("92233720368547758.07"));

    m1 = MyMoneyMoney(std::numeric_limits<std::int64_t>::min(), 100);
    QVERIFY(m1.formatMoney("", 2) == QString("-92,233,720,368,547,758.08"));
    QVERIFY(m1.formatMoney(100) == QString("-92,233,720,368,547,758.08"));
    QVERIFY(m1.formatMoney("", 2, false) == QString("-92233720368547758.08"));
    QVERIFY(m1.formatMoney(100, false) == QString("-92233720368547758.08"));

    // make sure we support numbers that need more than 64 bit
    m1 = MyMoneyMoney(321, 100) * MyMoneyMoney(std::numeric_limits<std::int64_t>::max(), 100);
    QVERIFY(m1.formatMoney("", 2) == QString("296,070,242,383,038,303.40"));
    QVERIFY(m1.formatMoney("", 4) == QString("296,070,242,383,038,303.4047"));
    QVERIFY(m1.formatMoney("", 6) == QString("296,070,242,383,038,303.404700"));


    m1 = MyMoneyMoney(1, 5);
    QVERIFY(m1.formatMoney("", 2) == QString("0.20"));
    QVERIFY(m1.formatMoney(1000) == QString("0.200"));
    QVERIFY(m1.formatMoney(100) == QString("0.20"));
    QVERIFY(m1.formatMoney(10) == QString("0.2"));

    m1 = MyMoneyMoney(13333, 5000);

    QVERIFY(m1.formatMoney("", 10) == QString("2.6666000000"));

    m1 = MyMoneyMoney(-1404, 100);
    QVERIFY(m1.formatMoney("", -1) == QString("-14.04"));
}

void MyMoneyMoneyTest::testRelation()
{
    MyMoneyMoney m1(100, 100);
    MyMoneyMoney m2(50, 100);
    MyMoneyMoney m3(100, 100);

    // tests with same denominator
    QVERIFY(m1 > m2);
    QVERIFY(m2 < m1);

    QVERIFY(m1 <= m3);
    QVERIFY(m3 >= m1);
    QVERIFY(m1 <= m1);
    QVERIFY(m3 >= m3);

    // tests with different denominator
    m1 = QString("1/8");
    m2 = QString("1/7");
    QVERIFY(m1 < m2);
    QVERIFY(m2 > m1);

    m2 = QString("-1/7");
    QVERIFY(m2 < m1);
    QVERIFY(m1 > m2);
    QVERIFY(m1 >= m2);
    QVERIFY(m2 <= m1);

    m1 = QString("-2/14");
    QVERIFY(m1 >= m2);
    QVERIFY(m1 <= m2);

}

void MyMoneyMoneyTest::testUnaryMinus()
{
    MyMoneyMoney m1(100, 100);
    MyMoneyMoney m2;

    m2 = -m1;

    QVERIFY(m1 == MyMoneyMoney(100, 100));
    QVERIFY(m2 == MyMoneyMoney(-100, 100));
}

void MyMoneyMoneyTest::testDoubleConstructor()
{
    QVERIFY(m_6->valueRef().get_num() == 249531687);
    QVERIFY(m_6->valueRef().get_den() == 200000000);

    for (int i = -123456; i < 123456; ++i) {
        // int i = -123456;
        double d = i;
        MyMoneyMoney r(i, 100);
        d /= 100;
        MyMoneyMoney t(d, 100);
        MyMoneyMoney s(i);
        QVERIFY(t == r);
        QVERIFY(i == s.toDouble());
    }
}

void MyMoneyMoneyTest::testAbsoluteFunction()
{
    MyMoneyMoney m1(-100, 100);
    MyMoneyMoney m2(100, 100);

    QVERIFY(m2.abs() == MyMoneyMoney(100, 100));
    QVERIFY(m1.abs() == MyMoneyMoney(100, 100));
}

void MyMoneyMoneyTest::testToString()
{
    MyMoneyMoney m1(-100, 100);
    MyMoneyMoney m2(1234, 100);
    MyMoneyMoney m3;

    QCOMPARE(m1.toString(), QStringLiteral("-1/1"));
    QCOMPARE(m2.toString(), QStringLiteral("617/50"));
    QCOMPARE(m3.toString(), QStringLiteral("0/1"));
}

void MyMoneyMoneyTest::testNegativeSignPos()
{
    MyMoneyMoney m("-123456/100");

    eMyMoney::Money::signPosition pos = MyMoneyMoney::negativeMonetarySignPosition();

    MyMoneyMoney::setNegativeSpaceSeparatesSymbol(true);
    MyMoneyMoney::setNegativePrefixCurrencySymbol(false);
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(1,234.56 CUR)"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("-1,234.56 CUR"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56- CUR"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56 -CUR"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56 CUR-"));

    MyMoneyMoney::setNegativePrefixCurrencySymbol(true);
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(CUR 1,234.56)"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR -1,234.56"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR 1,234.56-"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("-CUR 1,234.56"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR- 1,234.56"));

    MyMoneyMoney::setNegativeSpaceSeparatesSymbol(false);
    MyMoneyMoney::setNegativePrefixCurrencySymbol(false);
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(1,234.56CUR)"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("-1,234.56CUR"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56-CUR"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56-CUR"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56CUR-"));

    MyMoneyMoney::setNegativePrefixCurrencySymbol(true);
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(CUR1,234.56)"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR-1,234.56"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR1,234.56-"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("-CUR1,234.56"));
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR-1,234.56"));

    MyMoneyMoney::setNegativeMonetarySignPosition(pos);
}

void MyMoneyMoneyTest::testPositiveSignPos()
{
    MyMoneyMoney m("123456/100");

    eMyMoney::Money::signPosition pos = MyMoneyMoney::positiveMonetarySignPosition();

    MyMoneyMoney::setPositiveSpaceSeparatesSymbol(true);
    MyMoneyMoney::setPositivePrefixCurrencySymbol(false);
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(1,234.56 CUR)"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56 CUR"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56 CUR"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56 CUR"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56 CUR"));

    MyMoneyMoney::setPositivePrefixCurrencySymbol(true);
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(CUR 1,234.56)"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR 1,234.56"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR 1,234.56"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR 1,234.56"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR 1,234.56"));

    MyMoneyMoney::setPositiveSpaceSeparatesSymbol(false);
    MyMoneyMoney::setPositivePrefixCurrencySymbol(false);
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(1,234.56CUR)"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56CUR"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56CUR"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56CUR"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("1,234.56CUR"));

    MyMoneyMoney::setPositivePrefixCurrencySymbol(true);
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::ParensAround);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("(CUR1,234.56)"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR1,234.56"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedQuantityAndSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR1,234.56"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::PreceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR1,234.56"));
    MyMoneyMoney::setPositiveMonetarySignPosition(eMyMoney::Money::SucceedSymbol);
    QCOMPARE(m.formatMoney("CUR", 2), QStringLiteral("CUR1,234.56"));


    MyMoneyMoney::setPositiveMonetarySignPosition(pos);
}

void MyMoneyMoneyTest::testNegativeStringConstructor()
{
    MyMoneyMoney *m1;
    MyMoneyMoney::setDecimalSeparator(',');
    MyMoneyMoney::setThousandSeparator('.');
    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::ParensAround);
    m1 = new MyMoneyMoney("x(1.234,567) EUR");

    QVERIFY(m1->valueRef().get_num() == (-1234567));
    QVERIFY(m1->valueRef().get_den() == 1000);
    delete m1;

    MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
    m1 = new MyMoneyMoney("x1.234,567- EUR");
    //qDebug("Created: %s", m1->valueRef().get_str().c_str());

    QVERIFY(m1->valueRef().get_num() == (-1234567));
    QVERIFY(m1->valueRef().get_den() == 1000);
    delete m1;

    m1 = new MyMoneyMoney("x1.234,567 -EUR");
    QVERIFY(m1->valueRef().get_num() == (-1234567));
    QVERIFY(m1->valueRef().get_den() == 1000);
    delete m1;

    m1 = new MyMoneyMoney("-1.234,567 EUR");
    QVERIFY(m1->valueRef().get_num() == (-1234567));
    QVERIFY(m1->valueRef().get_den() == 1000);
    delete m1;
}

void MyMoneyMoneyTest::testReduce()
{
    MyMoneyMoney a(36488100, 1267390000);
    MyMoneyMoney b(-a);

    a = a.reduce();
    QVERIFY(a.valueRef().get_num() == 364881);
    QVERIFY(a.valueRef().get_den() == 12673900);

    b = b.reduce();
    QVERIFY(b.valueRef().get_num() == -364881);
    QVERIFY(b.valueRef().get_den() == 12673900);
}

void MyMoneyMoneyTest::testZeroDenominator()
{
    QVERIFY_EXCEPTION_THROWN(MyMoneyMoney m((int)1, 0), MyMoneyException);
    QVERIFY_EXCEPTION_THROWN(MyMoneyMoney m((signed64)1, 0), MyMoneyException);
}
