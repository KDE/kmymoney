/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#ifndef ONLINETASKCONVERTER_H
#define ONLINETASKCONVERTER_H

#include <qobject.h>
#include <QStringList>

class onlineTask;

/**
 * @brief Base to convert task of one type to another type.
 *
 * If you want to enable KMyMoney to convert a task to another task you must implement this
 * interface.
 */
class onlineTaskConverter
{
public:

  /**
   * @brief Type of conversion
   *
   * They are ordered. convertImpossible is 0, higher number means better.
   *
   * Used by canConvert().
   */
  enum convertType {
    /** Convert operation is not possible */
    convertImpossible = 0,
    /** Conversion is accompanied with loss of data. The user is warned and has to confirm the changes. */
    convertionLossyMajor,
    /**
     * Conversion is accompanied with change of data. The user must be informed and hast to confirm the changes.
     * Anyway the new task is quite equivalent to the old one.
     */
    convertionLossyMinor,
    /** Conversion is possible without user interaction */
    convertionLoseless
  };

  onlineTaskConverter();
  virtual ~onlineTaskConverter();

  /**
   * @brief List of tasks you accept to convert
   *
   * @return list of task iids
   */
  virtual QStringList convertibleTasks() const = 0;

  /**
   * @brief Task you convert into
   *
   * @return task iid
   */
  virtual QString convertedTask() const = 0;

  /**
   * @brief Convert a task
   *
   * @return The returned task must be of type convertedTask() or 0. Caller takes ownership.
   *
   * @param source task to convert (do not modify it!). It is always of one of the types convertibleTasks()
   * @param convertResult OUT convertType, if convertionLossy you should provide a userInformation
   * @param userInformation OUT a translated string with description which data was lost during conversion.
   * This string is shown by the ui to the user using a KMessageWidget.
   *
   * Never forget to set convertResult! You should always set userInformation and convertResult. Code for copy & paste:
   * @code
   * userInformation = QString();
   * convertResult = convertImpossible;
   * @endcode
   *
   * You must not throw exceptions.
   */
  virtual onlineTask* convert(const onlineTask& source, convertType &convertResult, QString& userInformation) const = 0;
};

Q_DECLARE_INTERFACE(onlineTaskConverter, "org.kmymoney.plugin.onlinetaskconverter");

#endif // ONLINETASKCONVERTER_H
