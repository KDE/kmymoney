/***************************************************************************
                          mymoneyexception.h  -  description
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

#ifndef MYMONEYEXCEPTION_H
#define MYMONEYEXCEPTION_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

class QString;

/**
  * @file
  * @author Thomas Baumgart
  */

/**
  * This class describes an exception that is thrown by the engine
  * in case of a failure.
  */

class MyMoneyExceptionPrivate;
class KMM_MYMONEY_EXPORT MyMoneyException
{
public:

  /**
    * @def MYMONEYEXCEPTION(text)
    * This is the preferred constructor to create a new exception
    * object. It automatically inserts the filename and the source
    * code line into the object upon creation.
    *
    * It is equivilant to MyMoneyException(text, __FILE__, __LINE__)
    */
#define MYMONEYEXCEPTION(what) MyMoneyException(what, __FILE__, __LINE__)

  /**
    * The constructor to create a new MyMoneyException object.
    *
    * @param msg reference to QString containing the message
    * @param file reference to QString containing the name of the sourcefile where
    *             the exception was thrown
    * @param line unsigned long containing the line number of the line where
    *             the exception was thrown in the file.
    *
    * An easier way to use this constructor is to use the macro
    * MYMONEYEXCEPTION(text) instead. It automatically assigns the file
    * and line parameter to the correct values.
    */
  explicit MyMoneyException(const QString& msg,
                            const QString& file,
                            const unsigned long line);

  MyMoneyException(const MyMoneyException & other);
  MyMoneyException(MyMoneyException && other);
  MyMoneyException & operator=(MyMoneyException other);
  friend void swap(MyMoneyException& first, MyMoneyException& second);

  ~MyMoneyException();

  /**
    * This method is used to return the message that was passed
    * during the creation of the exception object.
    *
    * @return reference to QString containing the message
    */
  QString what() const;

  /**
    * This method is used to return the filename that was passed
    * during the creation of the exception object.
    *
    * @return reference to QString containing the filename
    */
  QString file() const;

  /**
    * This method is used to return the linenumber that was passed
    * during the creation of the exception object.
    *
    * @return long integer containing the line number
    */
  unsigned long line() const;

private:
  // #define MYMONEYEXCEPTION(what) requires non-const d_ptr
  MyMoneyExceptionPrivate * d_ptr; // krazy:exclude=dpointer
  Q_DECLARE_PRIVATE(MyMoneyException)
  MyMoneyException();
};

inline void swap(MyMoneyException& first, MyMoneyException& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyException::MyMoneyException(MyMoneyException && other) : MyMoneyException() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyException & MyMoneyException::operator=(MyMoneyException other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

#endif
