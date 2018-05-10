/***************************************************************************
                          mymoneyschedule.cpp
                             -------------------
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyschedule.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "imymoneyprocessingcalendar.h"

static IMyMoneyProcessingCalendar* processingCalendarPtr = 0;

MyMoneySchedule::MyMoneySchedule() :
    MyMoneyObject()
{
  // Set up the default values
  m_occurrence = OCCUR_ANY;
  m_occurrenceMultiplier = 1;
  m_type = TYPE_ANY;
  m_paymentType = STYPE_ANY;
  m_fixed = false;
  m_lastDayInMonth = false;
  m_autoEnter = false;
  m_startDate = QDate();
  m_endDate = QDate();
  m_lastPayment = QDate();
  m_weekendOption = MoveNothing;
}

MyMoneySchedule::MyMoneySchedule(const QString& name, typeE type,
                                 occurrenceE occurrence, int occurrenceMultiplier,
                                 paymentTypeE paymentType,
                                 const QDate& /* startDate */,
                                 const QDate& endDate,
                                 bool fixed, bool autoEnter) :
    MyMoneyObject()
{
  // Set up the default values
  m_name = name;
  m_occurrence = occurrence;
  m_occurrenceMultiplier = occurrenceMultiplier;
  simpleToCompoundOccurrence(m_occurrenceMultiplier, m_occurrence);
  m_type = type;
  m_paymentType = paymentType;
  m_fixed = fixed;
  m_lastDayInMonth = false;
  m_autoEnter = autoEnter;
  m_startDate = QDate();
  m_endDate = endDate;
  m_lastPayment = QDate();
  m_weekendOption = MoveNothing;
}

MyMoneySchedule::MyMoneySchedule(const QDomElement& node) :
    MyMoneyObject(node)
{
  if ("SCHEDULED_TX" != node.tagName())
    throw MYMONEYEXCEPTION("Node was not SCHEDULED_TX");

  m_name = node.attribute("name");
  m_startDate = stringToDate(node.attribute("startDate"));
  m_endDate = stringToDate(node.attribute("endDate"));
  m_lastPayment = stringToDate(node.attribute("lastPayment"));

  m_type = static_cast<MyMoneySchedule::typeE>(node.attribute("type").toInt());
  m_paymentType = static_cast<MyMoneySchedule::paymentTypeE>(node.attribute("paymentType").toInt());
  m_occurrence = static_cast<MyMoneySchedule::occurrenceE>(node.attribute("occurence").toInt()); // krazy:exclude=spelling
  m_occurrenceMultiplier = node.attribute("occurenceMultiplier", "1").toInt(); // krazy:exclude=spelling
  // Convert to compound occurrence
  simpleToCompoundOccurrence(m_occurrenceMultiplier, m_occurrence);
  m_lastDayInMonth = static_cast<bool>(node.attribute("lastDayInMonth").toInt());
  m_autoEnter = static_cast<bool>(node.attribute("autoEnter").toInt());
  m_fixed = static_cast<bool>(node.attribute("fixed").toInt());
  m_weekendOption = static_cast<MyMoneySchedule::weekendOptionE>(node.attribute("weekendOption").toInt());

  // read in the associated transaction
  QDomNodeList nodeList = node.elementsByTagName("TRANSACTION");
  if (nodeList.count() == 0)
    throw MYMONEYEXCEPTION("SCHEDULED_TX has no TRANSACTION node");

  setTransaction(MyMoneyTransaction(nodeList.item(0).toElement(), false), true);

  // some old versions did not remove the entry date and post date fields
  // in the schedule. So if this is the case, we deal with a very old transaction
  // and can't use the post date field as next due date. Hence, we wipe it out here
  if (m_transaction.entryDate().isValid()) {
    m_transaction.setPostDate(QDate());
    m_transaction.setEntryDate(QDate());
  }

  // readin the recorded payments
  nodeList = node.elementsByTagName("PAYMENTS");
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName("PAYMENT");
    for (int i = 0; i < nodeList.count(); ++i) {
      m_recordedPayments << stringToDate(nodeList.item(i).toElement().attribute("date"));
    }
  }

  // if the next due date is not set (comes from old version)
  // then set it up the old way
  if (!nextDueDate().isValid() && !m_lastPayment.isValid()) {
    m_transaction.setPostDate(m_startDate);
    // clear it, because the schedule has never been used
    m_startDate = QDate();
  }

  // There are reports that lastPayment and nextDueDate are identical or
  // that nextDueDate is older than lastPayment. This could
  // be caused by older versions of the application. In this case, we just
  // clear out the nextDueDate and let it calculate from the lastPayment.
  if (nextDueDate().isValid() && nextDueDate() <= m_lastPayment) {
    m_transaction.setPostDate(QDate());
  }

  if (!nextDueDate().isValid()) {
    m_transaction.setPostDate(m_startDate);
    m_transaction.setPostDate(nextPayment(m_lastPayment.addDays(1)));
  }
}

