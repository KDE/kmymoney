/*
 * Copyright 2000-2004  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2005       Ace Jones <acejones@users.sourceforge.net>
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

#include "mymoneyschedule.h"
#include "mymoneyschedule_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysplit.h"
#include "imymoneyprocessingcalendar.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;
using namespace eMyMoney;

static IMyMoneyProcessingCalendar* processingCalendarPtr = 0;

MyMoneySchedule::MyMoneySchedule() :
    MyMoneyObject(*new MyMoneySchedulePrivate)
{
}

MyMoneySchedule::MyMoneySchedule(const QString& name,
                                 Schedule::Type type,
                                 Schedule::Occurrence occurrence,
                                 int occurrenceMultiplier,
                                 Schedule::PaymentType paymentType,
                                 const QDate& /* startDate */,
                                 const QDate& endDate,
                                 bool fixed,
                                 bool autoEnter) :
    MyMoneyObject(*new MyMoneySchedulePrivate)
{
  Q_D(MyMoneySchedule);
  // Set up the values possibly differeing from defaults
  d->m_name = name;
  d->m_occurrence = occurrence;
  d->m_occurrenceMultiplier = occurrenceMultiplier;
  simpleToCompoundOccurrence(d->m_occurrenceMultiplier, d->m_occurrence);
  d->m_type = type;
  d->m_paymentType = paymentType;
  d->m_fixed = fixed;
  d->m_autoEnter = autoEnter;
  d->m_endDate = endDate;
}

MyMoneySchedule::MyMoneySchedule(const QDomElement& node) :
    MyMoneyObject(*new MyMoneySchedulePrivate, node)
{
  if (nodeNames[nnScheduleTX] != node.tagName())
    throw MYMONEYEXCEPTION_CSTRING("Node was not SCHEDULED_TX");

  Q_D(MyMoneySchedule);
  d->m_name = node.attribute(d->getAttrName(Schedule::Attribute::Name));
  d->m_startDate = MyMoneyUtils::stringToDate(node.attribute(d->getAttrName(Schedule::Attribute::StartDate)));
  d->m_endDate = MyMoneyUtils::stringToDate(node.attribute(d->getAttrName(Schedule::Attribute::EndDate)));
  d->m_lastPayment = MyMoneyUtils::stringToDate(node.attribute(d->getAttrName(Schedule::Attribute::LastPayment)));

  d->m_type = static_cast<Schedule::Type>(node.attribute(d->getAttrName(Schedule::Attribute::Type)).toInt());
  d->m_paymentType = static_cast<Schedule::PaymentType>(node.attribute(d->getAttrName(Schedule::Attribute::PaymentType)).toInt());
  d->m_occurrence = static_cast<Schedule::Occurrence>(node.attribute(d->getAttrName(Schedule::Attribute::Occurrence)).toInt());
  d->m_occurrenceMultiplier = node.attribute(d->getAttrName(Schedule::Attribute::OccurrenceMultiplier), "1").toInt();
  // Convert to compound occurrence
  simpleToCompoundOccurrence(d->m_occurrenceMultiplier, d->m_occurrence);
  d->m_lastDayInMonth = static_cast<bool>(node.attribute("lastDayInMonth").toInt());
  d->m_autoEnter = static_cast<bool>(node.attribute(d->getAttrName(Schedule::Attribute::AutoEnter)).toInt());
  d->m_fixed = static_cast<bool>(node.attribute(d->getAttrName(Schedule::Attribute::Fixed)).toInt());
  d->m_weekendOption = static_cast<Schedule::WeekendOption>(node.attribute(d->getAttrName(Schedule::Attribute::WeekendOption)).toInt());

  // read in the associated transaction
  QDomNodeList nodeList = node.elementsByTagName(nodeNames[nnTransaction]);
  if (nodeList.count() == 0)
    throw MYMONEYEXCEPTION_CSTRING("SCHEDULED_TX has no TRANSACTION node");

  setTransaction(MyMoneyTransaction(nodeList.item(0).toElement(), false), true);

  // some old versions did not remove the entry date and post date fields
  // in the schedule. So if this is the case, we deal with a very old transaction
  // and can't use the post date field as next due date. Hence, we wipe it out here
  if (d->m_transaction.entryDate().isValid()) {
    d->m_transaction.setPostDate(QDate());
    d->m_transaction.setEntryDate(QDate());
  }

  // readin the recorded payments
  nodeList = node.elementsByTagName(d->getElName(Schedule::Element::Payments));
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName(d->getElName(Schedule::Element::Payment));
    for (int i = 0; i < nodeList.count(); ++i) {
      d->m_recordedPayments << MyMoneyUtils::stringToDate(nodeList.item(i).toElement().attribute(d->getAttrName(Schedule::Attribute::Date)));
    }
  }

  // if the next due date is not set (comes from old version)
  // then set it up the old way
  if (!nextDueDate().isValid() && !d->m_lastPayment.isValid()) {
    d->m_transaction.setPostDate(d->m_startDate);
    // clear it, because the schedule has never been used
    d->m_startDate = QDate();
  }

  // There are reports that lastPayment and nextDueDate are identical or
  // that nextDueDate is older than lastPayment. This could
  // be caused by older versions of the application. In this case, we just
  // clear out the nextDueDate and let it calculate from the lastPayment.
  if (nextDueDate().isValid() && nextDueDate() <= d->m_lastPayment) {
    d->m_transaction.setPostDate(QDate());
  }

  if (!nextDueDate().isValid()) {
    d->m_transaction.setPostDate(d->m_startDate);
    d->m_transaction.setPostDate(nextPayment(d->m_lastPayment.addDays(1)));
  }
}

MyMoneySchedule::MyMoneySchedule(const MyMoneySchedule& other) :
  MyMoneyObject(*new MyMoneySchedulePrivate(*other.d_func()), other.id())
{
}

MyMoneySchedule::MyMoneySchedule(const QString& id, const MyMoneySchedule& other) :
  MyMoneyObject(*new MyMoneySchedulePrivate(*other.d_func()), id)
{
}

