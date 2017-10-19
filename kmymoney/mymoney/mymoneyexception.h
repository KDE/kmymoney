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

#include <QString>
/**
  * @file
  * @author Thomas Baumgart
  */

/**
  * This class describes an exception that is thrown by the engine
  * in case of a failure.
  */

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
  MyMoneyException(const QString& msg, const QString& file, const unsigned long line);

  ~MyMoneyException();

  /**
    * This method is used to return the message that was passed
    * during the creation of the exception object.
    *
    * @return reference to QString containing the message
    */
  const QString& what() const {
    return m_msg;
  }

  /**
    * This method is used to return the filename that was passed
    * during the creation of the exception object.
    *
    * @return reference to QString containing the filename
    */
  const QString& file() const {
    return m_file;
  }

  /**
    * This method is used to return the linenumber that was passed
    * during the creation of the exception object.
    *
    * @return long integer containing the line number
    */
  unsigned long line() const {
    return m_line;
  }

private:
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

#endif