MyMoneySchedule::MyMoneySchedule(const QString& id, const MyMoneySchedule& right) :
    MyMoneyObject(id)
{
  *this = right;
  setId(id);
}

MyMoneySchedule::occurrenceE MyMoneySchedule::occurrence() const
{
  MyMoneySchedule::occurrenceE occ = m_occurrence;
  int mult = m_occurrenceMultiplier;
  compoundToSimpleOccurrence(mult, occ);
  return occ;
}

void MyMoneySchedule::setStartDate(const QDate& date)
{
  m_startDate = date;
}

void MyMoneySchedule::setPaymentType(paymentTypeE type)
{
  m_paymentType = type;
}

void MyMoneySchedule::setFixed(bool fixed)
{
  m_fixed = fixed;
}

void MyMoneySchedule::setTransaction(const MyMoneyTransaction& transaction)
{
  setTransaction(transaction, false);
}

void MyMoneySchedule::setTransaction(const MyMoneyTransaction& transaction, bool noDateCheck)
{
  MyMoneyTransaction t = transaction;
  if (!noDateCheck) {
    // don't allow a transaction that has no due date
    // if we get something like that, then we use the
    // the current next due date. If that is also invalid
    // we can't help it.
    if (!t.postDate().isValid()) {
      t.setPostDate(m_transaction.postDate());
    }

    if (!t.postDate().isValid())
      return;
  }

  // make sure to clear out some unused information in scheduled transactions
  // we need to do this for the case that the transaction passed as argument
  // is a matched or imported transaction.
  QList<MyMoneySplit> splits = t.splits();
  if (splits.count() > 0) {
    QList<MyMoneySplit>::const_iterator it_s;
    for (it_s = splits.constBegin(); it_s != splits.constEnd(); ++it_s) {
      MyMoneySplit s = *it_s;
      // clear out the bankID
      if (!(*it_s).bankID().isEmpty()) {
        s.setBankID(QString());
        t.modifySplit(s);
      }

      // only clear payees from second split onwards
      if (it_s == splits.constBegin())
        continue;

      if (!(*it_s).payeeId().isEmpty()) {
        // but only if the split references an income/expense category
        MyMoneyFile* file = MyMoneyFile::instance();
        // some unit tests don't have a storage attached, so we
        // simply skip the test
        // Don't check for accounts with an id of 'Phony-ID' which is used
        // internally for non-existing accounts (during creation of accounts)
        if (file->storageAttached() && s.accountId() != QString("Phony-ID")) {
          MyMoneyAccount acc = file->account(s.accountId());
          if (acc.isIncomeExpense()) {
            s.setPayeeId(QString());
            t.modifySplit(s);
          }
        }
      }
    }
  }

  m_transaction = t;
  // make sure that the transaction does not have an id so that we can enter
  // it into the engine
  m_transaction.clearId();
}

void MyMoneySchedule::setEndDate(const QDate& date)
{
  m_endDate = date;
}

void MyMoneySchedule::setLastDayInMonth(bool state)
{
  m_lastDayInMonth = state;
}

void MyMoneySchedule::setAutoEnter(bool autoenter)
{
  m_autoEnter = autoenter;
}

const QDate& MyMoneySchedule::startDate() const
{
  if (m_startDate.isValid())
    return m_startDate;
  return nextDueDate();
}