MyMoneySchedule::~MyMoneySchedule()
{
}

Schedule::Occurrence MyMoneySchedule::occurrence() const
{
  Q_D(const MyMoneySchedule);
  Schedule::Occurrence occ = d->m_occurrence;
  int mult = d->m_occurrenceMultiplier;
  compoundToSimpleOccurrence(mult, occ);
  return occ;
}

int MyMoneySchedule::occurrenceMultiplier() const
{
  Q_D(const MyMoneySchedule);
  return d->m_occurrenceMultiplier;
}

eMyMoney::Schedule::Type MyMoneySchedule::type() const
{
  Q_D(const MyMoneySchedule);
  return d->m_type;
}

eMyMoney::Schedule::Occurrence MyMoneySchedule::occurrencePeriod() const
{
  Q_D(const MyMoneySchedule);
  return d->m_occurrence;
}

void MyMoneySchedule::setStartDate(const QDate& date)
{
  Q_D(MyMoneySchedule);
  d->m_startDate = date;
}

void MyMoneySchedule::setPaymentType(Schedule::PaymentType type)
{
  Q_D(MyMoneySchedule);
  d->m_paymentType = type;
}

void MyMoneySchedule::setFixed(bool fixed)
{
  Q_D(MyMoneySchedule);
  d->m_fixed = fixed;
}

void MyMoneySchedule::setTransaction(const MyMoneyTransaction& transaction)
{
  setTransaction(transaction, false);
}

void MyMoneySchedule::setTransaction(const MyMoneyTransaction& transaction, bool noDateCheck)
{
  auto t = transaction;
  Q_D(MyMoneySchedule);
  if (!noDateCheck) {
    // don't allow a transaction that has no due date
    // if we get something like that, then we use the
    // the current next due date. If that is also invalid
    // we can't help it.
    if (!t.postDate().isValid()) {
      t.setPostDate(d->m_transaction.postDate());
    }

    if (!t.postDate().isValid())
      return;
  }

  // make sure to clear out some unused information in scheduled transactions
  // we need to do this for the case that the transaction passed as argument
  // is a matched or imported transaction.
  auto firstSplit = true;
  foreach (const auto split, t.splits()) {
    MyMoneySplit s = split;
    // clear out the bankID
    if (!split.bankID().isEmpty()) {
      s.setBankID(QString());
      t.modifySplit(s);
    }

    // only clear payees from second split onwards
    if (firstSplit) {
      firstSplit = false;
      continue;
    }

    if (!split.payeeId().isEmpty()) {
      // but only if the split references an income/expense category
      auto file = MyMoneyFile::instance();
      // some unit tests don't have a storage attached, so we
      // simply skip the test
      // Don't check for accounts with an id of 'Phony-ID' which is used
      // internally for non-existing accounts (during creation of accounts)
      if (file->storageAttached() && s.accountId() != QString("Phony-ID")) {
        auto acc = file->account(s.accountId());
        if (acc.isIncomeExpense()) {
          s.setPayeeId(QString());
          t.modifySplit(s);
        }
      }
    }
  }

  d->m_transaction = t;
  // make sure that the transaction does not have an id so that we can enter
  // it into the engine
  d->m_transaction.clearId();
}

void MyMoneySchedule::setEndDate(const QDate& date)
{
  Q_D(MyMoneySchedule);
  d->m_endDate = date;
}

void MyMoneySchedule::setLastDayInMonth(bool state)
{
  Q_D(MyMoneySchedule);
  d->m_lastDayInMonth = state;
}

void MyMoneySchedule::setAutoEnter(bool autoenter)
{
  Q_D(MyMoneySchedule);
  d->m_autoEnter = autoenter;
}

QDate MyMoneySchedule::startDate() const
{
  Q_D(const MyMoneySchedule);
  if (d->m_startDate.isValid())
    return d->m_startDate;
  return nextDueDate();
}

eMyMoney::Schedule::PaymentType MyMoneySchedule::paymentType() const
{
  Q_D(const MyMoneySchedule);
  return d->m_paymentType;
}

/**
  * Simple get method that returns true if the schedule is fixed.
  *
  * @return bool To indicate whether the instance is fixed.
  */
bool MyMoneySchedule::isFixed() const
{
  Q_D(const MyMoneySchedule);
  return d->m_fixed;
}

/**
  * Simple get method that returns true if the schedule will end
  * at some time.
  *
  * @return bool Indicates whether the instance will end.
  */
bool MyMoneySchedule::willEnd() const
{
  Q_D(const MyMoneySchedule);
  return d->m_endDate.isValid();
}


QDate MyMoneySchedule::nextDueDate() const
{
  Q_D(const MyMoneySchedule);
  return d->m_transaction.postDate();
}

QDate MyMoneySchedule::adjustedNextDueDate() const
{
  if (isFinished())
    return QDate();

  if (lastDayInMonth()) {
    QDate date = nextDueDate();
    return adjustedDate(QDate(date.year(), date.month(), date.daysInMonth()), weekendOption());
  }

  return adjustedDate(nextDueDate(), weekendOption());
}

QDate MyMoneySchedule::adjustedDate(QDate date, Schedule::WeekendOption option) const
{
  if (!date.isValid() || option == Schedule::WeekendOption::MoveNothing || isProcessingDate(date))
    return date;

  int step = 1;
  if (option == Schedule::WeekendOption::MoveBefore)
    step = -1;

  while (!isProcessingDate(date))
    date = date.addDays(step);

  return date;
}

void MyMoneySchedule::setNextDueDate(const QDate& date)
{
  Q_D(MyMoneySchedule);
  if (date.isValid()) {
    d->m_transaction.setPostDate(date);
    // m_startDate = date;
  }
}

void MyMoneySchedule::setLastPayment(const QDate& date)
{
  Q_D(MyMoneySchedule);
  // Delete all payments older than date
  QList<QDate>::Iterator it;
  QList<QDate> delList;

  for (it = d->m_recordedPayments.begin(); it != d->m_recordedPayments.end(); ++it) {
    if (*it < date || !date.isValid())
      delList.append(*it);
  }

  for (it = delList.begin(); it != delList.end(); ++it) {
    d->m_recordedPayments.removeAll(*it);
  }

  d->m_lastPayment = date;
  if (!d->m_startDate.isValid())
    d->m_startDate = date;
}

