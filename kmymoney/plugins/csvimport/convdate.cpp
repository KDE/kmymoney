/***************************************************************************
                                        convDate.cpp
                                      -------------------
    begin                        : Sat Jan 01 2010
    copyright                : (C) 2010 by Allan Anderson
    email                      : agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "convdate.h"

#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QtCore/QRegExp>

ConvertDate::ConvertDate()
{
  m_dateFormatIndex = 0;
}

ConvertDate::~ConvertDate()
{
}

QDate ConvertDate::convertDate(const QString& txt)
{
  QString aYear;
  QString aMonth;
  QString aDay;
  QString aFormat;
  static QString dat;
  QDate  aDate;

  QString dateFormatString = stringFormat();

  QRegExp rx("[. -]");//                           replace date field separators '.' ' ' '-'
  QString buffer = txt;
  buffer = buffer.replace(rx, QString('/'));//     ....with '/'
  int count = buffer.count('/', Qt::CaseSensitive);
  if (count == 0) { //                              no separators so use QDate()
    QDate result = QDate::fromString(buffer, dateFormatString);
    if (result.year() < 1950) {
      result = QDate();
    }
    return result;
  }

  QStringList dateSplit = buffer.split('/');
  if (dateSplit.count() != 3) { //                  not a valid date
    return QDate();
  }
  switch (m_dateFormatIndex) {
    case(0):    //                                 %y %m %d
      aYear =  dateSplit[0];
      aMonth = dateSplit[1];
      aDay =   dateSplit[2];
      break;
    case(1):    //                                 %m %d %y
      aMonth = dateSplit[0];
      aDay =   dateSplit[1];
      aYear =  dateSplit[2];
      break;
    case(2):    //                                 %d %m %y
      aDay =   dateSplit[0];
      aMonth = dateSplit[1];
      aYear =  dateSplit[2];
      break;
    default:
      qDebug("ConvertDate - not a valid date format");
  }
//                                                 Check year
  if (aYear.length() == 2) {  //                    2 digits
    if ((aYear.toInt() >= 0) && (aYear.toInt() < 50)) {
      aYear = "20" + aYear;//                      take year to be 2000-2049
    } else if ((aYear.toInt() >= 50) && (aYear.toInt() <= 99))
      aYear = "19" + aYear;//                      take year to be 1950-1999
  } else if (aYear.length() == 4) {
    if ((aYear.toInt() < 1950) || (aYear.toInt() > 2050)) { //  not a valid year
      return QDate();
    }
  } else {
    return QDate();//                              2 or 4 digits for a valid year
  }
  // only years 1950-2050 valid
  //                                               check day
  if (aDay.length() == 1)
    aDay = '0' + aDay;//                           add a leading '0'
  if ((aDay < "0") || (aDay > "31") //              check day value
      || (aDay.length()  < 1) || (aDay.length()  > 2)) {
    return QDate();//                              not a valid day
  }
//                                                 check month
  if (aMonth.length() == 1) {
    aMonth = '0' + aMonth;
    aFormat = "MM";
  } else if (aMonth.length() == 2) { //             assume numeric
    bool datefound = ((aMonth > "0") && (aMonth < "13"));
    if (!datefound) {
      return QDate();//                            not a valid day
    }
    aFormat = "MM";//                              aMonth is numeric
  }

  else//                                           aMonth NOT numeric
    if (aMonth.length() == 3)
      aFormat = "MMM";
    else
      aFormat = "MMMM";
  QString dateFormat;
  switch (m_dateFormatIndex) {
    case(0):    //                                 %y %m %d
      dateFormat = "yyyy" + aFormat + "dd";
      dat = aYear + aMonth + aDay;
      break;
    case(1):    //                                 %m %d %y
      dateFormat = aFormat + "dd" + "yyyy";
      dat = aMonth + aDay + aYear;
      break;
    case(2):    //                                 %d %m %y
      dateFormat =  "dd" + aFormat + "yyyy";
      dat = aDay + aMonth + aYear;
      break;
    default:
      qDebug("ConvertDate - date format unknown");
  }
  aDate = QDate::fromString(dat, dateFormat);
  return aDate;
}

void ConvertDate::dateFormatSelected(int dateFormat)
{
  if (dateFormat == -1) {  //                       need UK, USA or ISO date format
    return;//                                      no selection made
  }
  m_dateFormatIndex = dateFormat;
}

void ConvertDate::setDateFormatIndex(int index)
{
  m_dateFormatIndex = index;
}

QString ConvertDate::stringFormat()
{
  QString dateFormatString;
  switch (m_dateFormatIndex) {
    case(0):    //                                 %y %m %d
      dateFormatString = "yyyyMMdd";
      break;
    case(1):    //                                 %m %d %y
      dateFormatString = "MMddyyyy";
      break;
    case(2):    //                                 %d %m %y
      dateFormatString =  "ddMMyyyy";
      break;
    default:
      qDebug("ConvertDate - date format unknown");
  }
  return dateFormatString;
}
