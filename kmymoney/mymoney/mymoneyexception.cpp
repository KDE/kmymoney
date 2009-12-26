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

MyMoneyException::MyMoneyException(const QString& msg, const QString& file, const unsigned long line)
{
  // qDebug("MyMoneyException(%s,%s,%ul)", qPrintable(msg), qPrintable(file), line);
  m_msg = msg;
  m_file = file;
  m_line = line;
}

MyMoneyException::~MyMoneyException()
{
}