QString MyMoneySchedule::name() const
{
  Q_D(const MyMoneySchedule);
  return d->m_name;
}

void MyMoneySchedule::setName(const QString& nm)
{
  Q_D(MyMoneySchedule);
  d->m_name = nm;
}

eMyMoney::Schedule::WeekendOption MyMoneySchedule::weekendOption() const
{
  Q_D(const MyMoneySchedule);
  return d->m_weekendOption;
}

void MyMoneySchedule::setOccurrence(Schedule::Occurrence occ)
{
  auto occ2 = occ;
  auto mult = 1;
  simpleToCompoundOccurrence(mult, occ2);
  setOccurrencePeriod(occ2);
  setOccurrenceMultiplier(mult);
}

void MyMoneySchedule::setOccurrencePeriod(Schedule::Occurrence occ)
{
  Q_D(MyMoneySchedule);
  d->m_occurrence = occ;
}

void MyMoneySchedule::setOccurrenceMultiplier(int occmultiplier)
{
  Q_D(MyMoneySchedule);
  d->m_occurrenceMultiplier = occmultiplier < 1 ? 1 : occmultiplier;
}

void MyMoneySchedule::setType(Schedule::Type type)
{
  Q_D(MyMoneySchedule);
  d->m_type = type;
}

void MyMoneySchedule::validate(bool id_check) const
{
  /* Check the supplied instance is valid...
   *
   * To be valid it must not have the id set and have the following fields set:
   *
   * m_occurrence
   * m_type
   * m_startDate
   * m_paymentType
   * m_transaction
   *   the transaction must contain at least one split (two is better ;-)  )
   */
  Q_D(const MyMoneySchedule);
  if (id_check && !d->m_id.isEmpty())
    throw MYMONEYEXCEPTION_CSTRING("ID for schedule not empty when required");

  if (d->m_occurrence == Schedule::Occurrence::Any)
    throw MYMONEYEXCEPTION_CSTRING("Invalid occurrence type for schedule");

  if (d->m_type == Schedule::Type::Any)
    throw MYMONEYEXCEPTION_CSTRING("Invalid type for schedule");

  if (!nextDueDate().isValid())
    throw MYMONEYEXCEPTION_CSTRING("Invalid next due date for schedule");

  if (d->m_paymentType == Schedule::PaymentType::Any)
    throw MYMONEYEXCEPTION_CSTRING("Invalid payment type for schedule");

  if (d->m_transaction.splitCount() == 0)
    throw MYMONEYEXCEPTION_CSTRING("Scheduled transaction does not contain splits");

  // Check the payment types
  switch (d->m_type) {
    case Schedule::Type::Bill:
      if (d->m_paymentType == Schedule::PaymentType::DirectDeposit || d->m_paymentType == Schedule::PaymentType::ManualDeposit)
        throw MYMONEYEXCEPTION_CSTRING("Invalid payment type for bills");
      break;

    case Schedule::Type::Deposit:
      if (d->m_paymentType == Schedule::PaymentType::DirectDebit || d->m_paymentType == Schedule::PaymentType::WriteChecque)
        throw MYMONEYEXCEPTION_CSTRING("Invalid payment type for deposits");
      break;

    case Schedule::Type::Any:
      throw MYMONEYEXCEPTION_CSTRING("Invalid type ANY");
      break;

    case Schedule::Type::Transfer:
//        if (m_paymentType == DirectDeposit || m_paymentType == ManualDeposit)
//          return false;
      break;

    case Schedule::Type::LoanPayment:
      break;
  }
}

QDate MyMoneySchedule::adjustedNextPayment(const QDate& refDate) const
{
  return nextPaymentDate(true, refDate);
}

QDate MyMoneySchedule::adjustedNextPayment() const
{
  return adjustedNextPayment(QDate::currentDate());
}

QDate MyMoneySchedule::nextPayment(const QDate& refDate) const
{
  return nextPaymentDate(false, refDate);
}

QDate MyMoneySchedule::nextPayment() const
{
  return nextPayment(QDate::currentDate());
}

QDate MyMoneySchedule::nextPaymentDate(const bool& adjust, const QDate& refDate) const
{
  Schedule::WeekendOption option(adjust ? weekendOption() :
                        Schedule::WeekendOption::MoveNothing);

  Q_D(const MyMoneySchedule);
  QDate adjEndDate(adjustedDate(d->m_endDate, option));

  // if the enddate is valid and it is before the reference date,
  // then there will be no more payments.
  if (adjEndDate.isValid() && adjEndDate < refDate) {
    return QDate();
  }

  QDate dueDate(nextDueDate());
  QDate paymentDate(adjustedDate(dueDate, option));

  if (paymentDate.isValid() &&
      (paymentDate <= refDate || d->m_recordedPayments.contains(dueDate))) {
    switch (d->m_occurrence) {
      case Schedule::Occurrence::Once:
        // If the lastPayment is already set or the payment should have been
        // prior to the reference date then invalidate the payment date.
        if (d->m_lastPayment.isValid() || paymentDate <= refDate)
          paymentDate = QDate();
        break;

      case Schedule::Occurrence::Daily: {
          int step = d->m_occurrenceMultiplier;
          do {
            dueDate = dueDate.addDays(step);
            paymentDate = adjustedDate(dueDate, option);
          } while (paymentDate.isValid() &&
                   (paymentDate <= refDate ||
                    d->m_recordedPayments.contains(dueDate)));
        }
        break;

      case Schedule::Occurrence::Weekly: {
          int step = 7 * d->m_occurrenceMultiplier;
          do {
            dueDate = dueDate.addDays(step);
            paymentDate = adjustedDate(dueDate, option);
          } while (paymentDate.isValid() &&
                   (paymentDate <= refDate ||
                    d->m_recordedPayments.contains(dueDate)));
        }
        break;

      case Schedule::Occurrence::EveryHalfMonth:
        do {
          dueDate = addHalfMonths(dueDate, d->m_occurrenceMultiplier);
          paymentDate = adjustedDate(dueDate, option);
        } while (paymentDate.isValid() &&
                 (paymentDate <= refDate ||
                  d->m_recordedPayments.contains(dueDate)));
        break;

      case Schedule::Occurrence::Monthly:
        do {
          dueDate = dueDate.addMonths(d->m_occurrenceMultiplier);
          fixDate(dueDate);
          paymentDate = adjustedDate(dueDate, option);
        } while (paymentDate.isValid() &&
                 (paymentDate <= refDate ||
                  d->m_recordedPayments.contains(dueDate)));
        break;

      case Schedule::Occurrence::Yearly:
        do {
          dueDate = dueDate.addYears(d->m_occurrenceMultiplier);
          fixDate(dueDate);
          paymentDate = adjustedDate(dueDate, option);
        } while (paymentDate.isValid() &&
                 (paymentDate <= refDate ||
                  d->m_recordedPayments.contains(dueDate)));
        break;

      case Schedule::Occurrence::Any:
      default:
        paymentDate = QDate();
        break;
    }
  }
  if (paymentDate.isValid() && adjEndDate.isValid() && paymentDate > adjEndDate)
    paymentDate = QDate();

  return paymentDate;
}

