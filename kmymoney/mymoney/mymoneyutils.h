/*
 * Copyright 2002-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2016  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2002       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef MYMONEYUTILS_H
#define MYMONEYUTILS_H

#include <QString>
#include "kmm_mymoney_export.h"

#include "mymoneyenums.h"

class MyMoneyMoney;
class MyMoneySecurity;
class MyMoneyAccount;
class MyMoneyTransaction;
class MyMoneySplit;
class QDate;
class QString;

//class that has utility functions to use throughout the application.
namespace MyMoneyUtils
{
  KMM_MYMONEY_EXPORT QString getFileExtension(QString strFileName);

  /**
   * This is a convenience method. It behaves exactly as
   * MyMoneyMoney::formatMoney, but takes the information
   * about currency symbol and precision out of the MyMoneySecurity
   * and MyMoneyAccount objects @a acc and @a sec. The value to be
   * formatted is passed as @a val.
   */
  KMM_MYMONEY_EXPORT QString formatMoney(const MyMoneyMoney& val,
                                         const MyMoneyAccount& acc,
                                         const MyMoneySecurity& sec,
                                         bool showThousandSeparator = true);

  /**
   * This is a convenience method. It behaves exactly as the above one,
   * but takes the information about currency symbol and precision out
   * of the MyMoneySecurity object @a sec.  The value to be
   * formatted is passed as @a val.
   */
  KMM_MYMONEY_EXPORT QString formatMoney(const MyMoneyMoney& val,
                                         const MyMoneySecurity& sec,
                                         bool showThousandSeparator = true);

  /**
   * This function returns a date in the form specified by Qt::ISODate.
   * If the @p date is invalid an empty string will be returned.
   *
   * @param date const reference to date to be converted
   * @return QString containing the converted date
   */
  KMM_MYMONEY_EXPORT QString dateToString(const QDate& date);

  /**
   * This function returns a date as QDate object as specified by
   * the parameter @p str. @p str must be in Qt::ISODate format.
   * If @p str is empty or contains an invalid date, QDate() is
   * returned.
   *
   * @param str date in Qt::ISODate format
   * @return QDate object
   */
  KMM_MYMONEY_EXPORT QDate stringToDate(const QString& str);

  KMM_MYMONEY_EXPORT QString QStringEmpty(const QString&);

  KMM_MYMONEY_EXPORT unsigned long extractId(const QString& txt);

  KMM_MYMONEY_EXPORT void dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, eMyMoney::Split::InvestmentTransactionType& transactionType);

  KMM_MYMONEY_EXPORT QString formatDate(const QDate& date);

  /**
   * This method is used to convert the payment type from its
   * internal representation into a human readable format.
   *
   * @param paymentType numerical representation of the MyMoneySchedule
   *                  payment type
   *
   * @return QString representing the human readable format translated according to the language catalog
   *
   * @sa MyMoneySchedule::paymentMethodToString()
   */
  KMM_MYMONEY_EXPORT QString paymentMethodToString(eMyMoney::Schedule::PaymentType paymentType);


}

#endif
