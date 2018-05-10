/***************************************************************************
                          mymoneyexception.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            Thomas Baumgart <ipwizard@users.sourceforge.net>
                         (C) 2017-2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYEXCEPTION_H
#define MYMONEYEXCEPTION_H

#define KMM_STRINGIFY(x) #x
#define KMM_TOSTRING(x) KMM_STRINGIFY(x)

#include <stdexcept>

/**
  * @file
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */

/**
 * @def The MYMONEYEXCEPTION(exceptionMessage) define
 * This is the preferred constructor to create a new exception
 * object. It automatically inserts the filename and the source
 * code line into the object upon creation.
 */

#define MYMONEYEXCEPTION(exceptionMessage) MyMoneyException(qPrintable(QString::fromLatin1("%1 %2:%3").arg(exceptionMessage, QString::fromLatin1(__FILE__), QString::number(__LINE__))))

/**
 * @def The MYMONEYEXCEPTION(what) define
 * This is alternative constructor to create a new exception
 * object. It avoids needless string conversion if the input string is const char*
 */

#define MYMONEYEXCEPTION_CSTRING(exceptionMessage) MyMoneyException(exceptionMessage " " __FILE__ ":" KMM_TOSTRING(__LINE__))

/**
  * The constructor to create a new MyMoneyException object.
  *
  * @param exceptionMessage reference to const char * containing the message
  *
  * An easier way to use this constructor is to use the macro
  * MYMONEYEXCEPTION(text) instead. It automatically assigns the file
  * and line parameter to the correct values.
  */

class MyMoneyException : public std::runtime_error
{
public:
  explicit MyMoneyException(const char *exceptionMessage) : std::runtime_error(exceptionMessage) {}
};

#endif
