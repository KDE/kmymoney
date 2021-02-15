/*
 * SPDX-FileCopyrightText: 2002-2003 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2002-2016 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
    //find last . deliminator
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

