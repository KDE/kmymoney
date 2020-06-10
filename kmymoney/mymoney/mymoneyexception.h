/*
 * Copyright 2002-2008  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYMONEYEXCEPTION_H
#define MYMONEYEXCEPTION_H

#define KMM_STRINGIFY(x) #x
#define KMM_TOSTRING(x) KMM_STRINGIFY(x)

#include <stdexcept>
#include "kmm_mymoney_export.h"

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

// krazy:excludeall=dpointer
// krazy:excludeall=inline

#if defined(Q_OS_WIN)
// Otherwise
// non dll-interface class 'std::runtime_error' used as base for dll-interface class 'MyMoneyException'
class MyMoneyException final : public std::runtime_error

#else
// Based on https://gcc.gnu.org/wiki/Visibility
// custom exception classes should always be exported
// Otherwise we get an error like that:
// Expected exception of type MyMoneyException to be thrown but std::exception caught
class KMM_MYMONEY_EXPORT MyMoneyException final : public std::runtime_error

#endif
{
public:
/**
  * The constructor to create a new MyMoneyException object.
  *
  * @param exceptionMessage reference to const char * containing the message
  *
  * An easier way to use this constructor is to use the macro
  * MYMONEYEXCEPTION(text) instead. It automatically assigns the file
  * and line parameter to the correct values.
  */
  explicit MyMoneyException(const char *exceptionMessage) : std::runtime_error(exceptionMessage) {}  // krazy:exclude=inline
};

#endif
