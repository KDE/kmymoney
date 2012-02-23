/***************************************************************************
                        csvdatetest.cpp
                      -------------------
begin                : Sat Jan 01 2010
copyright            : (C) 2010 by Allan Anderson
email                : agander93@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "csvdatetest.h"

#include <QtTest/QtTest>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include <KLocalizedString>

#include "convdate.h"

QTEST_MAIN(CsvDateTest);

CsvDateTest::CsvDateTest()
{
}

void CsvDateTest::init()
{
  m_convert = new ConvertDate;
}

void CsvDateTest::cleanup()
{
  delete m_convert;
}

void CsvDateTest::testConstructor()
{
}

void CsvDateTest::testDefaultConstructor()
{
}

void CsvDateTest::testDateConvert()
{
  QString format = "dd/MM/yyyy";

  m_convert->setDateFormatIndex(2);  //             UK/EU date format;

  QVERIFY(m_convert->convertDate("13/09/81") ==
          QDate::fromString("13/09/1981", format));  //a = "13/09/81"

  QVERIFY(m_convert->convertDate("13/09/01") ==
          QDate::fromString("13/09/2001", format));  //b = "13/09/01"

  QVERIFY(m_convert->convertDate("13-09-81") ==
          QDate::fromString("13/09/1981", format));  //c = "13-09-81"

  QVERIFY(m_convert->convertDate("13-09-01") ==
          QDate::fromString("13/09/2001", format));  //d = "13-09-01"

  QVERIFY(m_convert->convertDate(QString("25-" + QDate::longMonthName(12) + "-2000")) ==
          QDate::fromString("25/12/2000", format));  //e = "25-December-2000"

  // TODO: change or fix this test since it's based on localization
  //which makes it fail depending on the translation
  // See qt-docs: "The month names will be localized according to the
  //system's locale settings."
  //QVERIFY(m_convert->convertDate(QString("5-" +
  //QDate::shortMonthName(11) + "-1999")) ==
  //QDate::fromString("5/11/1999", format));//f = "5-Nov-1999"

  QVERIFY(m_convert->convertDate("13.09.81") ==
          QDate::fromString("13/09/1981", format));  //g = "13.09.81"

  QVERIFY(m_convert->convertDate("32/01/2000") ==
          QDate());//                             h ="32/01/2000" invalid day

  QVERIFY(m_convert->convertDate(QLatin1String("13-rubbishmonth-2000")) ==
          QDate());//        i = "13-rubbishmonth-2000" invalid month

  QVERIFY(m_convert->convertDate("01/13/2000") ==
          QDate());//                  j = "01/13/2000"  invalid month

  QVERIFY(m_convert->convertDate("01/12/200") ==
          QDate());//                   k = "01/12/200"  invalid year

  QVERIFY(m_convert->convertDate("") ==
          QDate());//                            l = ""   empty date

  format = "ddMMyyyy";
  QVERIFY(m_convert->convertDate("31-1-2010") ==
          QDate::fromString("31012010", format));  //m = "31-1-2010" single digit month

// Now with no separators

  QVERIFY(m_convert->convertDate("13091981") ==
          QDate(1981, 9, 13));
}

void CsvDateTest::testDateConvertFormats()
{
  QString aDate = "2001-11-30";
  QString format = "yyyy/MM/dd";

  m_convert->setDateFormatIndex(0);  //           ISO date format

  QVERIFY(m_convert->convertDate(aDate) == QDate::fromString("2001/11/30", format));

  m_convert->setDateFormatIndex(1);  //           US date format

  QVERIFY(m_convert->convertDate(aDate) == QDate());

// Now with no separators

  aDate = "20011130";

  m_convert->setDateFormatIndex(0);  //           ISO date format

  QVERIFY(m_convert->convertDate(aDate) == QDate(2001, 11, 30));

  m_convert->setDateFormatIndex(1);  //           US date format

  QVERIFY(m_convert->convertDate(aDate) == QDate());

  aDate = "11-30-2001";
  format = "MM/dd/yyyy";

  m_convert->setDateFormatIndex(0);  //             ISO date format

  QVERIFY(m_convert->convertDate(aDate) == QDate());

  m_convert->setDateFormatIndex(1);  //           US date format

  QVERIFY(m_convert->convertDate(aDate) == QDate::fromString("11/30/2001", format));

  // Now with no separators

  aDate = "11302001";

  m_convert->setDateFormatIndex(0);  //             ISO date format

  QVERIFY(m_convert->convertDate(aDate) == QDate());

  m_convert->setDateFormatIndex(1);  //           US date format

  QVERIFY(m_convert->convertDate(aDate) == QDate(2001, 11, 30));
}

#include "csvdatetest.moc"
