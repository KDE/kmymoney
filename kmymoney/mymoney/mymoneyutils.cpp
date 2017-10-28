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


int MyMoneyTracer::m_indentLevel = 0;
int MyMoneyTracer::m_onoff = 0;

MyMoneyTracer::MyMoneyTracer(const char* name)
{
  if (m_onoff) {
    QRegExp exp("(.*)::(.*)");
    if (exp.indexIn(name) != -1) {
      m_className = exp.cap(1);
      m_memberName = exp.cap(2);
    } else {
      m_className = QString(name);
      m_memberName.clear();
    }
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent) << "ENTER: " << qPrintable(m_className) << "::" << qPrintable(m_memberName) << std::endl;
  }
  m_indentLevel += 2;
}

MyMoneyTracer::MyMoneyTracer(const QString& className, const QString& memberName) :
    m_className(className),
    m_memberName(memberName)
{
  if (m_onoff) {
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent) << "ENTER: " << qPrintable(m_className) << "::" << qPrintable(m_memberName) << std::endl;
  }
  m_indentLevel += 2;
}

MyMoneyTracer::~MyMoneyTracer()
{
  m_indentLevel -= 2;
  if (m_onoff) {
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent) << "LEAVE: " << qPrintable(m_className) << "::" << qPrintable(m_memberName) << std::endl;
  }
}

void MyMoneyTracer::printf(const char *format, ...) const
{
  if (m_onoff) {
    va_list args;
    va_start(args, format);
    QString indent;
    indent.fill(' ', m_indentLevel);
    std::cerr << qPrintable(indent);

    vfprintf(stderr, format, args);
    putc('\n', stderr);
    va_end(args);
  }
}

void MyMoneyTracer::onOff(int onOff)
{
  m_onoff = onOff;
}

void MyMoneyTracer::on()
{
  m_onoff = 1;
}

void MyMoneyTracer::off()
{
  m_onoff = 0;
}

QString dateToString(const QDate& date)
{
  if (!date.isNull() && date.isValid())
    return date.toString(Qt::ISODate);

  return QString();
}

QDate stringToDate(const QString& str)
{
  if (str.length()) {
    QDate date = QDate::fromString(str, Qt::ISODate);
    if (!date.isNull() && date.isValid())
      return date;
  }
  return QDate();
}

QString QStringEmpty(const QString& val)
{
  if (!val.isEmpty())
    return QString(val);

  return QString();
}

unsigned long extractId(const QString& txt)
{
  int pos;
  unsigned long rc = 0;

  pos = txt.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    rc = txt.mid(pos).toInt();
  }
  return rc;
}

