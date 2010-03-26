/***************************************************************************
                          mymoneymoneytest.cpp
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// make sure, that this is defined before we even include any other header file
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS         // force definition of min and max values
#endif

#include "mymoneymoneytest.h"
#include "mymoneyexception.h"
#include <iostream>
#include <stdint.h>

// make sure, we have the correct suffix
#if SIZEOF_LONG == 8
#define LLCONST(a) a ## L
#else
#define LLCONST(a) a ## LL
#endif

MyMoneyMoneyTest::MyMoneyMoneyTest()
{
}


void MyMoneyMoneyTest::setUp()
{
  m_0 = new MyMoneyMoney(12);
  m_1 = new MyMoneyMoney(-10);
  m_2 = new MyMoneyMoney(2);
  m_3 = new MyMoneyMoney(123, 1);
  m_4 = new MyMoneyMoney(1234, 1000);
  m_5 = new MyMoneyMoney(195883, 100000);

  MyMoneyMoney::setDecimalSeparator('.');
  MyMoneyMoney::setThousandSeparator(',');
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::BeforeQuantityMoney);
}

void MyMoneyMoneyTest::tearDown()
{
  delete m_0;
  delete m_1;
  delete m_2;
  delete m_3;
  delete m_4;
  delete m_5;
}

void MyMoneyMoneyTest::testEmptyConstructor()
{
  MyMoneyMoney *m = new MyMoneyMoney();
  CPPUNIT_ASSERT(m->m_num == 0);
  CPPUNIT_ASSERT(m->m_denom == 1);
  delete m;
}

void MyMoneyMoneyTest::testIntConstructor()
{
  CPPUNIT_ASSERT(m_0->m_num == 12);
  CPPUNIT_ASSERT(m_0->m_denom == 100);

  MyMoneyMoney a(123, 10000);
  CPPUNIT_ASSERT(a.m_num == 123);
  CPPUNIT_ASSERT(a.m_denom == 10000);
}

void MyMoneyMoneyTest::testAssignment()
{
  MyMoneyMoney *m = new MyMoneyMoney();
  *m = *m_1;
  CPPUNIT_ASSERT(m->m_num == -10);
  CPPUNIT_ASSERT(m->m_denom == 100);
#if 0
  *m = 0;
  CPPUNIT_ASSERT(m->m_num == 0);
  CPPUNIT_ASSERT(m->m_denom == 100);

  *m = 777888999;
  CPPUNIT_ASSERT(m->m_num == 777888999);
  CPPUNIT_ASSERT(m->m_denom == 100);

  *m = (int) - 5678;
  CPPUNIT_ASSERT(m->m_num == -5678);
  CPPUNIT_ASSERT(m->m_denom == 100);

  *m = QString("-987");
  CPPUNIT_ASSERT(m->m_num == -987);
  CPPUNIT_ASSERT(m->m_denom == 1);

  *m = QString("9998887776665554.44");
  CPPUNIT_ASSERT(m->m_num == 999888777666555444LL);
  CPPUNIT_ASSERT(m->m_denom == 100);

  *m = QString("-99988877766655.444");
  CPPUNIT_ASSERT(m->m_num == -99988877766655444LL);
  CPPUNIT_ASSERT(m->m_denom == 1000);

  *m = -666555444333222111LL;
  CPPUNIT_ASSERT(m->m_num == -666555444333222111LL);
  CPPUNIT_ASSERT(m->m_denom == 100);
#endif
  delete m;
}

void MyMoneyMoneyTest::testStringConstructor()
{
  MyMoneyMoney *m1 = new MyMoneyMoney("-999666555444");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(-999666555444));
  CPPUNIT_ASSERT(m1->m_denom == 1);

  MyMoneyMoney *m2 = new MyMoneyMoney("4445556669.99");
  CPPUNIT_ASSERT(m2->m_num == LLCONST(444555666999));
  CPPUNIT_ASSERT(m2->m_denom == 100);

  delete m1;
  delete m2;

  m1 = new MyMoneyMoney("");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(0));
  CPPUNIT_ASSERT(m1->m_denom == 1);
  delete m1;

  m1 = new MyMoneyMoney("1,123.");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(1123));
  CPPUNIT_ASSERT(m1->m_denom == 1);
  delete m1;

  m1 = new MyMoneyMoney("123.1");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(1231));
  CPPUNIT_ASSERT(m1->m_denom == 10);
  delete m1;

  m1 = new MyMoneyMoney("123.456");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(123456));
  CPPUNIT_ASSERT(m1->m_denom == 1000);
  delete m1;

  m1 = new MyMoneyMoney("12345/100");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(12345));
  CPPUNIT_ASSERT(m1->m_denom == 100);
  delete m1;

  m1 = new MyMoneyMoney("-54321/100");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(-54321));
  CPPUNIT_ASSERT(m1->m_denom == 100);
  delete m1;

  MyMoneyMoney::setDecimalSeparator(',');
  MyMoneyMoney::setThousandSeparator('.');
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::ParensAround);
  m1 = new MyMoneyMoney("x1.234,567 EUR");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(1234567));
  CPPUNIT_ASSERT(m1->m_denom == 1000);
  delete m1;

  m1 = new MyMoneyMoney("x(1.234,567) EUR");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(-1234567));
  CPPUNIT_ASSERT(m1->m_denom == 1000);
  delete m1;

  m1 = new MyMoneyMoney("1 5/8");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(13));
  CPPUNIT_ASSERT(m1->m_denom == 8);
  delete m1;
}

void MyMoneyMoneyTest::testConvert()
{
  MyMoneyMoney a("123.456");
  MyMoneyMoney b = a.convert(100);
  CPPUNIT_ASSERT(b.m_num == 12346);
  CPPUNIT_ASSERT(b.m_denom == 100);
  a = QString("-123.456");
  b = a.convert(100);
  CPPUNIT_ASSERT(b.m_num == -12346);
  CPPUNIT_ASSERT(b.m_denom == 100);

  a = QString("123.1");
  b = a.convert(100);
  CPPUNIT_ASSERT(b.m_num == 12310);
  CPPUNIT_ASSERT(b.m_denom == 100);

  a = QString("-73010.28");
  b = QString("1.95583");
  CPPUNIT_ASSERT((a * b).convert(100) == QString("-142795.70"));

  a = QString("-142795.69");
  CPPUNIT_ASSERT((a / b).convert(100) == QString("-73010.28"));
}

void MyMoneyMoneyTest::testEquality()
{
  CPPUNIT_ASSERT(*m_1 == *m_1);
  CPPUNIT_ASSERT(!(*m_1 == *m_0));

  MyMoneyMoney m1(LLCONST(999666555444));
  MyMoneyMoney m2(LLCONST(999666555444));
  CPPUNIT_ASSERT(m1 == m2);

  MyMoneyMoney m3(LLCONST(-999666555444));
  MyMoneyMoney m4(LLCONST(-999666555444));
  CPPUNIT_ASSERT(m3 == m4);

  MyMoneyMoney m5(1230, 100);
  MyMoneyMoney m6(123, 10);
  MyMoneyMoney m7(246, 20);
  CPPUNIT_ASSERT(m5 == m6);
  CPPUNIT_ASSERT(m5 == m7);

  CPPUNIT_ASSERT(m5 == QString("369/30"));

  CPPUNIT_ASSERT(MyMoneyMoney::autoCalc == MyMoneyMoney::autoCalc);
}

void MyMoneyMoneyTest::testInequality()
{
  CPPUNIT_ASSERT(*m_1 != *m_0);
  CPPUNIT_ASSERT(!(*m_1 != *m_1));

  MyMoneyMoney m1(LLCONST(999666555444));
  MyMoneyMoney m2(LLCONST(-999666555444));
  CPPUNIT_ASSERT(m1 != m2);

  MyMoneyMoney m3(LLCONST(-999666555444));
  MyMoneyMoney m4(LLCONST(999666555444));
  CPPUNIT_ASSERT(m3 != m4);

  CPPUNIT_ASSERT(m4 != QString("999666555444"));

  CPPUNIT_ASSERT(MyMoneyMoney::autoCalc != MyMoneyMoney(1, 100));
  CPPUNIT_ASSERT(MyMoneyMoney(1, 100) != MyMoneyMoney::autoCalc);
}


void MyMoneyMoneyTest::testAddition()
{
  CPPUNIT_ASSERT(*m_0 + *m_1 == *m_2);

  MyMoneyMoney m1(100);

  // CPPUNIT_ASSERT((m1 + 50) == MyMoneyMoney(51,1));
  // CPPUNIT_ASSERT((m1 + 1000000000) == MyMoneyMoney(1000000001,1));
  // CPPUNIT_ASSERT((m1 + -50) == MyMoneyMoney(-49,1));

  CPPUNIT_ASSERT((m1 += *m_0) == MyMoneyMoney(112));
  // CPPUNIT_ASSERT((m1 += -12) == MyMoneyMoney(100));

  // m1++;
  // CPPUNIT_ASSERT(m1 == MyMoneyMoney(101));
  // CPPUNIT_ASSERT((++m1) == MyMoneyMoney(102));

  m1 = QString("123.20");
  MyMoneyMoney m2(40, 1000);
  CPPUNIT_ASSERT((m1 + m2) == QString("123.24"));

  m1 += m2;
  CPPUNIT_ASSERT(m1.m_num == 123240);
  CPPUNIT_ASSERT(m1.m_denom == 1000);
}

void MyMoneyMoneyTest::testSubtraction()
{
  CPPUNIT_ASSERT(*m_2 - *m_1 == *m_0);

  MyMoneyMoney m1(100);

  // CPPUNIT_ASSERT((m1-50) == MyMoneyMoney(-49,1));
  // CPPUNIT_ASSERT((m1-1000000000) == MyMoneyMoney(-999999999,1));
  // CPPUNIT_ASSERT((m1 - -50) == MyMoneyMoney(51,1));

  CPPUNIT_ASSERT((m1 -= *m_0) == MyMoneyMoney(88));
  // CPPUNIT_ASSERT((m1 -= -12) == MyMoneyMoney(100));

  // m1--;
  // CPPUNIT_ASSERT(m1 == MyMoneyMoney(99));
  // CPPUNIT_ASSERT((--m1) == MyMoneyMoney(98));

  m1 = QString("123.20");
  MyMoneyMoney m2(1, 5);
  CPPUNIT_ASSERT((m1 - m2) == MyMoneyMoney(123, 1));

  m1 -= m2;
  CPPUNIT_ASSERT(m1.m_num == 12300);
  CPPUNIT_ASSERT(m1.m_denom == 100);
}

void MyMoneyMoneyTest::testMultiplication()
{
  MyMoneyMoney m1(100, 1);

  CPPUNIT_ASSERT((m1 * MyMoneyMoney(50, 1)) == MyMoneyMoney(5000, 1));
  CPPUNIT_ASSERT((m1 * MyMoneyMoney(10000000, 1)) == MyMoneyMoney(1000000000, 1));
  CPPUNIT_ASSERT((m1 *(*m_0)) == MyMoneyMoney(1200));

  MyMoneyMoney m2(QString("-73010.28"));
  m1 = QString("1.95583");
  CPPUNIT_ASSERT((m1 * m2) == QString("-142795.6959324"));
}

void MyMoneyMoneyTest::testDivision()
{
  MyMoneyMoney m1(100);
  CPPUNIT_ASSERT((m1 / MyMoneyMoney(50)) == MyMoneyMoney(2, 1));

  MyMoneyMoney m2(QString("-142795.69"));
  m1 = QString("1.95583");
  CPPUNIT_ASSERT((m2 / m1).convert(100000000) == QString("-73010.27696681"));

  MyMoneyMoney m3 = MyMoneyMoney(0) / MyMoneyMoney(100);
  CPPUNIT_ASSERT(m3.m_num == 0);
  CPPUNIT_ASSERT(m3.m_denom != 0);
}

void MyMoneyMoneyTest::testSetDecimalSeparator()
{
  MyMoneyMoney m1(100000);
  MyMoneyMoney m2(200000);

  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("1,000.00"));
  CPPUNIT_ASSERT(MyMoneyMoney::decimalSeparator() == '.');

  MyMoneyMoney::setDecimalSeparator(':');
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("1,000:00"));
  CPPUNIT_ASSERT(m2.formatMoney("", 2) == QString("2,000:00"));

  CPPUNIT_ASSERT(MyMoneyMoney::decimalSeparator() == ':');
}

void MyMoneyMoneyTest::testSetThousandSeparator()
{
  MyMoneyMoney m1(100000);
  MyMoneyMoney m2(200000);

  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("1,000.00"));
  CPPUNIT_ASSERT(MyMoneyMoney::thousandSeparator() == ',');

  MyMoneyMoney::setThousandSeparator(':');
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("1:000.00"));
  CPPUNIT_ASSERT(m2.formatMoney("", 2) == QString("2:000.00"));

  CPPUNIT_ASSERT(MyMoneyMoney::thousandSeparator() == ':');
}

void MyMoneyMoneyTest::testFormatMoney()
{
  CPPUNIT_ASSERT(m_0->formatMoney("", 2) == QString("0.12"));
  CPPUNIT_ASSERT(m_1->formatMoney("", 2) == QString("-0.10"));

  MyMoneyMoney m1(10099);
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("100.99"));

  m1 = MyMoneyMoney(100, 1);
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("100.00"));
  CPPUNIT_ASSERT(m1.formatMoney("", -1) == QString("100"));

  m1 = m1 * 10;
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("1,000.00"));
  CPPUNIT_ASSERT(m1.formatMoney("", -1) == QString("1,000"));
  CPPUNIT_ASSERT(m1.formatMoney("", -1, false) == QString("1000"));
  CPPUNIT_ASSERT(m1.formatMoney("", 3, false) == QString("1000.000"));

  m1 = MyMoneyMoney(INT64_MAX, 100);
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("92,233,720,368,547,758.07"));
  CPPUNIT_ASSERT(m1.formatMoney(100) == QString("92,233,720,368,547,758.07"));
  CPPUNIT_ASSERT(m1.formatMoney("", 2, false) == QString("92233720368547758.07"));
  CPPUNIT_ASSERT(m1.formatMoney(100, false) == QString("92233720368547758.07"));

  m1 = MyMoneyMoney(INT64_MIN, 100);
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("-92,233,720,368,547,758.08"));
  CPPUNIT_ASSERT(m1.formatMoney(100) == QString("-92,233,720,368,547,758.08"));
  CPPUNIT_ASSERT(m1.formatMoney("", 2, false) == QString("-92233720368547758.08"));
  CPPUNIT_ASSERT(m1.formatMoney(100, false) == QString("-92233720368547758.08"));

  m1 = MyMoneyMoney(1, 5);
  CPPUNIT_ASSERT(m1.formatMoney("", 2) == QString("0.20"));
  CPPUNIT_ASSERT(m1.formatMoney(1000) == QString("0.200"));
  CPPUNIT_ASSERT(m1.formatMoney(100) == QString("0.20"));
  CPPUNIT_ASSERT(m1.formatMoney(10) == QString("0.2"));

  m1 = MyMoneyMoney(13333, 5000);
  CPPUNIT_ASSERT(m1.formatMoney("", 10) == QString("2.6666000000"));

  m1 = MyMoneyMoney(-1404, 100);
  CPPUNIT_ASSERT(m1.formatMoney("", -1) == QString("-14.04"));
}

void MyMoneyMoneyTest::testRelation()
{
  MyMoneyMoney m1(100);
  MyMoneyMoney m2(50);
  MyMoneyMoney m3(100);

  // tests with same denominator
  CPPUNIT_ASSERT(m1 > m2);
  CPPUNIT_ASSERT(m2 < m1);

  CPPUNIT_ASSERT(m1 <= m3);
  CPPUNIT_ASSERT(m3 >= m1);
  CPPUNIT_ASSERT(m1 <= m1);
  CPPUNIT_ASSERT(m3 >= m3);

  // tests with different denominator
  m1 = QString("1/8");
  m2 = QString("1/7");
  CPPUNIT_ASSERT(m1 < m2);
  CPPUNIT_ASSERT(m2 > m1);
  m2 = QString("-1/7");
  CPPUNIT_ASSERT(m2 < m1);
  CPPUNIT_ASSERT(m1 > m2);
  CPPUNIT_ASSERT(m1 >= m2);
  CPPUNIT_ASSERT(m2 <= m1);

  m1 = QString("-2/14");
  CPPUNIT_ASSERT(m1 >= m2);
  CPPUNIT_ASSERT(m1 <= m2);

}

void MyMoneyMoneyTest::testUnaryMinus()
{
  MyMoneyMoney m1(100);
  MyMoneyMoney m2;

  m2 = -m1;

  CPPUNIT_ASSERT(m1 == MyMoneyMoney(100));
  CPPUNIT_ASSERT(m2 == MyMoneyMoney(-100));
}

void MyMoneyMoneyTest::testDoubleConstructor()
{
  for (int i = -123456; i < 123456; ++i) {
    double d = i;
    MyMoneyMoney r(i);
    d /= 100;
    MyMoneyMoney t(d);
    CPPUNIT_ASSERT(t == r);
  }
}

void MyMoneyMoneyTest::testAbsoluteFunction()
{
  MyMoneyMoney m1(-100);
  MyMoneyMoney m2(100);

  CPPUNIT_ASSERT(m2.abs() == MyMoneyMoney(100));
  CPPUNIT_ASSERT(m1.abs() == MyMoneyMoney(100));
}

void MyMoneyMoneyTest::testToString()
{
  MyMoneyMoney m1(-100);
  MyMoneyMoney m2(1234);
  MyMoneyMoney m3;

  CPPUNIT_ASSERT(m1.toString() == QString("-100/100"));
  CPPUNIT_ASSERT(m2.toString() == QString("1234/100"));
  CPPUNIT_ASSERT(m3.toString() == QString("0/1"));
}

void MyMoneyMoneyTest::testNegativeSignPos(void)
{
  MyMoneyMoney m("-123456/100");

  MyMoneyMoney::signPosition pos = MyMoneyMoney::negativeMonetarySignPosition();

  MyMoneyMoney::setNegativePrefixCurrencySymbol(false);
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::ParensAround);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "(1,234.56) CUR");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::BeforeQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "-1,234.56 CUR");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::AfterQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "1,234.56- CUR");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::BeforeMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "1,234.56 -CUR");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::AfterMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "1,234.56 CUR-");

  MyMoneyMoney::setNegativePrefixCurrencySymbol(true);
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::ParensAround);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR (1,234.56)");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::BeforeQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR -1,234.56");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::AfterQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR 1,234.56-");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::BeforeMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "-CUR 1,234.56");
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::AfterMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR- 1,234.56");

  MyMoneyMoney::setNegativeMonetarySignPosition(pos);
}

void MyMoneyMoneyTest::testPositiveSignPos(void)
{
  MyMoneyMoney m("123456/100");

  MyMoneyMoney::signPosition pos = MyMoneyMoney::positiveMonetarySignPosition();

  MyMoneyMoney::setPositivePrefixCurrencySymbol(false);
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::ParensAround);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "(1,234.56) CUR");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::BeforeQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "1,234.56 CUR");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::AfterQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "1,234.56 CUR");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::BeforeMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "1,234.56 CUR");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::AfterMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "1,234.56 CUR");

  MyMoneyMoney::setPositivePrefixCurrencySymbol(true);
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::ParensAround);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR (1,234.56)");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::BeforeQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR 1,234.56");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::AfterQuantityMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR 1,234.56");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::BeforeMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR 1,234.56");
  MyMoneyMoney::setPositiveMonetarySignPosition(MyMoneyMoney::AfterMoney);
  CPPUNIT_ASSERT(m.formatMoney("CUR", 2) == "CUR 1,234.56");

  MyMoneyMoney::setPositiveMonetarySignPosition(pos);
}

void MyMoneyMoneyTest::testNegativeStringConstructor(void)
{
  MyMoneyMoney *m1;
  MyMoneyMoney::setDecimalSeparator(',');
  MyMoneyMoney::setThousandSeparator('.');
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::ParensAround);
  m1 = new MyMoneyMoney("x(1.234,567) EUR");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(-1234567));
  CPPUNIT_ASSERT(m1->m_denom == 1000);
  delete m1;
  MyMoneyMoney::setNegativeMonetarySignPosition(MyMoneyMoney::BeforeQuantityMoney);
  m1 = new MyMoneyMoney("x1.234,567- EUR");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(-1234567));
  CPPUNIT_ASSERT(m1->m_denom == 1000);
  delete m1;
  m1 = new MyMoneyMoney("x1.234,567 -EUR");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(-1234567));
  CPPUNIT_ASSERT(m1->m_denom == 1000);
  delete m1;
  m1 = new MyMoneyMoney("-1.234,567 EUR");
  CPPUNIT_ASSERT(m1->m_num == LLCONST(-1234567));
  CPPUNIT_ASSERT(m1->m_denom == 1000);
  delete m1;
}

void MyMoneyMoneyTest::testReduce(void)
{
  MyMoneyMoney a(36488100, 1267390000);
  MyMoneyMoney b(-a);

  a = a.reduce();
  CPPUNIT_ASSERT(a.m_num == 364881);
  CPPUNIT_ASSERT(a.m_denom == 12673900);

  b = b.reduce();
  CPPUNIT_ASSERT(b.m_num == -364881);
  CPPUNIT_ASSERT(b.m_denom == 12673900);
}

void MyMoneyMoneyTest::testZeroDenominator()
{
  try {
    MyMoneyMoney m((int)1, 0);
    CPPUNIT_FAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }

  try {
    MyMoneyMoney m((signed64)1, 0);
    CPPUNIT_FAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