const QDate& MyMoneySchedule::nextDueDate() const
{
  return m_transaction.postDate();
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

QDate MyMoneySchedule::adjustedDate(QDate date, weekendOptionE option) const
{
  if (!date.isValid() || option == MyMoneySchedule::MoveNothing || isProcessingDate(date))
    return date;

  int step = 1;
  if (option == MyMoneySchedule::MoveBefore)
    step = -1;

  while (!isProcessingDate(date))
    date = date.addDays(step);

  return date;
}

void MyMoneySchedule::setNextDueDate(const QDate& date)
{
  if (date.isValid()) {
    m_transaction.setPostDate(date);
    // m_startDate = date;
  }
}

void MyMoneySchedule::setLastPayment(const QDate& date)
{
  // Delete all payments older than date
  QList<QDate>::Iterator it;
  QList<QDate> delList;

  for (it = m_recordedPayments.begin(); it != m_recordedPayments.end(); ++it) {
    if (*it < date || !date.isValid())
      delList.append(*it);
  }

  for (it = delList.begin(); it != delList.end(); ++it) {
    m_recordedPayments.removeAll(*it);
  }

  m_lastPayment = date;
  if (!m_startDate.isValid())
    m_startDate = date;
}

void MyMoneySchedule::setName(const QString& nm)
{
  m_name = nm;
}

void MyMoneySchedule::setOccurrence(occurrenceE occ)
{
  MyMoneySchedule::occurrenceE occ2 = occ;
  int mult = 1;
  simpleToCompoundOccurrence(mult, occ2);
  setOccurrencePeriod(occ2);
  setOccurrenceMultiplier(mult);
}

void MyMoneySchedule::setOccurrencePeriod(occurrenceE occ)
{
  m_occurrence = occ;
}

void MyMoneySchedule::setOccurrenceMultiplier(int occmultiplier)
{
  m_occurrenceMultiplier = occmultiplier < 1 ? 1 : occmultiplier;
}

void MyMoneySchedule::setType(typeE type)
{
  m_type = type;
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
  if (id_check && !m_id.isEmpty())
    throw MYMONEYEXCEPTION("ID for schedule not empty when required");

  if (m_occurrence == OCCUR_ANY)
    throw MYMONEYEXCEPTION("Invalid occurrence type for schedule");

  if (m_type == TYPE_ANY)
    throw MYMONEYEXCEPTION("Invalid type for schedule");

  if (!nextDueDate().isValid())
    throw MYMONEYEXCEPTION("Invalid next due date for schedule");

  if (m_paymentType == STYPE_ANY)
    throw MYMONEYEXCEPTION("Invalid payment type for schedule");

  if (m_transaction.splitCount() == 0)
    throw MYMONEYEXCEPTION("Scheduled transaction does not contain splits");

  // Check the payment types
  switch (m_type) {
    case TYPE_BILL:
      if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
        throw MYMONEYEXCEPTION("Invalid payment type for bills");
      break;

    case TYPE_DEPOSIT:
      if (m_paymentType == STYPE_DIRECTDEBIT || m_paymentType == STYPE_WRITECHEQUE)
        throw MYMONEYEXCEPTION("Invalid payment type for deposits");
      break;

    case TYPE_ANY:
      throw MYMONEYEXCEPTION("Invalid type ANY");
      break;

    case TYPE_TRANSFER:
//        if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
//          return false;
      break;

    case TYPE_LOANPAYMENT:
      break;
  }
}

QDate MyMoneySchedule::adjustedNextPayment(const QDate& refDate) const
{
  return nextPaymentDate(true, refDate);
}

QDate MyMoneySchedule::nextPayment(const QDate& refDate) const
{
  return nextPaymentDate(false, refDate);
}

QDate MyMoneySchedule::nextPaymentDate(const bool& adjust, const QDate& refDate) const
{
  weekendOptionE option(adjust ? weekendOption() :
                        MyMoneySchedule::MoveNothing);

  QDate adjEndDate(adjustedDate(m_endDate, option));

  // if the enddate is valid and it is before the reference date,
  // then there will be no more payments.
  if (adjEndDate.isValid() && adjEndDate < refDate) {
    return QDate();
  }

  QDate dueDate(nextDueDate());
  QDate paymentDate(adjustedDate(dueDate, option));

  if (paymentDate.isValid() &&
      (paymentDate <= refDate || m_recordedPayments.contains(dueDate))) {
    switch (m_occurrence) {
      case OCCUR_ONCE:
        // If the lastPayment is already set or the payment should have been
        // prior to the reference date then invalidate the payment date.
        if (m_lastPayment.isValid() || paymentDate <= refDate)
          paymentDate = QDate();
        break;

      case OCCUR_DAILY: {
          int step = m_occurrenceMultiplier;
          do {
            dueDate = dueDate.addDays(step);
            paymentDate = adjustedDate(dueDate, option);
          } while (paymentDate.isValid() &&
                   (paymentDate <= refDate ||
                    m_recordedPayments.contains(dueDate)));
        }
        break;

      case OCCUR_WEEKLY: {
          int step = 7 * m_occurrenceMultiplier;
          do {
            dueDate = dueDate.addDays(step);
            paymentDate = adjustedDate(dueDate, option);
          } while (paymentDate.isValid() &&
                   (paymentDate <= refDate ||
                    m_recordedPayments.contains(dueDate)));
        }
        break;

      case OCCUR_EVERYHALFMONTH:
        do {
          dueDate = addHalfMonths(dueDate, m_occurrenceMultiplier);
          paymentDate = adjustedDate(dueDate, option);
        } while (paymentDate.isValid() &&
                 (paymentDate <= refDate ||
                  m_recordedPayments.contains(dueDate)));
        break;

      case OCCUR_MONTHLY:
        do {
          dueDate = dueDate.addMonths(m_occurrenceMultiplier);
          fixDate(dueDate);
          paymentDate = adjustedDate(dueDate, option);
        } while (paymentDate.isValid() &&
                 (paymentDate <= refDate ||
                  m_recordedPayments.contains(dueDate)));
        break;

      case OCCUR_YEARLY:
        do {
          dueDate = dueDate.addYears(m_occurrenceMultiplier);
          fixDate(dueDate);
          paymentDate = adjustedDate(dueDate, option);
        } while (paymentDate.isValid() &&
                 (paymentDate <= refDate ||
                  m_recordedPayments.contains(dueDate)));
        break;

      case OCCUR_ANY:
      default:
        paymentDate = QDate();
        break;
    }
  }
  if (paymentDate.isValid() && adjEndDate.isValid() && paymentDate > adjEndDate)
    paymentDate = QDate();

  return paymentDate;
}

QList<QDate> MyMoneySchedule::paymentDates(const QDate& _startDate, const QDate& _endDate) const
{
  QDate paymentDate(nextDueDate());
  QList<QDate> theDates;

  weekendOptionE option(weekendOption());

  QDate endDate(_endDate);
  if (willEnd() && m_endDate < endDate) {
    // consider the adjusted end date instead of the plain end date
    endDate = adjustedDate(m_endDate, option);
  }

  QDate start_date(adjustedDate(startDate(), option));
  // if the period specified by the parameters and the adjusted period
  // defined for this schedule don't overlap, then the list remains empty
  if ((willEnd() && adjustedDate(m_endDate, option) < _startDate)
      || start_date > endDate)
    return theDates;

  QDate date(adjustedDate(paymentDate, option));

  switch (m_occurrence) {
    case OCCUR_ONCE:
      if (start_date >= _startDate && start_date <= endDate)
        theDates.append(start_date);
      break;

    case OCCUR_DAILY:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = paymentDate.addDays(m_occurrenceMultiplier);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case OCCUR_WEEKLY: {
        int step = 7 * m_occurrenceMultiplier;
        while (date.isValid() && (date <= endDate)) {
          if (date >= _startDate)
            theDates.append(date);
          paymentDate = paymentDate.addDays(step);
          date = adjustedDate(paymentDate, option);
        }
      }
      break;

    case OCCUR_EVERYHALFMONTH:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = addHalfMonths(paymentDate, m_occurrenceMultiplier);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case OCCUR_MONTHLY:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = paymentDate.addMonths(m_occurrenceMultiplier);
        fixDate(paymentDate);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case OCCUR_YEARLY:
      while (date.isValid() && (date <= endDate)) {
        if (date >= _startDate)
          theDates.append(date);
        paymentDate = paymentDate.addYears(m_occurrenceMultiplier);
        fixDate(paymentDate);
        date = adjustedDate(paymentDate, option);
      }
      break;

    case OCCUR_ANY:
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
  if (MyMoneyObject::operator==(right) &&
      m_occurrence == right.m_occurrence &&
      m_occurrenceMultiplier == right.m_occurrenceMultiplier &&
      m_type == right.m_type &&
      m_startDate == right.m_startDate &&
      m_paymentType == right.m_paymentType &&
      m_fixed == right.m_fixed &&
      m_transaction == right.m_transaction &&
      m_endDate == right.m_endDate &&
      m_lastDayInMonth == right.m_lastDayInMonth &&
      m_autoEnter == right.m_autoEnter &&
      m_lastPayment == right.m_lastPayment &&
      ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)))
    return true;
  return false;
}

