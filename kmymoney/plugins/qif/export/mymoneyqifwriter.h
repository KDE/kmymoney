/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYQIFWRITER_H
#define MYMONEYQIFWRITER_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QObject>
#include <QDateTime>
#include <QTextStream>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers

#include "../config/mymoneyqifprofile.h"

class MyMoneyTransaction;
class MyMoneySplit;

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents the QIF writer. All conversion between the
  * internal representation of accounts, transactions is handled in this
  * object. The conversion is controlled using a MyMoneyQifProfile to allow
  * the user to control the conversion.
  */
class MyMoneyQifWriter : public QObject
{
  Q_OBJECT

public:
  MyMoneyQifWriter();
  ~MyMoneyQifWriter();

  /**
    * This method is used to start the conversion. The parameters control
    * the destination of the data and the parts that will be exported.
    * Individual errors will be reported using message boxes.
    *
    * @param filename The name of the output file with full path information
    * @param profile The name of the profile to be used for conversion
    * @param accountId The id of the account that will be exported
    * @param accountData If true, the transactions will be exported
    * @param categoryData If true, the categories will be exported as well
    * @param startDate Transactions before this date will not be exported
    * @param endDate Transactions after this date will not be exported
    */
  void write(const QString &filename, const QString &profile,
             const QString &accountId, const bool accountData,
             const bool categoryData,
             const QDate &startDate, const QDate &endDate);

private:

  /**
    * This method writes the entries necessary for an account. First
    * the leadin, and then the transactions that are in the account
    * specified by @p accountId in the range from @p startDate to @p
    * endDate.
    *
    * @param s reference to textstream
    * @param accountId id of the account to be written
    * @param startDate date from which entries are written
    * @param endDate date until which entries are written
    */
  void writeAccountEntry(QTextStream &s, const QString &accountId, const QDate &startDate, const QDate &endDate);

  /**
    * This method writes the category entries to the stream
    * @p s. It writes the leadin and uses writeCategoryEntries()
    * to write the entries and emits signalProgess() where needed.
    *
    * @param s reference to textstream
    */
  void writeCategoryEntries(QTextStream &s);

  /**
    * This method writes the category entry for account with
    * the ID @p accountId to the stream @p s. All subaccounts
    * are processed as well.
    *
    * @param s reference to textstream
    * @param accountId id of the account to be written
    * @param leadIn constant text that will be prepended to the account's name
    */
  void writeCategoryEntry(QTextStream &s, const QString &accountId, const QString &leadIn);

  void writeTransactionEntry(QTextStream &s, const MyMoneyTransaction &t, const QString &accountId);
  void writeSplitEntry(QTextStream &s, const MyMoneySplit &t);
  void extractInvestmentEntries(QTextStream &s, const QString &accountId, const QDate &startDate, const QDate &endDate);
  void writeInvestmentEntry(QTextStream &stream, const MyMoneyTransaction &t, const int count);

Q_SIGNALS:
  /**
    * This signal is emitted while the operation progresses.
    * When the operation starts, the signal is emitted with
    * @p current being 0 and @p max having the maximum value.
    *
    * During the operation, the signal is emitted with @p current
    * containing the current value on the way to the maximum value.
    * @p max will be 0 in this case.
    *
    * When the operation is finished, the signal is emitted with
    * @p current and @p max set to -1 to identify the end of the
    * operation.
    *
    * @param current see above
    * @param max see above
    */
  void signalProgress(int current, int max);

private:
  MyMoneyQifProfile m_qifProfile;

};

#endif
