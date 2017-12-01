/***************************************************************************
                            mymoneytestutils.h
                           -------------------
    copyright            : (C) 2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2014 by Christian Dávid

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTESTUTILS_H
#define MYMONEYTESTUTILS_H

#include "mymoneyexception.h"

#define unexpectedException(e) QFAIL(qPrintable(unexpectedExceptionString(e)));

QString unexpectedExceptionString(const MyMoneyException &e);

#endif // MYMONEYTESTUTILS_H