int MyMoneySchedule::transactionsRemaining() const
{
  return transactionsRemainingUntil(adjustedDate(m_endDate, weekendOption()));
}

int MyMoneySchedule::transactionsRemainingUntil(const QDate& endDate) const
{
  int counter = 0;

  QDate startDate = m_lastPayment.isValid() ? m_lastPayment : m_startDate;
  if (startDate.isValid() && endDate.isValid()) {
    QList<QDate> dates = paymentDates(startDate, endDate);
    counter = dates.count();
  }
  return counter;
}

MyMoneyAccount MyMoneySchedule::account(int cnt) const
{
  QList<MyMoneySplit> splits = m_transaction.splits();
  QList<MyMoneySplit>::ConstIterator it;
  MyMoneyFile* file = MyMoneyFile::instance();
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

QDate MyMoneySchedule::dateAfter(int transactions) const
{
  int counter = 1;
  QDate paymentDate(startDate());

  if (transactions <= 0)
    return paymentDate;

  switch (m_occurrence) {
    case OCCUR_ONCE:
      break;

    case OCCUR_DAILY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(m_occurrenceMultiplier);
      break;

    case OCCUR_WEEKLY: {
        int step = 7 * m_occurrenceMultiplier;
        while (counter++ < transactions)
          paymentDate = paymentDate.addDays(step);
      }
      break;

    case OCCUR_EVERYHALFMONTH:
      paymentDate = addHalfMonths(paymentDate, m_occurrenceMultiplier * (transactions - 1));
      break;

    case OCCUR_MONTHLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(m_occurrenceMultiplier);
      break;

    case OCCUR_YEARLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addYears(m_occurrenceMultiplier);
      break;

    case OCCUR_ANY:
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
  if (!m_lastPayment.isValid())
    return false;

  if (m_endDate.isValid()) {
    if (m_lastPayment >= m_endDate
        || !nextDueDate().isValid()
        || nextDueDate() > m_endDate)
      return true;
  }

  // Check to see if its a once off payment
  if (m_occurrence == MyMoneySchedule::OCCUR_ONCE)
    return true;

  return false;
}

bool MyMoneySchedule::hasRecordedPayment(const QDate& date) const
{
  // m_lastPayment should always be > recordedPayments()
  if (m_lastPayment.isValid() && m_lastPayment >= date)
    return true;

  if (m_recordedPayments.contains(date))
    return true;

  return false;
}

void MyMoneySchedule::recordPayment(const QDate& date)
{
  m_recordedPayments.append(date);
}

void MyMoneySchedule::setWeekendOption(const weekendOptionE option)
{
  // make sure only valid values are used. Invalid defaults to MoveNothing.
  switch (option) {
    case MoveBefore:
    case MoveAfter:
      m_weekendOption = option;
      break;

    default:
      m_weekendOption = MoveNothing;
      break;
  }
}

void MyMoneySchedule::fixDate(QDate& date) const
{
  QDate fixDate(m_startDate);
  if (fixDate.isValid()
      && date.day() != fixDate.day()
      && QDate::isValid(date.year(), date.month(), fixDate.day())) {
    date = QDate(date.year(), date.month(), fixDate.day());
  }
}

void MyMoneySchedule::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("SCHEDULED_TX");

  writeBaseXML(document, el);

  el.setAttribute("name", m_name);
  el.setAttribute("type", m_type);
  el.setAttribute("occurence", m_occurrence); // krazy:exclude=spelling
  el.setAttribute("occurenceMultiplier", m_occurrenceMultiplier);
  el.setAttribute("paymentType", m_paymentType);
  el.setAttribute("startDate", dateToString(m_startDate));
  el.setAttribute("endDate", dateToString(m_endDate));
  el.setAttribute("fixed", m_fixed);
  el.setAttribute("lastDayInMonth", m_lastDayInMonth);
  el.setAttribute("autoEnter", m_autoEnter);
  el.setAttribute("lastPayment", dateToString(m_lastPayment));
  el.setAttribute("weekendOption", m_weekendOption);

  //store the payment history for this scheduled task.
  QList<QDate> payments = recordedPayments();
  QList<QDate>::ConstIterator it;
  QDomElement paymentsElement = document.createElement("PAYMENTS");
  for (it = payments.constBegin(); it != payments.constEnd(); ++it) {
    QDomElement paymentEntry = document.createElement("PAYMENT");
    paymentEntry.setAttribute("date", dateToString(*it));
    paymentsElement.appendChild(paymentEntry);
  }
  el.appendChild(paymentsElement);

  //store the transaction data for this task.
  m_transaction.writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneySchedule::hasReferenceTo(const QString& id) const
{
  return m_transaction.hasReferenceTo(id);
}

QString MyMoneySchedule::occurrenceToString() const
{
  return occurrenceToString(occurrenceMultiplier(), occurrencePeriod());
}

QString MyMoneySchedule::occurrenceToString(occurrenceE occurrence)
{
  QString occurrenceString = I18N_NOOP2("Frequency of schedule", "Any");

  if (occurrence == MyMoneySchedule::OCCUR_ONCE)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Once");
  else if (occurrence == MyMoneySchedule::OCCUR_DAILY)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Daily");
  else if (occurrence == MyMoneySchedule::OCCUR_WEEKLY)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Weekly");
  else if (occurrence == MyMoneySchedule::OCCUR_FORTNIGHTLY)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Fortnightly");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYOTHERWEEK)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every other week");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every half month");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every three weeks");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYFOURWEEKS)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every four weeks");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every thirty days");
  else if (occurrence == MyMoneySchedule::OCCUR_MONTHLY)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Monthly");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every eight weeks");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYOTHERMONTH)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every two months");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every three months");
  else if (occurrence == MyMoneySchedule::OCCUR_QUARTERLY)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Quarterly");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYFOURMONTHS)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every four months");
  else if (occurrence == MyMoneySchedule::OCCUR_TWICEYEARLY)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Twice yearly");
  else if (occurrence == MyMoneySchedule::OCCUR_YEARLY)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Yearly");
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYOTHERYEAR)
    occurrenceString = I18N_NOOP2("Frequency of schedule", "Every other year");
  return occurrenceString;
}