QDate MyMoneySchedule::nextPaymentDate(const bool& adjust) const
{
  return nextPaymentDate(adjust, QDate::currentDate());
}

QList<QDate> MyMoneySchedule::paymentDates(const QDate& _startDate, const QDate& _endDate) const
{
  QDate paymentDate(nextDueDate());
  QList<QDate> theDates;

  Schedule::WeekendOption option(weekendOption());

  Q_D(const MyMoneySchedule);
  QDate endDate(_endDate);
  if (willEnd() && d->m_endDate < endDate) {
    // consider the adjusted end date instead of the plain end date
    endDate = adjustedDate(d->m_endDate, option);
  }

  QDate start_date(adjustedDate(startDate(), option));
  // if the period specified by the parameters and the adjusted period
  // defined for this schedule don't overlap, then the list remains empty
  if ((willEnd() && adjustedDate(d->m_endDate, option) < _startDate)
      || start_date > endDate)
    return theDates;

  QDate date(adjustedDate(paymentDate, option));

  switch (d->m_occurrence) {
    case Schedule::Occurrence::Once:
      if (start_date >= _startDate && start_date <= endDate)
        theDates.append(start_date);
      break;

    case Schedule::Occurrence::Daily:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = paymentDate.addDays(d->m_occurrenceMultiplier);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case Schedule::Occurrence::Weekly: {
        int step = 7 * d->m_occurrenceMultiplier;
        while (date.isValid() && (date <= endDate)) {
          if (date >= _startDate)
            theDates.append(date);
          paymentDate = paymentDate.addDays(step);
          date = adjustedDate(paymentDate, option);
        }
      }
      break;

    case Schedule::Occurrence::EveryHalfMonth:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = addHalfMonths(paymentDate, d->m_occurrenceMultiplier);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case Schedule::Occurrence::Monthly:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = paymentDate.addMonths(d->m_occurrenceMultiplier);
        fixDate(paymentDate);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case Schedule::Occurrence::Yearly:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = paymentDate.addYears(d->m_occurrenceMultiplier);
        fixDate(paymentDate);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case Schedule::Occurrence::Any:
    default:
      break;
  }

  return theDates;
}

bool MyMoneySchedule::operator <(const MyMoneySchedule& right) const
{
  return adjustedNextDueDate() < right.adjustedNextDueDate();
}

bool MyMoneySchedule::operator ==(const MyMoneySchedule& right) const
{
  Q_D(const MyMoneySchedule);
  auto d2 = static_cast<const MyMoneySchedulePrivate *>(right.d_func());
  if (MyMoneyObject::operator==(right) &&
      d->m_occurrence == d2->m_occurrence &&
      d->m_occurrenceMultiplier == d2->m_occurrenceMultiplier &&
      d->m_type == d2->m_type &&
      d->m_startDate == d2->m_startDate &&
      d->m_paymentType == d2->m_paymentType &&
      d->m_fixed == d2->m_fixed &&
      d->m_transaction == d2->m_transaction &&
      d->m_endDate == d2->m_endDate &&
      d->m_lastDayInMonth == d2->m_lastDayInMonth &&
      d->m_autoEnter == d2->m_autoEnter &&
      d->m_lastPayment == d2->m_lastPayment &&
      ((d->m_name.length() == 0 && d2->m_name.length() == 0) || (d->m_name == d2->m_name)))
    return true;
  return false;
}

bool MyMoneySchedule::operator !=(const MyMoneySchedule& right) const
{
  return ! operator==(right);
}

int MyMoneySchedule::transactionsRemaining() const
{
  Q_D(const MyMoneySchedule);
  return transactionsRemainingUntil(adjustedDate(d->m_endDate, weekendOption()));
}

int MyMoneySchedule::transactionsRemainingUntil(const QDate& endDate) const
{
  auto counter = 0;
  Q_D(const MyMoneySchedule);

  QDate startDate = d->m_lastPayment.isValid() ? d->m_lastPayment : d->m_startDate;
  if (startDate.isValid() && endDate.isValid()) {
    QList<QDate> dates = paymentDates(startDate, endDate);
    counter = dates.count();
  }
  return counter;
}

QDate MyMoneySchedule::endDate() const
{
  Q_D(const MyMoneySchedule);
  return d->m_endDate;
}

bool MyMoneySchedule::autoEnter() const
{
  Q_D(const MyMoneySchedule);
  return d->m_autoEnter;
}

bool MyMoneySchedule::lastDayInMonth() const
{
  Q_D(const MyMoneySchedule);
  return d->m_lastDayInMonth;
}

