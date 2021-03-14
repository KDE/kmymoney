/*
    SPDX-FileCopyrightText: 2010-2012 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "csvdate-test.h"

#include <QTest>

#include "../convdate.h"

QTEST_GUILESS_MAIN(CsvDateTest);

void CsvDateTest::init()
{
  m_convert = new ConvertDate;
}

void CsvDateTest::cleanup()
{
  delete m_convert;
}

void CsvDateTest::testConvertDate()
{
  m_convert->setDateFormatIndex(DateFormat::YearMonthDay);  //           ISO date format

  QVERIFY(m_convert->convertDate("2001-11-30") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("20011130") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("2001-11-30-09.32.35") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("08.00.00 2001-11-30") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("2001-11-30-14.52.10") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("2001-11-30 11:08:50") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("2001-11-30-07.03") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("2001-11-30:06.35 AM") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("20011130 020100") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("11-30-2001") == QDate());
  QVERIFY(m_convert->convertDate("11302001") == QDate());

  m_convert->setDateFormatIndex(DateFormat::MonthDayYear);  //           US date format

  QVERIFY(m_convert->convertDate("2001-11-30") == QDate());
  QVERIFY(m_convert->convertDate("20011130") == QDate());
  QVERIFY(m_convert->convertDate("11-30-2001") == QDate(2001, 11, 30));
  QVERIFY(m_convert->convertDate("11302001") == QDate(2001, 11, 30));

  m_convert->setDateFormatIndex(DateFormat::DayMonthYear);  //             UK/EU date format;

  QVERIFY(m_convert->convertDate("13/09/81") == QDate(1981, 9, 13));
  QVERIFY(m_convert->convertDate("13/09/01") == QDate(2001, 9, 13));
  QVERIFY(m_convert->convertDate("13-09-81") == QDate(1981, 9, 13));
  QVERIFY(m_convert->convertDate("13-09-01") == QDate(2001, 9, 13));
  QVERIFY(m_convert->convertDate(QString("25-" + QDate::longMonthName(1, QDate::DateFormat) + "-2000")) == QDate(2000, 1, 25));
  QVERIFY(m_convert->convertDate(QString("25-" + QDate::longMonthName(3, QDate::StandaloneFormat) + "-2000")) == QDate(2000, 3, 25));
  QVERIFY(m_convert->convertDate(QString("25-" + QDate::longMonthName(5) + "-2000")) == QDate(2000, 5, 25));
  QVERIFY(m_convert->convertDate(QString("25-" + QLocale().standaloneMonthName(7, QLocale::ShortFormat) + "-2000")) == QDate(2000, 7, 25));
  QVERIFY(m_convert->convertDate(QString("25-" + QLocale().standaloneMonthName(9, QLocale::LongFormat) + "-2000")) == QDate(2000, 9, 25));
  QVERIFY(m_convert->convertDate("13.09.81") == QDate(1981, 9, 13));
  QVERIFY(m_convert->convertDate("32/01/2000") == QDate()); // invalid day
  QVERIFY(m_convert->convertDate(QLatin1String("13-rubbishmonth-2000")) == QDate()); // invalid month
  QVERIFY(m_convert->convertDate("01/13/2000") == QDate()); // invalid month
  QVERIFY(m_convert->convertDate("01/12/200") == QDate()); // invalid year
  QVERIFY(m_convert->convertDate("") == QDate()); // empty date
  QVERIFY(m_convert->convertDate("31-1-2010") == QDate(2010, 1, 31)); // single digit month
  QVERIFY(m_convert->convertDate("13091981") == QDate(1981, 9, 13));
}

void CsvDateTest::testLastDayInFebruary()
{
  m_convert->setDateFormatIndex(DateFormat::YearMonthDay);  //           ISO date format

  QCOMPARE(m_convert->convertDate(QLatin1String("2018-02-30")).toString(), QDate(2018,2,28).toString());
  QCOMPARE(m_convert->convertDate(QLatin1String("2020-02-30")).toString(), QDate(2020,2,29).toString());
}
