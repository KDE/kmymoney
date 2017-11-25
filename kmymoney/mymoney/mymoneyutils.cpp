/***************************************************************************
                          mymoneyutils.cpp  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyutils.h"

#include <iostream>

#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"

#include <cstdio>
#include <cstdarg>

#include <QRegExp>
#include <QDate>

QString MyMoneyUtils::getFileExtension(QString strFileName)
{
  QString strTemp;
  if (!strFileName.isEmpty()) {
    //find last . delminator
    int nLoc = strFileName.lastIndexOf('.');
    if (nLoc != -1) {
      strTemp = strFileName.right(strFileName.length() - (nLoc + 1));
      return strTemp.toUpper();
    }
  }
  return strTemp;
}

QString MyMoneyUtils::formatMoney(const MyMoneyMoney& val,
                                  const MyMoneyAccount& acc,
                                  const MyMoneySecurity& sec,
                                  bool showThousandSeparator)
{
  return val.formatMoney(sec.tradingSymbol(),
                         val.denomToPrec(acc.fraction()),
                         showThousandSeparator);
}

QString MyMoneyUtils::formatMoney(const MyMoneyMoney& val,
                                  const MyMoneySecurity& sec,
                                  bool showThousandSeparator)
{
  return val.formatMoney(sec.tradingSymbol(),
                         val.denomToPrec(sec.smallestAccountFraction()),
                         showThousandSeparator);
}

QString MyMoneyUtils::dateToString(const QDate& date)
{
  if (!date.isNull() && date.isValid())
    return date.toString(Qt::ISODate);

  return QString();
}

QDate MyMoneyUtils::stringToDate(const QString& str)
{
  if (str.length()) {
    QDate date = QDate::fromString(str, Qt::ISODate);
    if (!date.isNull() && date.isValid())
      return date;
  }
  return QDate();
}

QString MyMoneyUtils::QStringEmpty(const QString& val)
{
  if (!val.isEmpty())
    return QString(val);

  return QString();
}

unsigned long MyMoneyUtils::extractId(const QString& txt)
{
  int pos;
  unsigned long rc = 0;

  pos = txt.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    rc = txt.mid(pos).toInt();
  }
  return rc;
}