MyMoneyTransaction MyMoneySchedule::transaction() const
{
  Q_D(const MyMoneySchedule);
  return d->m_transaction;
}

QDate MyMoneySchedule::lastPayment() const
{
  Q_D(const MyMoneySchedule);
  return d->m_lastPayment;
}

MyMoneyAccount MyMoneySchedule::account(int cnt) const
{
  Q_D(const MyMoneySchedule);
  QList<MyMoneySplit> splits = d->m_transaction.splits();
  QList<MyMoneySplit>::ConstIterator it;
  auto file = MyMoneyFile::instance();
  MyMoneyAccount acc;

  // search the first asset or liability account
  for (it = splits.constBegin(); it != splits.constEnd() && (acc.id().isEmpty() || cnt); ++it) {
    try {
      acc = file->account((*it).accountId());
      if (acc.isAssetLiability())
        --cnt;

      if (!cnt)
        return acc;
    } catch (const MyMoneyException &) {
      qWarning("Schedule '%s' references unknown account '%s'", qPrintable(id()),   qPrintable((*it).accountId()));
      return MyMoneyAccount();
    }
  }

  return MyMoneyAccount();
}

MyMoneyAccount MyMoneySchedule::transferAccount() const {
  return account(2);
}

QDate MyMoneySchedule::dateAfter(int transactions) const
{
  auto counter = 1;
  QDate paymentDate(startDate());

  if (transactions <= 0)
    return paymentDate;

  Q_D(const MyMoneySchedule);
  switch (d->m_occurrence) {
    case Schedule::Occurrence::Once:
      break;

    case Schedule::Occurrence::Daily:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(d->m_occurrenceMultiplier);
      break;

    case Schedule::Occurrence::Weekly: {
        int step = 7 * d->m_occurrenceMultiplier;
        while (counter++ < transactions)
          paymentDate = paymentDate.addDays(step);
      }
      break;

    case Schedule::Occurrence::EveryHalfMonth:
      paymentDate = addHalfMonths(paymentDate, d->m_occurrenceMultiplier * (transactions - 1));
      break;

    case Schedule::Occurrence::Monthly:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(d->m_occurrenceMultiplier);
      break;

    case Schedule::Occurrence::Yearly:
      while (counter++ < transactions)
        paymentDate = paymentDate.addYears(d->m_occurrenceMultiplier);
      break;

    case Schedule::Occurrence::Any:
    default:
      break;
  }

  return paymentDate;
}

bool MyMoneySchedule::isOverdue() const
{
  if (isFinished())
    return false;

  if (adjustedNextDueDate() >= QDate::currentDate())
    return false;

  return true;
}

bool MyMoneySchedule::isFinished() const
{
  Q_D(const MyMoneySchedule);
  if (!d->m_lastPayment.isValid())
    return false;

  if (d->m_endDate.isValid()) {
    if (d->m_lastPayment >= d->m_endDate
        || !nextDueDate().isValid()
        || nextDueDate() > d->m_endDate)
      return true;
  }

  // Check to see if its a once off payment
  if (d->m_occurrence == Schedule::Occurrence::Once)
    return true;

  return false;
}

bool MyMoneySchedule::hasRecordedPayment(const QDate& date) const
{
  Q_D(const MyMoneySchedule);
  // m_lastPayment should always be > recordedPayments()
  if (d->m_lastPayment.isValid() && d->m_lastPayment >= date)
    return true;

  if (d->m_recordedPayments.contains(date))
    return true;

  return false;
}

void MyMoneySchedule::recordPayment(const QDate& date)
{
  Q_D(MyMoneySchedule);
  d->m_recordedPayments.append(date);
}

QList<QDate> MyMoneySchedule::recordedPayments() const
{
  Q_D(const MyMoneySchedule);
  return d->m_recordedPayments;
}

void MyMoneySchedule::setWeekendOption(const Schedule::WeekendOption option)
{
  Q_D(MyMoneySchedule);
  // make sure only valid values are used. Invalid defaults to MoveNothing.
  switch (option) {
    case Schedule::WeekendOption::MoveBefore:
    case Schedule::WeekendOption::MoveAfter:
      d->m_weekendOption = option;
      break;

    default:
      d->m_weekendOption = Schedule::WeekendOption::MoveNothing;
      break;
  }
}

void MyMoneySchedule::fixDate(QDate& date) const
{
  Q_D(const MyMoneySchedule);
  QDate fixDate(d->m_startDate);
  if (fixDate.isValid()
      && date.day() != fixDate.day()
      && QDate::isValid(date.year(), date.month(), fixDate.day())) {
    date = QDate(date.year(), date.month(), fixDate.day());
  }
}

void MyMoneySchedule::writeXML(QDomDocument& document, QDomElement& parent) const
{
  auto el = document.createElement(nodeNames[nnScheduleTX]);

  Q_D(const MyMoneySchedule);
  d->writeBaseXML(document, el);

  el.setAttribute(d->getAttrName(Schedule::Attribute::Name), d->m_name);
  el.setAttribute(d->getAttrName(Schedule::Attribute::Type), (int)d->m_type);
  el.setAttribute(d->getAttrName(Schedule::Attribute::Occurrence), (int)d->m_occurrence);
  el.setAttribute(d->getAttrName(Schedule::Attribute::OccurrenceMultiplier), d->m_occurrenceMultiplier);
  el.setAttribute(d->getAttrName(Schedule::Attribute::PaymentType), (int)d->m_paymentType);
  el.setAttribute(d->getAttrName(Schedule::Attribute::StartDate), MyMoneyUtils::dateToString(d->m_startDate));
  el.setAttribute(d->getAttrName(Schedule::Attribute::EndDate), MyMoneyUtils::dateToString(d->m_endDate));
  el.setAttribute(d->getAttrName(Schedule::Attribute::Fixed), d->m_fixed);
  el.setAttribute(d->getAttrName(Schedule::Attribute::LastDayInMonth), d->m_lastDayInMonth);
  el.setAttribute(d->getAttrName(Schedule::Attribute::AutoEnter), d->m_autoEnter);
  el.setAttribute(d->getAttrName(Schedule::Attribute::LastPayment), MyMoneyUtils::dateToString(d->m_lastPayment));
  el.setAttribute(d->getAttrName(Schedule::Attribute::WeekendOption), (int)d->m_weekendOption);

  //store the payment history for this scheduled task.
  QList<QDate> payments = recordedPayments();
  QList<QDate>::ConstIterator it;
  QDomElement paymentsElement = document.createElement(d->getElName(Schedule::Element::Payments));
  for (it = payments.constBegin(); it != payments.constEnd(); ++it) {
    QDomElement paymentEntry = document.createElement(d->getElName(Schedule::Element::Payment));
    paymentEntry.setAttribute(d->getAttrName(Schedule::Attribute::Date), MyMoneyUtils::dateToString(*it));
    paymentsElement.appendChild(paymentEntry);
  }
  el.appendChild(paymentsElement);

  //store the transaction data for this task.
  d->m_transaction.writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneySchedule::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneySchedule);
  return d->m_transaction.hasReferenceTo(id);
}

