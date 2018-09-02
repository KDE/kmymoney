/*
 * Copyright 2001-2002  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002-2003  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2006-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
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

#include "mymoneyaccountloan.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QDate>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

MyMoneyAccountLoan::MyMoneyAccountLoan(const MyMoneyAccount& acc)
    : MyMoneyAccount(acc)
{
}

const MyMoneyMoney MyMoneyAccountLoan::loanAmount() const
{
  return MyMoneyMoney(value("loan-amount"));
}

void MyMoneyAccountLoan::setLoanAmount(const MyMoneyMoney& amount)
{
  setValue("loan-amount", amount.toString());
}

const MyMoneyMoney MyMoneyAccountLoan::interestRate(const QDate& date) const
{
  MyMoneyMoney rate;
  QString key;
  QString val;

  if (!date.isValid())
    return rate;

  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());

  QRegExp regExp("ir-(\\d{4})-(\\d{2})-(\\d{2})");

  QMap<QString, QString>::ConstIterator it;
  for (it = pairs().constBegin(); it != pairs().constEnd(); ++it) {
    if (regExp.indexIn(it.key()) > -1) {
      if (qstrcmp(it.key().toLatin1(), key.toLatin1()) <= 0)
        val = *it;
      else
        break;

    } else if (!val.isEmpty())
      break;
  }
  if (!val.isEmpty()) {
    rate = MyMoneyMoney(val);
  }

  return rate;
}

void MyMoneyAccountLoan::setInterestRate(const QDate& date, const MyMoneyMoney& value)
{
  if (!date.isValid())
    return;

  QString key;
  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());
  setValue(key, value.toString());
}

MyMoneyAccountLoan::interestDueE MyMoneyAccountLoan::interestCalculation() const
{
  QString payTime(value("interest-calculation"));
  if (payTime == "paymentDue")
    return paymentDue;
  return paymentReceived;
}

void MyMoneyAccountLoan::setInterestCalculation(const MyMoneyAccountLoan::interestDueE onReception)
{
  if (onReception == paymentDue)
    setValue("interest-calculation", "paymentDue");
  else
    setValue("interest-calculation", "paymentReceived");
}

const QDate MyMoneyAccountLoan::nextInterestChange() const
{
  QDate rc;

  QRegExp regExp("(\\d{4})-(\\d{2})-(\\d{2})");
  if (regExp.indexIn(value("interest-nextchange")) != -1) {
    rc.setDate(regExp.cap(1).toInt(), regExp.cap(2).toInt(), regExp.cap(3).toInt());
  }
  return rc;
}

void MyMoneyAccountLoan::setNextInterestChange(const QDate& date)
{
  setValue("interest-nextchange", date.toString(Qt::ISODate));
}

int MyMoneyAccountLoan::interestChangeFrequency(int* unit) const
{
  int rc = -1;

  if (unit)
    *unit = 1;

  QRegExp regExp("(\\d+)/(\\d{1})");
  if (regExp.indexIn(value("interest-changefrequency")) != -1) {
    rc = regExp.cap(1).toInt();
    if (unit != 0) {
      *unit = regExp.cap(2).toInt();
    }
  }
  return rc;
}

void MyMoneyAccountLoan::setInterestChangeFrequency(const int amount, const int unit)
{
  QString val;
  val.sprintf("%d/%d", amount, unit);
  setValue("interest-changeFrequency", val);
}

const QString MyMoneyAccountLoan::schedule() const
{
  return QString(value("schedule").toLatin1());
}

void MyMoneyAccountLoan::setSchedule(const QString& sched)
{
  setValue("schedule", sched);
}

bool MyMoneyAccountLoan::fixedInterestRate() const
{
  // make sure, that an empty kvp element returns true
  return !(value("fixed-interest") == "no");
}

void MyMoneyAccountLoan::setFixedInterestRate(const bool fixed)
{
  setValue("fixed-interest", fixed ? "yes" : "no");
  if (fixed) {
    deletePair("interest-nextchange");
    deletePair("interest-changeFrequency");
  }
}

const MyMoneyMoney MyMoneyAccountLoan::finalPayment() const
{
  return MyMoneyMoney(value("final-payment"));
}

void MyMoneyAccountLoan::setFinalPayment(const MyMoneyMoney& finalPayment)
{
  setValue("final-payment", finalPayment.toString());
}

unsigned int MyMoneyAccountLoan::term() const
{
  return value("term").toUInt();
}

void MyMoneyAccountLoan::setTerm(const unsigned int payments)
{
  setValue("term", QString::number(payments));
}

const MyMoneyMoney MyMoneyAccountLoan::periodicPayment() const
{
  return MyMoneyMoney(value("periodic-payment"));
}

void MyMoneyAccountLoan::setPeriodicPayment(const MyMoneyMoney& payment)
{
  setValue("periodic-payment", payment.toString());
}

const QString MyMoneyAccountLoan::payee() const
{
  return value("payee");
}

void MyMoneyAccountLoan::setPayee(const QString& payee)
{
  setValue("payee", payee);
}

const QString MyMoneyAccountLoan::interestAccountId() const
{
  return QString();
}

void MyMoneyAccountLoan::setInterestAccountId(const QString& /* id */)
{

}

bool MyMoneyAccountLoan::hasReferenceTo(const QString& id) const
{
  return MyMoneyAccount::hasReferenceTo(id)
         || (id == payee())
         || (id == schedule());
}

void MyMoneyAccountLoan::setInterestCompounding(int frequency)
{
  setValue("compoundingFrequency", QString("%1").arg(frequency));
}

int MyMoneyAccountLoan::interestCompounding() const
{
  return value("compoundingFrequency").toInt();
}
