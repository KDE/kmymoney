/***************************************************************************
                          mymoneyexception.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
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

#include "mymoneyexception.h"

class MyMoneyExceptionPrivate
{
public:
  MyMoneyExceptionPrivate()
  {
  }

  /**
    * This member variable holds the message
    */
  QString m_msg;

  /**
    * This member variable holds the filename
    */
  QString m_file;

  /**
    * This member variable holds the line number
    */
  unsigned long m_line;
};

MyMoneyException::MyMoneyException() :
  d_ptr(new MyMoneyExceptionPrivate)
{
}

MyMoneyException::MyMoneyException(const QString& msg,
                                   const QString& file,
                                   const unsigned long line) :
  d_ptr(new MyMoneyExceptionPrivate)
{
  Q_D(MyMoneyException);
  // qDebug("MyMoneyException(%s,%s,%ul)", qPrintable(msg), qPrintable(file), line);
  d->m_msg = msg;
  d->m_file = file;
  d->m_line = line;
}

MyMoneyException::MyMoneyException(const MyMoneyException& other) :
  d_ptr(new MyMoneyExceptionPrivate(*other.d_func()))
{
}

MyMoneyException::~MyMoneyException()
{
  Q_D(MyMoneyException);
  delete d;
}

QString MyMoneyException::what() const
{
  Q_D(const MyMoneyException);
  return d->m_msg;
}

QString MyMoneyException::file() const
{
  Q_D(const MyMoneyException);
  return d->m_file;
}

unsigned long MyMoneyException::line() const
{
  Q_D(const MyMoneyException);
  return d->m_line;
}