QString MyMoneySchedule::occurrenceToString() const
{
  return occurrenceToString(occurrenceMultiplier(), occurrencePeriod());
}

QString MyMoneySchedule::occurrenceToString(Schedule::Occurrence occurrence)
{
  QString occurrenceString = I18N_NOOP2("Frequency of schedule", "Any");

  if (occurrence == Schedule::Occurrence::Once)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Once");
  else if (occurrence == Schedule::Occurrence::Daily)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Daily");
  else if (occurrence == Schedule::Occurrence::Weekly)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Weekly");
  else if (occurrence == Schedule::Occurrence::Fortnightly)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Fortnightly");
  else if (occurrence == Schedule::Occurrence::EveryOtherWeek)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every other week");
  else if (occurrence == Schedule::Occurrence::EveryHalfMonth)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every half month");
  else if (occurrence == Schedule::Occurrence::EveryThreeWeeks)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every three weeks");
  else if (occurrence == Schedule::Occurrence::EveryFourWeeks)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every four weeks");
  else if (occurrence == Schedule::Occurrence::EveryThirtyDays)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every thirty days");
  else if (occurrence == Schedule::Occurrence::Monthly)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Monthly");
  else if (occurrence == Schedule::Occurrence::EveryEightWeeks)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every eight weeks");
  else if (occurrence == Schedule::Occurrence::EveryOtherMonth)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every two months");
  else if (occurrence == Schedule::Occurrence::EveryThreeMonths)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every three months");
  else if (occurrence == Schedule::Occurrence::Quarterly)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Quarterly");
  else if (occurrence == Schedule::Occurrence::EveryFourMonths)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every four months");
  else if (occurrence == Schedule::Occurrence::TwiceYearly)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Twice yearly");
  else if (occurrence == Schedule::Occurrence::Yearly)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Yearly");
  else if (occurrence == Schedule::Occurrence::EveryOtherYear)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every other year");
  return occurrenceString;
}

QString MyMoneySchedule::occurrenceToString(int mult, Schedule::Occurrence type)
{
  QString occurrenceString = I18N_NOOP2("Frequency of schedule", "Any");

  if (type == Schedule::Occurrence::Once)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Once");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("%1 times").arg(mult));
    }
  else if (type == Schedule::Occurrence::Daily)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Daily");
        break;
      case 30:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every thirty days");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("Every %1 days").arg(mult));
    }
  else if (type == Schedule::Occurrence::Weekly)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Weekly");
        break;
      case 2:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every other week");
        break;
      case 3:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every three weeks");
        break;
      case 4:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every four weeks");
        break;
      case 8:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every eight weeks");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("Every %1 weeks").arg(mult));
    }
  else if (type == Schedule::Occurrence::EveryHalfMonth)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every half month");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("Every %1 half months").arg(mult));
    }
  else if (type == Schedule::Occurrence::Monthly)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Monthly");
        break;
      case 2:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every two months");
        break;
      case 3:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every three months");
        break;
      case 4:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every four months");
        break;
      case 6:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Twice yearly");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("Every %1 months").arg(mult));
    }
  else if (type == Schedule::Occurrence::Yearly)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Yearly");
        break;
      case 2:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every other year");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("Every %1 years").arg(mult));
    }
  return occurrenceString;
}

QString MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence type)
{
  QString occurrenceString = I18N_NOOP2("Schedule occurrence period", "Any");

  if (type == Schedule::Occurrence::Once)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Once");
  else if (type == Schedule::Occurrence::Daily)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Day");
  else if (type == Schedule::Occurrence::Weekly)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Week");
  else if (type == Schedule::Occurrence::EveryHalfMonth)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Half-month");
  else if (type == Schedule::Occurrence::Monthly)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Month");
  else if (type == Schedule::Occurrence::Yearly)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Year");
  return occurrenceString;
}

QString MyMoneySchedule::scheduleTypeToString(Schedule::Type type)
{
  QString text;

  switch (type) {
    case Schedule::Type::Bill:
      text = I18N_NOOP2("Scheduled transaction type", "Bill");
      break;
    case Schedule::Type::Deposit:
      text = I18N_NOOP2("Scheduled transaction type", "Deposit");
      break;
    case Schedule::Type::Transfer:
      text = I18N_NOOP2("Scheduled transaction type", "Transfer");
      break;
    case Schedule::Type::LoanPayment:
      text = I18N_NOOP2("Scheduled transaction type", "Loan payment");
      break;
    case Schedule::Type::Any:
    default:
      text = I18N_NOOP2("Scheduled transaction type", "Unknown");
  }
  return text;
}


QString MyMoneySchedule::paymentMethodToString(Schedule::PaymentType paymentType)
{
  QString text;

  switch (paymentType) {
    case Schedule::PaymentType::DirectDebit:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Direct debit");
      break;
    case Schedule::PaymentType::DirectDeposit:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Direct deposit");
      break;
    case Schedule::PaymentType::ManualDeposit:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Manual deposit");
      break;
    case Schedule::PaymentType::Other:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Other");
      break;
    case Schedule::PaymentType::WriteChecque:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Write check");
      break;
    case Schedule::PaymentType::StandingOrder:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Standing order");
      break;
    case Schedule::PaymentType::BankTransfer:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Bank transfer");
      break;
    case Schedule::PaymentType::Any:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Any (Error)");
      break;
  }
  return text;
}