QString MyMoneySchedule::occurrenceToString(int mult, occurrenceE type)
{
  QString occurrenceString = I18N_NOOP2("Frequency of schedule", "Any");

  if (type == MyMoneySchedule::OCCUR_ONCE)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Once");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("%1 times").arg(mult));
    }
  else if (type == MyMoneySchedule::OCCUR_DAILY)
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
  else if (type == MyMoneySchedule::OCCUR_WEEKLY)
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
  else if (type == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    switch (mult) {
      case 1:
        occurrenceString = I18N_NOOP2("Frequency of schedule", "Every half month");
        break;
      default:
        occurrenceString = I18N_NOOP2("Frequency of schedule", QString("Every %1 half months").arg(mult));
    }
  else if (type == MyMoneySchedule::OCCUR_MONTHLY)
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
  else if (type == MyMoneySchedule::OCCUR_YEARLY)
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

QString MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::occurrenceE type)
{
  QString occurrenceString = I18N_NOOP2("Schedule occurrence period", "Any");

  if (type == MyMoneySchedule::OCCUR_ONCE)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Once");
  else if (type == MyMoneySchedule::OCCUR_DAILY)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Day");
  else if (type == MyMoneySchedule::OCCUR_WEEKLY)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Week");
  else if (type == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Half-month");
  else if (type == MyMoneySchedule::OCCUR_MONTHLY)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Month");
  else if (type == MyMoneySchedule::OCCUR_YEARLY)
    occurrenceString = I18N_NOOP2("Schedule occurrence period", "Year");
  return occurrenceString;
}

QString MyMoneySchedule::scheduleTypeToString(MyMoneySchedule::typeE type)
{
  QString text;

  switch (type) {
    case MyMoneySchedule::TYPE_BILL:
      text = I18N_NOOP2("Scheduled transaction type", "Bill");
      break;
    case MyMoneySchedule::TYPE_DEPOSIT:
      text = I18N_NOOP2("Scheduled transaction type", "Deposit");
      break;
    case MyMoneySchedule::TYPE_TRANSFER:
      text = I18N_NOOP2("Scheduled transaction type", "Transfer");
      break;
    case MyMoneySchedule::TYPE_LOANPAYMENT:
      text = I18N_NOOP2("Scheduled transaction type", "Loan payment");
      break;
    case MyMoneySchedule::TYPE_ANY:
    default:
      text = I18N_NOOP2("Scheduled transaction type", "Unknown");
  }
  return text;
}


QString MyMoneySchedule::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
{
  QString text;

  switch (paymentType) {
    case MyMoneySchedule::STYPE_DIRECTDEBIT:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Direct debit");
      break;
    case MyMoneySchedule::STYPE_DIRECTDEPOSIT:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Direct deposit");
      break;
    case MyMoneySchedule::STYPE_MANUALDEPOSIT:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Manual deposit");
      break;
    case MyMoneySchedule::STYPE_OTHER:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Other");
      break;
    case MyMoneySchedule::STYPE_WRITECHEQUE:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Write check");
      break;
    case MyMoneySchedule::STYPE_STANDINGORDER:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Standing order");
      break;
    case MyMoneySchedule::STYPE_BANKTRANSFER:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Bank transfer");
      break;
    case MyMoneySchedule::STYPE_ANY:
      text = I18N_NOOP2("Scheduled Transaction payment type", "Any (Error)");
      break;
  }
  return text;
}

QString MyMoneySchedule::weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption)
{
  QString text;

  switch (weekendOption) {
    case MyMoneySchedule::MoveBefore:
      text = I18N_NOOP("Change the date to the previous processing day");
      break;
    case MyMoneySchedule::MoveAfter:
      text = I18N_NOOP("Change the date to the next processing day");
      break;
    case MyMoneySchedule::MoveNothing:
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

int MyMoneySchedule::eventsPerYear(MyMoneySchedule::occurrenceE occurrence)
{
  int rc = 0;

  switch (occurrence) {
    case MyMoneySchedule::OCCUR_DAILY:
      rc = 365;
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      rc = 52;
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      rc = 26;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      rc = 26;
      break;
    case MyMoneySchedule::OCCUR_EVERYHALFMONTH:
      rc = 24;
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEWEEKS:
      rc = 17;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      rc = 13;
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
    case MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS:
      rc = 12;
      break;
    case MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS:
      rc = 6;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      rc = 6;
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEMONTHS:
    case MyMoneySchedule::OCCUR_QUARTERLY:
      rc = 4;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      rc = 3;
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      rc = 2;
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      rc = 1;
      break;
    default:
      qWarning("Occurrence not supported by financial calculator");
  }

  return rc;
}

int MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::occurrenceE occurrence)
{
  int rc = 0;

  switch (occurrence) {
    case MyMoneySchedule::OCCUR_DAILY:
      rc = 1;
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      rc = 7;
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      rc = 14;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      rc = 14;
      break;
    case MyMoneySchedule::OCCUR_EVERYHALFMONTH:
      rc = 15;
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEWEEKS:
      rc = 21;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      rc = 28;
      break;
    case MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS:
      rc = 30;
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      rc = 30;
      break;
    case MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS:
      rc = 56;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      rc = 60;
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEMONTHS:
    case MyMoneySchedule::OCCUR_QUARTERLY:
      rc = 90;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      rc = 120;
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      rc = 180;
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
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
void MyMoneySchedule::simpleToCompoundOccurrence(int& multiplier, occurrenceE& occurrence)
{
  occurrenceE newOcc = occurrence;
  int newMulti = 1;
  if (occurrence == MyMoneySchedule::OCCUR_ONCE ||
      occurrence == MyMoneySchedule::OCCUR_DAILY ||
      occurrence == MyMoneySchedule::OCCUR_WEEKLY ||
      occurrence == MyMoneySchedule::OCCUR_EVERYHALFMONTH ||
      occurrence == MyMoneySchedule::OCCUR_MONTHLY ||
      occurrence == MyMoneySchedule::OCCUR_YEARLY) { // Already a base occurrence and multiplier
  } else if (occurrence == MyMoneySchedule::OCCUR_FORTNIGHTLY ||
             occurrence == MyMoneySchedule::OCCUR_EVERYOTHERWEEK) {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 2;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 3;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYFOURWEEKS) {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 4;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) {
    newOcc    = MyMoneySchedule::OCCUR_DAILY;
    newMulti  = 30;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 8;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYOTHERMONTH) {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 2;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS ||
             occurrence == MyMoneySchedule::OCCUR_QUARTERLY) {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 3;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYFOURMONTHS) {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 4;
  } else if (occurrence == MyMoneySchedule::OCCUR_TWICEYEARLY) {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 6;
  } else if (occurrence == MyMoneySchedule::OCCUR_EVERYOTHERYEAR) {
    newOcc    = MyMoneySchedule::OCCUR_YEARLY;
    newMulti  = 2;
  } else { // Unknown
    newOcc    = MyMoneySchedule::OCCUR_ANY;
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
void MyMoneySchedule::compoundToSimpleOccurrence(int& multiplier, occurrenceE& occurrence)
{
  occurrenceE newOcc = occurrence;
  if (occurrence == MyMoneySchedule::OCCUR_ONCE) { // Nothing to do
  } else if (occurrence == MyMoneySchedule::OCCUR_DAILY) {
    switch (multiplier) {
      case 1:
        break;
      case 30:
        newOcc = MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS;
        break;
    }
  } else if (newOcc == MyMoneySchedule::OCCUR_WEEKLY) {
    switch (multiplier) {
      case 1:
        break;
      case 2:
        newOcc = MyMoneySchedule::OCCUR_EVERYOTHERWEEK;
        break;
      case 3:
        newOcc = MyMoneySchedule::OCCUR_EVERYTHREEWEEKS;
        break;
      case 4:
        newOcc = MyMoneySchedule::OCCUR_EVERYFOURWEEKS;
        break;
      case 8:
        newOcc = MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS;
        break;
    }
  } else if (occurrence == MyMoneySchedule::OCCUR_MONTHLY)
    switch (multiplier) {
      case 1:
        break;
      case 2:
        newOcc = MyMoneySchedule::OCCUR_EVERYOTHERMONTH;
        break;
      case 3:
        newOcc = MyMoneySchedule::OCCUR_EVERYTHREEMONTHS;
        break;
      case 4:
        newOcc = MyMoneySchedule::OCCUR_EVERYFOURMONTHS;
        break;
      case 6:
        newOcc = MyMoneySchedule::OCCUR_TWICEYEARLY;
        break;
    }
  else if (occurrence == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    switch (multiplier) {
      case 1:
        break;
    }
  else if (occurrence == MyMoneySchedule::OCCUR_YEARLY) {
    switch (multiplier) {
      case 1:
        break;
      case 2:
        newOcc = MyMoneySchedule::OCCUR_EVERYOTHERYEAR;
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

  return date.dayOfWeek() < Qt::Saturday;
}

IMyMoneyProcessingCalendar* MyMoneySchedule::processingCalendar() const
{
  return processingCalendarPtr;
}

bool MyMoneySchedule::replaceId(const QString& newId, const QString& oldId)
{
  return m_transaction.replaceId(newId, oldId);
}

