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

#include "mymoneyutils.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QDate>
#include <QLocale>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneyschedule.h"

QString MyMoneyUtils::getFileExtension(QString strFileName)
{
  QString strTemp;
  if (!strFileName.isEmpty()) {
    //find last . deliminator
    int nLoc = strFileName.lastIndexOf('.');
    if (nLoc != -1) {
      strTemp = strFileName.right(strFileName.length() - (nLoc + 1));
      return strTemp.toUpper();
    }
  }
  return strTemp;
}

QString MyMoneyUtils::formatMoney(const MyMoneyMoney& val,
                                  const MyMoneyAccount& acc,
                                  const MyMoneySecurity& sec,
                                  bool showThousandSeparator)
{
  return val.formatMoney(sec.tradingSymbol(),
                         val.denomToPrec(acc.fraction()),
                         showThousandSeparator);
}

QString MyMoneyUtils::formatMoney(const MyMoneyMoney& val,
                                  const MyMoneySecurity& sec,
                                  bool showThousandSeparator)
{
  return val.formatMoney(sec.tradingSymbol(),
                         val.denomToPrec(sec.smallestAccountFraction()),
                         showThousandSeparator);
}

QString MyMoneyUtils::dateToString(const QDate& date)
{
  if (!date.isNull() && date.isValid())
    return date.toString(Qt::ISODate);

  return QString();
}

QDate MyMoneyUtils::stringToDate(const QString& str)
{
  if (str.length()) {
    QDate date = QDate::fromString(str, Qt::ISODate);
    if (!date.isNull() && date.isValid())
      return date;
  }
  return QDate();
}

QString MyMoneyUtils::QStringEmpty(const QString& val)
{
  if (!val.isEmpty())
    return QString(val);

  return QString();
}

unsigned long MyMoneyUtils::extractId(const QString& txt)
{
  int pos;
  unsigned long rc = 0;

  pos = txt.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    rc = txt.mid(pos).toInt();
  }
  return rc;
}

void MyMoneyUtils::dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, eMyMoney::Split::InvestmentTransactionType& transactionType)
{
  // collect the splits. split references the stock account and should already
  // be set up. assetAccountSplit references the corresponding asset account (maybe
  // empty), feeSplits is the list of all expenses and interestSplits
  // the list of all incomes
  assetAccountSplit = MyMoneySplit(); // set to none to check later if it was assigned
  auto file = MyMoneyFile::instance();
  foreach (const auto tsplit, transaction.splits()) {
    auto acc = file->account(tsplit.accountId());
    if (tsplit.id() == split.id()) {
      security = file->security(acc.currencyId());
    } else if (acc.accountGroup() == eMyMoney::Account::Type::Expense) {
      feeSplits.append(tsplit);
      // feeAmount += tsplit.value();
    } else if (acc.accountGroup() == eMyMoney::Account::Type::Income) {
      interestSplits.append(tsplit);
      // interestAmount += tsplit.value();
    } else {
      if (assetAccountSplit == MyMoneySplit()) // first asset Account should be our requested brokerage account
        assetAccountSplit = tsplit;
      else if (tsplit.value().isNegative())  // the rest (if present) is handled as fee or interest
        feeSplits.append(tsplit);              // and shouldn't be allowed to override assetAccountSplit
        else if (tsplit.value().isPositive())
          interestSplits.append(tsplit);
    }
  }

  // determine transaction type
  transactionType = split.investmentTransactionType();
  if (transactionType == eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType) {
    transactionType = eMyMoney::Split::InvestmentTransactionType::BuyShares;
  }

  currency.setTradingSymbol("???");
  try {
    currency = file->security(transaction.commodity());
  } catch (const MyMoneyException &) {
  }
}

QString MyMoneyUtils::formatDate(const QDate& date)
{
  static QString format;
  if (format.isEmpty()) {
     format = QLocale().dateFormat(QLocale::ShortFormat);
    if (!format.contains(QLatin1String("yyyy")) && format.contains(QLatin1String("yy"))) {
      format.replace(QLatin1String("yy"), QLatin1String("yyyy"));
    }
  }
  return date.toString(format);
}

QString MyMoneyUtils::paymentMethodToString(eMyMoney::Schedule::PaymentType paymentType)
{
  return i18nc("Scheduled Transaction payment type", MyMoneySchedule::paymentMethodToString(paymentType).toLatin1());
}