QString MyMoneySchedule::weekendOptionToString(Schedule::WeekendOption weekendOption)
{
  QString text;

  switch (weekendOption) {
    case Schedule::WeekendOption::MoveBefore:
      text = I18N_NOOP("Change the date to the previous processing day");
      break;
    case Schedule::WeekendOption::MoveAfter:
      text = I18N_NOOP("Change the date to the next processing day");
      break;
    case Schedule::WeekendOption::MoveNothing:
      text = I18N_NOOP("Do not change the date");
      break;
  }
  return text;
}

// until we don't have the means to store the value
// of the variation, we default to 10% in case this
// scheduled transaction is marked 'not fixed'.
//
// ipwizard 2009-04-18

int MyMoneySchedule::variation() const
{
  int rc = 0;
  if (!isFixed()) {
    rc = 10;
#if 0
    QString var = value("kmm-variation");
    if (!var.isEmpty())
      rc = var.toInt();
#endif
  }
  return rc;
}

void MyMoneySchedule::setVariation(int var)
{
  Q_UNUSED(var)
#if 0
  deletePair("kmm-variation");
  if (var != 0)
    setValue("kmm-variation", QString("%1").arg(var));
#endif
}

int MyMoneySchedule::eventsPerYear(Schedule::Occurrence occurrence)
{
  int rc = 0;

  switch (occurrence) {
    case Schedule::Occurrence::Daily:
      rc = 365;
      break;
    case Schedule::Occurrence::Weekly:
      rc = 52;
      break;
    case Schedule::Occurrence::Fortnightly:
      rc = 26;
      break;
    case Schedule::Occurrence::EveryOtherWeek:
      rc = 26;
      break;
    case Schedule::Occurrence::EveryHalfMonth:
      rc = 24;
      break;
    case Schedule::Occurrence::EveryThreeWeeks:
      rc = 17;
      break;
    case Schedule::Occurrence::EveryFourWeeks:
      rc = 13;
      break;
    case Schedule::Occurrence::Monthly:
    case Schedule::Occurrence::EveryThirtyDays:
      rc = 12;
      break;
    case Schedule::Occurrence::EveryEightWeeks:
      rc = 6;
      break;
    case Schedule::Occurrence::EveryOtherMonth:
      rc = 6;
      break;
    case Schedule::Occurrence::EveryThreeMonths:
    case Schedule::Occurrence::Quarterly:
      rc = 4;
      break;
    case Schedule::Occurrence::EveryFourMonths:
      rc = 3;
      break;
    case Schedule::Occurrence::TwiceYearly:
      rc = 2;
      break;
    case Schedule::Occurrence::Yearly:
      rc = 1;
      break;
    default:
      qWarning("Occurrence not supported by financial calculator");
  }

  return rc;
}

int MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence occurrence)
{
  int rc = 0;

  switch (occurrence) {
    case Schedule::Occurrence::Daily:
      rc = 1;
      break;
    case Schedule::Occurrence::Weekly:
      rc = 7;
      break;
    case Schedule::Occurrence::Fortnightly:
      rc = 14;
      break;
    case Schedule::Occurrence::EveryOtherWeek:
      rc = 14;
      break;
    case Schedule::Occurrence::EveryHalfMonth:
      rc = 15;
      break;
    case Schedule::Occurrence::EveryThreeWeeks:
      rc = 21;
      break;
    case Schedule::Occurrence::EveryFourWeeks:
      rc = 28;
      break;
    case Schedule::Occurrence::EveryThirtyDays:
      rc = 30;
      break;
    case Schedule::Occurrence::Monthly:
      rc = 30;
      break;
    case Schedule::Occurrence::EveryEightWeeks:
      rc = 56;
      break;
    case Schedule::Occurrence::EveryOtherMonth:
      rc = 60;
      break;
    case Schedule::Occurrence::EveryThreeMonths:
    case Schedule::Occurrence::Quarterly:
      rc = 90;
      break;
    case Schedule::Occurrence::EveryFourMonths:
      rc = 120;
      break;
    case Schedule::Occurrence::TwiceYearly:
      rc = 180;
      break;
    case Schedule::Occurrence::Yearly:
      rc = 360;
      break;
    default:
      qWarning("Occurrence not supported by financial calculator");
  }

  return rc;
}

QDate MyMoneySchedule::addHalfMonths(QDate date, int mult) const
{
  QDate newdate = date;
  int d, dm;
  if (mult > 0) {
    d = newdate.day();
    if (d <= 12) {
      if (mult % 2 == 0)
        newdate = newdate.addMonths(mult >> 1);
      else
        newdate = newdate.addMonths(mult >> 1).addDays(15);
    } else
      for (int i = 0; i < mult; i++) {
        if (d <= 13)
          newdate = newdate.addDays(15);
        else {
          dm = newdate.daysInMonth();
          if (d == 14)
            newdate = newdate.addDays((dm < 30) ? dm - d : 15);
          else if (d == 15)
            newdate = newdate.addDays(dm - d);
          else if (d == dm)
            newdate = newdate.addDays(15 - d).addMonths(1);
          else
            newdate = newdate.addDays(-15).addMonths(1);
        }
        d = newdate.day();
      }
  } else if (mult < 0)  // Go backwards
    for (int i = 0; i > mult; i--) {
      d = newdate.day();
      dm = newdate.daysInMonth();
      if (d > 15) {
        dm = newdate.daysInMonth();
        newdate = newdate.addDays((d == dm) ? 15 - dm : -15);
      } else if (d <= 13)
        newdate = newdate.addMonths(-1).addDays(15);
      else if (d == 15)
        newdate = newdate.addDays(-15);
      else { // 14
        newdate = newdate.addMonths(-1);
        dm = newdate.daysInMonth();
        newdate = newdate.addDays((dm < 30) ? dm - d : 15);
      }
    }
  return newdate;
}

/**
  * Helper method to convert simple occurrence to compound occurrence + multiplier
  *
  * @param multiplier Returned by reference.  Adjusted multiplier
  * @param occurrence Returned by reference.  Occurrence type
  */
void MyMoneySchedule::simpleToCompoundOccurrence(int& multiplier, Schedule::Occurrence& occurrence)
{
  Schedule::Occurrence newOcc = occurrence;
  int newMulti = 1;
  if (occurrence == Schedule::Occurrence::Once ||
      occurrence == Schedule::Occurrence::Daily ||
      occurrence == Schedule::Occurrence::Weekly ||
      occurrence == Schedule::Occurrence::EveryHalfMonth ||
      occurrence == Schedule::Occurrence::Monthly ||
      occurrence == Schedule::Occurrence::Yearly) { // Already a base occurrence and multiplier
  } else if (occurrence == Schedule::Occurrence::Fortnightly ||
             occurrence == Schedule::Occurrence::EveryOtherWeek) {
    newOcc    = Schedule::Occurrence::Weekly;
    newMulti  = 2;
  } else if (occurrence == Schedule::Occurrence::EveryThreeWeeks) {
    newOcc    = Schedule::Occurrence::Weekly;
    newMulti  = 3;
  } else if (occurrence == Schedule::Occurrence::EveryFourWeeks) {
    newOcc    = Schedule::Occurrence::Weekly;
    newMulti  = 4;
  } else if (occurrence == Schedule::Occurrence::EveryThirtyDays) {
    newOcc    = Schedule::Occurrence::Daily;
    newMulti  = 30;
  } else if (occurrence == Schedule::Occurrence::EveryEightWeeks) {
    newOcc    = Schedule::Occurrence::Weekly;
    newMulti  = 8;
  } else if (occurrence == Schedule::Occurrence::EveryOtherMonth) {
    newOcc    = Schedule::Occurrence::Monthly;
    newMulti  = 2;
  } else if (occurrence == Schedule::Occurrence::EveryThreeMonths ||
             occurrence == Schedule::Occurrence::Quarterly) {
    newOcc    = Schedule::Occurrence::Monthly;
    newMulti  = 3;
  } else if (occurrence == Schedule::Occurrence::EveryFourMonths) {
    newOcc    = Schedule::Occurrence::Monthly;
    newMulti  = 4;
  } else if (occurrence == Schedule::Occurrence::TwiceYearly) {
    newOcc    = Schedule::Occurrence::Monthly;
    newMulti  = 6;
  } else if (occurrence == Schedule::Occurrence::EveryOtherYear) {
    newOcc    = Schedule::Occurrence::Yearly;
    newMulti  = 2;
  } else { // Unknown
    newOcc    = Schedule::Occurrence::Any;
    newMulti  = 1;
  }
  if (newOcc != occurrence) {
    occurrence   = newOcc;
    multiplier  = newMulti == 1 ? multiplier : newMulti * multiplier;
  }
}

/**
  * Helper method to convert compound occurrence + multiplier to simple occurrence
  *
  * @param multiplier Returned by reference.  Adjusted multiplier
  * @param occurrence Returned by reference.  Occurrence type
  */
void MyMoneySchedule::compoundToSimpleOccurrence(int& multiplier, Schedule::Occurrence& occurrence)
{
  Schedule::Occurrence newOcc = occurrence;
  if (occurrence == Schedule::Occurrence::Once) { // Nothing to do
  } else if (occurrence == Schedule::Occurrence::Daily) {
    switch (multiplier) {
      case 1:
        break;
      case 30:
        newOcc = Schedule::Occurrence::EveryThirtyDays;
        break;
    }
  } else if (newOcc == Schedule::Occurrence::Weekly) {
    switch (multiplier) {
      case 1:
        break;
      case 2:
        newOcc = Schedule::Occurrence::EveryOtherWeek;
        break;
      case 3:
        newOcc = Schedule::Occurrence::EveryThreeWeeks;
        break;
      case 4:
        newOcc = Schedule::Occurrence::EveryFourWeeks;
        break;
      case 8:
        newOcc = Schedule::Occurrence::EveryEightWeeks;
        break;
    }
  } else if (occurrence == Schedule::Occurrence::Monthly)
    switch (multiplier) {
      case 1:
        break;
      case 2:
        newOcc = Schedule::Occurrence::EveryOtherMonth;
        break;
      case 3:
        newOcc = Schedule::Occurrence::EveryThreeMonths;
        break;
      case 4:
        newOcc = Schedule::Occurrence::EveryFourMonths;
        break;
      case 6:
        newOcc = Schedule::Occurrence::TwiceYearly;
        break;
    }
  else if (occurrence == Schedule::Occurrence::EveryHalfMonth)
    switch (multiplier) {
      case 1:
        break;
    }
  else if (occurrence == Schedule::Occurrence::Yearly) {
    switch (multiplier) {
      case 1:
        break;
      case 2:
        newOcc = Schedule::Occurrence::EveryOtherYear;
        break;
    }
  }
  if (occurrence != newOcc) { // Changed to derived type
    occurrence = newOcc;
    multiplier = 1;
  }
}

void MyMoneySchedule::setProcessingCalendar(IMyMoneyProcessingCalendar* pc)
{
  processingCalendarPtr = pc;
}

bool MyMoneySchedule::isProcessingDate(const QDate& date) const
{
  if (processingCalendarPtr)
    return processingCalendarPtr->isProcessingDate(date);

  /// @todo test against m_processingDays instead?  (currently only for tests)
  return date.dayOfWeek() < Qt::Saturday;
}

IMyMoneyProcessingCalendar* MyMoneySchedule::processingCalendar() const
{
  return processingCalendarPtr;
}

bool MyMoneySchedule::replaceId(const QString& newId, const QString& oldId)
{
  Q_D(MyMoneySchedule);
  return d->m_transaction.replaceId(newId, oldId);
}
