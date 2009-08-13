/***************************************************************************
                          mymoneyscheduled.cpp
                             -------------------
    copyright            : (C) 2000-2002 by Michael Edwardes
                           (C) 2007 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyscheduled.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
//Added by qt3to4:
#include <Q3ValueList>

MyMoneySchedule::MyMoneySchedule() :
  MyMoneyObject()
{
  // Set up the default values
  m_occurence = OCCUR_ANY;
  m_occurenceMultiplier = 1;
  m_type = TYPE_ANY;
  m_paymentType = STYPE_ANY;
  m_fixed = false;
  m_autoEnter = false;
  m_startDate = QDate();
  m_endDate = QDate();
  m_lastPayment = QDate();
  m_weekendOption = MoveNothing;
}

MyMoneySchedule::MyMoneySchedule(const QString& name, typeE type,
                                 occurenceE occurence, int occurenceMultiplier,
                                 paymentTypeE paymentType,
                                 const QDate& /* startDate */,
                                 const QDate& endDate,
                                 bool fixed, bool autoEnter) :
  MyMoneyObject()
{
  // Set up the default values
  m_name = name;
  m_occurence = occurence;
  m_occurenceMultiplier = occurenceMultiplier;
  simpleToCompoundOccurence(m_occurenceMultiplier,m_occurence);
  m_type = type;
  m_paymentType = paymentType;
  m_fixed = fixed;
  m_autoEnter = autoEnter;
  m_startDate = QDate();
  m_endDate = endDate;
  m_lastPayment = QDate();
  m_weekendOption = MoveNothing;
}

MyMoneySchedule::MyMoneySchedule(const QDomElement& node) :
  MyMoneyObject(node)
{
  if("SCHEDULED_TX" != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not SCHEDULED_TX");

  m_name = node.attribute("name");
  m_startDate = stringToDate(node.attribute("startDate"));
  m_endDate = stringToDate(node.attribute("endDate"));
  m_lastPayment = stringToDate(node.attribute("lastPayment"));

  m_type = static_cast<MyMoneySchedule::typeE>(node.attribute("type").toInt());
  m_paymentType = static_cast<MyMoneySchedule::paymentTypeE>(node.attribute("paymentType").toInt());
  m_occurence = static_cast<MyMoneySchedule::occurenceE>(node.attribute("occurence").toInt());
  m_occurenceMultiplier = node.attribute("occurenceMultiplier", "1").toInt();
  // Convert to compound occurence
  simpleToCompoundOccurence(m_occurenceMultiplier,m_occurence);
  m_autoEnter = static_cast<bool>(node.attribute("autoEnter").toInt());
  m_fixed = static_cast<bool>(node.attribute("fixed").toInt());
  m_weekendOption = static_cast<MyMoneySchedule::weekendOptionE>(node.attribute("weekendOption").toInt());

  // read in the associated transaction
  QDomNodeList nodeList = node.elementsByTagName("TRANSACTION");
  if(nodeList.count() == 0)
    throw new MYMONEYEXCEPTION("SCHEDULED_TX has no TRANSACTION node");

  setTransaction(MyMoneyTransaction(nodeList.item(0).toElement(), false), true);

  // some old versions did not remove the entry date and post date fields
  // in the schedule. So if this is the case, we deal with a very old transaction
  // and can't use the post date field as next due date. Hence, we wipe it out here
  if(m_transaction.entryDate().isValid()) {
    m_transaction.setPostDate(QDate());
    m_transaction.setEntryDate(QDate());
  }

  // readin the recorded payments
  nodeList = node.elementsByTagName("PAYMENTS");
  if(nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName("PAYMENT");
    for(int i = 0; i < nodeList.count(); ++i) {
      m_recordedPayments << stringToDate(nodeList.item(i).toElement().attribute("date"));
    }
  }

  // if the next due date is not set (comes from old version)
  // then set it up the old way
  if(!nextDueDate().isValid() && !m_lastPayment.isValid()) {
    m_transaction.setPostDate(m_startDate);
    // clear it, because the schedule has never been used
    m_startDate = QDate();
  }

  // There are reports that lastPayment and nextDueDate are identical or
  // that nextDueDate is older than lastPayment. This could
  // be caused by older versions of the application. In this case, we just
  // clear out the nextDueDate and let it calculate from the lastPayment.
  if(nextDueDate().isValid() && nextDueDate() <= m_lastPayment) {
    m_transaction.setPostDate(QDate());
  }

  if(!nextDueDate().isValid()) {
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

MyMoneySchedule::occurenceE MyMoneySchedule::occurence(void) const
{
  MyMoneySchedule::occurenceE occ = m_occurence;
  int mult = m_occurenceMultiplier;
  compoundToSimpleOccurence(mult, occ);
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
  if(!noDateCheck) {
    // don't allow a transaction that has no due date
    // if we get something like that, then we use the
    // the current next due date. If that is also invalid
    // we can't help it.
    if(!t.postDate().isValid()) {
      t.setPostDate(m_transaction.postDate());
    }

    if(!t.postDate().isValid())
      return;
  }

  // make sure to clear out some unused information in scheduled transactions
  // we need to do this for the case that the transaction passed as argument
  // is a matched or imported transaction.
  Q3ValueList<MyMoneySplit> splits = t.splits();
  if(splits.count() > 0) {
    Q3ValueList<MyMoneySplit>::const_iterator it_s;
    for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      MyMoneySplit s = *it_s;
      // clear out the bankID
      if(!(*it_s).bankID().isEmpty()) {
        s.setBankID(QString());
        t.modifySplit(s);
      }

      // only clear payees from second split onwards
      if(it_s == splits.begin())
        continue;

      if(!(*it_s).payeeId().isEmpty()) {
        s.setPayeeId(QString());
        t.modifySplit(s);
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

void MyMoneySchedule::setAutoEnter(bool autoenter)
{
  m_autoEnter = autoenter;
}

const QDate& MyMoneySchedule::startDate(void) const
{
  if(m_startDate.isValid())
    return m_startDate;
  return nextDueDate();
}

const QDate& MyMoneySchedule::nextDueDate(void) const
{
  return m_transaction.postDate();
}

QDate MyMoneySchedule::adjustedNextDueDate(void) const
{
  QDate date(nextDueDate());

  if(isFinished())
    return QDate();

  if(weekendOption() != MyMoneySchedule::MoveNothing) {
    int dayOfWeek = date.dayOfWeek();
    if (dayOfWeek >= 6) {
      if (weekendOption() == MyMoneySchedule::MoveFriday) {
        if (dayOfWeek == 7)
          date = date.addDays(-2);
        else
          date = date.addDays(-1);
      } else {
        if (dayOfWeek == 6)
          date = date.addDays(2);
        else
          date = date.addDays(1);
      }
    }
  }
  return date;
}

void MyMoneySchedule::setNextDueDate(const QDate& date)
{
  m_transaction.setPostDate(date);
  if(date.isValid())
    m_startDate = date;
}

void MyMoneySchedule::setLastPayment(const QDate& date)
{
  // Delete all payments older than date
  Q3ValueList<QDate>::Iterator it;
  Q3ValueList<QDate> delList;

  for (it=m_recordedPayments.begin(); it!=m_recordedPayments.end(); ++it)
  {
    if (*it < date || !date.isValid())
      delList.append(*it);
  }

  for (it=delList.begin(); it!=delList.end(); ++it)
  {
    m_recordedPayments.remove(*it);
  }

  m_lastPayment = date;
  if(!m_startDate.isValid())
    m_startDate = date;
}

void MyMoneySchedule::setName(const QString& nm)
{
  m_name = nm;
}

void MyMoneySchedule::setOccurence(occurenceE occ)
{
  MyMoneySchedule::occurenceE occ2 = occ;
  int mult = 1;
  simpleToCompoundOccurence(mult, occ2);
  setOccurencePeriod( occ2 );
  setOccurenceMultiplier( mult );
}

void MyMoneySchedule::setOccurencePeriod(occurenceE occ)
{
  m_occurence = occ;
}

void MyMoneySchedule::setOccurenceMultiplier(int occmultiplier)
{
  m_occurenceMultiplier = occmultiplier < 1 ? 1 : occmultiplier;
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
   * m_occurence
   * m_type
   * m_startDate
   * m_paymentType
   * m_transaction
   *   the transaction must contain at least one split (two is better ;-)  )
   */
  if (id_check && !m_id.isEmpty())
    throw new MYMONEYEXCEPTION("ID for schedule not empty when required");

  if(m_occurence == OCCUR_ANY)
    throw new MYMONEYEXCEPTION("Invalid occurence type for schedule");

  if(m_type == TYPE_ANY)
    throw new MYMONEYEXCEPTION("Invalid type for schedule");

  if(!nextDueDate().isValid())
    throw new MYMONEYEXCEPTION("Invalid next due date for schedule");

  if(m_paymentType == STYPE_ANY)
    throw new MYMONEYEXCEPTION("Invalid payment type for schedule");

  if(m_transaction.splitCount() == 0)
    throw new MYMONEYEXCEPTION("Scheduled transaction does not contain splits");

  // Check the payment types
  switch (m_type)
  {
    case TYPE_BILL:
      if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
        throw new MYMONEYEXCEPTION("Invalid payment type for bills");
      break;

    case TYPE_DEPOSIT:
      if (m_paymentType == STYPE_DIRECTDEBIT || m_paymentType == STYPE_WRITECHEQUE)
        throw new MYMONEYEXCEPTION("Invalid payment type for deposits");
      break;

    case TYPE_ANY:
      throw new MYMONEYEXCEPTION("Invalid type ANY");
      break;

    case TYPE_TRANSFER:
//        if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
//          return false;
      break;

    case TYPE_LOANPAYMENT:
      break;
  }
}

QDate MyMoneySchedule::nextPayment(const QDate& refDate) const
{
#if 0
  QDate paymentDate(m_lastPayment);

  // if there never was a payment, then the next payment date must
  // be identical to the start date of the payments.

  if(!paymentDate.isValid()) {
    paymentDate = m_startDate;

    // if the reference date is invalid, then that's what we're looking for
    if(!refDate.isValid())
    {
      if (m_recordedPayments.contains(paymentDate))
        return QDate();

      return paymentDate;

    } else {
      // if the first payment date is past the given ref date,
      // then that's what we're looking for
      if(paymentDate > refDate)
        return paymentDate;
    }
  }
#endif

  // if the enddate is valid and it is before the reference date,
  // then there will be no more payments.
  if(m_endDate.isValid() && m_endDate < refDate) {
    return QDate();
  }

  QDate paymentDate(nextDueDate());

  if(refDate >= paymentDate) {
    switch (m_occurence)
    {
      case OCCUR_ONCE:
        // if the lastPayment is already set, then there will be no more payments
        // otherwise, the start date is the payment date
        if(m_lastPayment.isValid())
          return QDate();
        // if the only payment should have been prior to the reference date,
        // then don't show it
        if(nextDueDate() < refDate)
          return QDate();
        break;

      case OCCUR_DAILY:
        paymentDate = refDate.addDays(m_occurenceMultiplier);
        break;

      case OCCUR_WEEKLY:
        {
          int step = 7*m_occurenceMultiplier;
          do {
            paymentDate = paymentDate.addDays(step);
          }
          while (paymentDate <= refDate);
        }
        break;

      case OCCUR_EVERYHALFMONTH:
        do
        {
           paymentDate = addHalfMonths(paymentDate,m_occurenceMultiplier);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_MONTHLY:
        do {
          paymentDate = paymentDate.addMonths(m_occurenceMultiplier);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_YEARLY:
        do {
          paymentDate = paymentDate.addYears(m_occurenceMultiplier);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_ANY:
      default:
        paymentDate = QDate();
        break;
    }
  }
  if(paymentDate.isValid()) {
    if(m_endDate.isValid() && paymentDate > m_endDate)
      paymentDate = QDate();
  }

  if (paymentDate.isValid() && m_recordedPayments.contains(paymentDate))
    paymentDate = nextPayment(paymentDate);

  return paymentDate;
}

Q3ValueList<QDate> MyMoneySchedule::paymentDates(const QDate& _startDate, const QDate& _endDate) const
{
  QDate paymentDate(nextDueDate());
  Q3ValueList<QDate> theDates;

  QDate endDate(_endDate);
  if ( willEnd() && m_endDate < endDate )
    endDate = m_endDate;

  // if the period specified by the parameters and the period
  // defined for this schedule don't overlap, then the list remains empty
  if ((willEnd() && m_endDate < _startDate)
  || startDate() > endDate)
    return theDates;

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      if (startDate() >= _startDate && startDate() <= endDate)
        theDates.append(startDate());
      break;

    case OCCUR_DAILY:
      if (paymentDate < _startDate)
      {
        int daysMod = paymentDate.daysTo(_startDate) % m_occurenceMultiplier;
        paymentDate = (daysMod==0) ? _startDate : _startDate.addDays( m_occurenceMultiplier -daysMod );
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(m_occurenceMultiplier);
      }
      break;

    case OCCUR_WEEKLY:
      {
        int step = 7*m_occurenceMultiplier;
        while (paymentDate < _startDate)
          paymentDate = paymentDate.addDays(step);
        while (paymentDate <= endDate)
        {
          theDates.append(paymentDate);
          paymentDate = paymentDate.addDays(step);
        }
      }
      break;

    case OCCUR_EVERYHALFMONTH:
      while (paymentDate < _startDate)
      {
        paymentDate = addHalfMonths(paymentDate,m_occurenceMultiplier);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = addHalfMonths(paymentDate,m_occurenceMultiplier);
      }
      break;

    case OCCUR_MONTHLY:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addMonths(m_occurenceMultiplier);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addMonths(m_occurenceMultiplier);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_YEARLY:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addYears(m_occurenceMultiplier);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addYears(m_occurenceMultiplier);
        fixDate(paymentDate);
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
  if (  MyMoneyObject::operator==(right) &&
        m_occurence == right.m_occurence &&
        m_occurenceMultiplier == right.m_occurenceMultiplier &&
        m_type == right.m_type &&
        m_startDate == right.m_startDate &&
        m_paymentType == right.m_paymentType &&
        m_fixed == right.m_fixed &&
        m_transaction == right.m_transaction &&
        m_endDate == right.m_endDate &&
        m_autoEnter == right.m_autoEnter &&
        m_lastPayment == right.m_lastPayment &&
        ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)))
    return true;
  return false;
}

int MyMoneySchedule::transactionsRemaining(void) const
{
  int counter=0;

  if (m_endDate.isValid())
  {
    Q3ValueList<QDate> dates = paymentDates(m_lastPayment, m_endDate);
    // Dont include the last payment so -1
    counter = dates.count();
  }
  return counter;
}

MyMoneyAccount MyMoneySchedule::account(int cnt) const
{
  Q3ValueList<MyMoneySplit> splits = m_transaction.splits();
  Q3ValueList<MyMoneySplit>::ConstIterator it;
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc;

  // search the first asset or liability account
  for(it = splits.begin(); it != splits.end() && (acc.id().isEmpty() || cnt); ++it) {
    try {
      acc = file->account((*it).accountId());
      if(acc.isAssetLiability())
          --cnt;

      if(!cnt)
        return acc;
    } catch(MyMoneyException *e) {
      qWarning("Schedule '%s' references unknown account '%s'", qPrintable(id()),   qPrintable((*it).accountId()));
      delete e;
      return MyMoneyAccount();
    }
  }

  return MyMoneyAccount();
}

QDate MyMoneySchedule::dateAfter(int transactions) const
{
  int counter=1;
  QDate paymentDate(startDate());

  if (transactions<=0)
    return paymentDate;

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      break;

    case OCCUR_DAILY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(m_occurenceMultiplier);
      break;

    case OCCUR_WEEKLY:
      {
        int step = 7 * m_occurenceMultiplier;
        while (counter++ < transactions)
          paymentDate = paymentDate.addDays(step);
      }
      break;

    case OCCUR_EVERYHALFMONTH:
      paymentDate = addHalfMonths(paymentDate,m_occurenceMultiplier*(transactions-1));
      break;

    case OCCUR_MONTHLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(m_occurenceMultiplier);
      break;

    case OCCUR_YEARLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addYears(m_occurenceMultiplier);
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

  if(adjustedNextDueDate() >= QDate::currentDate())
    return false;

  return true;

#if 0
  bool bOverdue = true;

  // Check the payment dates first
  Q3ValueList<QDate> datesBeforeToday = paymentDates(m_startDate, QDate::currentDate().addDays(-1));
  if (datesBeforeToday.count() == 0)
  {
    bOverdue = false;
  }
  else if (datesBeforeToday.count() == 1)
  {
    if (nextPayment(m_lastPayment).isValid() &&
        (nextPayment(m_lastPayment) >= QDate::currentDate()))
      bOverdue = false;
  }
  else
  {
    // Check the dates
    // Remove all dates before m_lastPayment
    Q3ValueList<QDate> delList;
    Q3ValueList<QDate>::ConstIterator it;

    for (it=datesBeforeToday.begin(); it!=datesBeforeToday.end(); ++it)
    {
      if (*it <= m_lastPayment)
        delList.append(*it);
    }
    for (it=delList.begin(); it!=delList.end(); ++it)
    {
      datesBeforeToday.remove(*it);
    }

    // Remove nextPayment (lastPayments returns it?)
    if (datesBeforeToday.contains(nextPayment(m_lastPayment)))
      datesBeforeToday.remove(nextPayment(m_lastPayment));

    for (it=m_recordedPayments.begin(); it!=m_recordedPayments.end(); ++it)
    {
      if (datesBeforeToday.contains(*it))
        datesBeforeToday.remove(*it);
    }

    if (datesBeforeToday.contains(m_lastPayment))
      datesBeforeToday.remove(m_lastPayment);

    // Now finally check
    if (datesBeforeToday.count() == 0)
      bOverdue = false;
  }
  return bOverdue;
#endif
}

bool MyMoneySchedule::isFinished() const
{
  if(!m_lastPayment.isValid())
    return false;

  if (m_endDate.isValid()) {
    if(m_lastPayment >= m_endDate
    || !nextDueDate().isValid()
    || nextDueDate() > m_endDate)
      return true;
  }

  // Check to see if its a once off payment
  if (m_occurence == MyMoneySchedule::OCCUR_ONCE)
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
  switch(option) {
    case MoveFriday:
    case MoveMonday:
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
  if(fixDate.isValid()
  && date.day() != fixDate.day()
  && QDate::isValid(date.year(), date.month(), fixDate.day())) {
    date.setYMD(date.year(), date.month(), fixDate.day());
  }
}

void MyMoneySchedule::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("SCHEDULED_TX");

  writeBaseXML(document, el);

  el.setAttribute("name", m_name);
  el.setAttribute("type", m_type);
  el.setAttribute("occurence", m_occurence);
  el.setAttribute("occurenceMultiplier", m_occurenceMultiplier);
  el.setAttribute("paymentType", m_paymentType);
  el.setAttribute("startDate", dateToString(m_startDate));
  el.setAttribute("endDate", dateToString(m_endDate));
  el.setAttribute("fixed", m_fixed);
  el.setAttribute("autoEnter", m_autoEnter);
  el.setAttribute("lastPayment", dateToString(m_lastPayment));
  el.setAttribute("weekendOption", m_weekendOption);

  //store the payment history for this scheduled task.
  Q3ValueList<QDate> payments = recordedPayments();
  Q3ValueList<QDate>::ConstIterator it;
  QDomElement paymentsElement = document.createElement("PAYMENTS");
  for (it=payments.begin(); it!=payments.end(); ++it) {
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

QString MyMoneySchedule::occurenceToString() const
{
  return occurenceToString( occurenceMultiplier(), occurencePeriod() );
}

QString MyMoneySchedule::occurenceToString(occurenceE occurence)
{
  QString occurenceString = I18N_NOOP("Any");

  if(occurence == MyMoneySchedule::OCCUR_ONCE)
    occurenceString = I18N_NOOP("Once");
  else if(occurence == MyMoneySchedule::OCCUR_DAILY)
    occurenceString = I18N_NOOP("Daily");
  else if(occurence == MyMoneySchedule::OCCUR_WEEKLY)
    occurenceString = I18N_NOOP("Weekly");
  else if(occurence == MyMoneySchedule::OCCUR_FORTNIGHTLY)
    occurenceString = I18N_NOOP("Fortnightly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERWEEK)
    occurenceString = I18N_NOOP("Every other week");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    occurenceString = I18N_NOOP("Every half month");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS)
    occurenceString = I18N_NOOP("Every three weeks");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYFOURWEEKS)
    occurenceString = I18N_NOOP("Every four weeks");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS)
    occurenceString = I18N_NOOP("Every thirty days");
  else if(occurence == MyMoneySchedule::OCCUR_MONTHLY)
    occurenceString = I18N_NOOP("Monthly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS)
    occurenceString = I18N_NOOP("Every eight weeks");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERMONTH)
    occurenceString = I18N_NOOP("Every two months");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS)
    occurenceString = I18N_NOOP("Every three months");
  else if(occurence == MyMoneySchedule::OCCUR_QUARTERLY)
    occurenceString = I18N_NOOP("Quarterly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYFOURMONTHS)
    occurenceString = I18N_NOOP("Every four months");
  else if(occurence == MyMoneySchedule::OCCUR_TWICEYEARLY)
    occurenceString = I18N_NOOP("Twice yearly");
  else if(occurence == MyMoneySchedule::OCCUR_YEARLY)
    occurenceString = I18N_NOOP("Yearly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERYEAR)
    occurenceString = I18N_NOOP("Every other year");
  return occurenceString;
}

QString MyMoneySchedule::occurenceToString(int mult, occurenceE type)
{
  QString occurenceString = I18N_NOOP("Any");

  if (type == MyMoneySchedule::OCCUR_ONCE)
    switch (mult)
    {
      case 1: occurenceString = I18N_NOOP("Once"); break;
      default: occurenceString = I18N_NOOP(static_cast<QString>("%1 times").arg(mult));
    }
  else if(type == MyMoneySchedule::OCCUR_DAILY)
    switch (mult)
    {
      case 1: occurenceString = I18N_NOOP("Daily"); break;
      case 30: occurenceString = I18N_NOOP("Every thirty days"); break;
      default: occurenceString = I18N_NOOP(static_cast<QString>("Every %1 days").arg(mult));
    }
  else if(type == MyMoneySchedule::OCCUR_WEEKLY)
    switch (mult)
    {
      case 1: occurenceString = I18N_NOOP("Weekly"); break;
      case 2: occurenceString = I18N_NOOP("Every other week"); break;
      case 3: occurenceString = I18N_NOOP("Every three weeks"); break;
      case 4: occurenceString = I18N_NOOP("Every four weeks"); break;
      case 8: occurenceString = I18N_NOOP("Every eight weeks"); break;
      default: occurenceString = I18N_NOOP(static_cast<QString>("Every %1 weeks").arg(mult));
    }
  else if(type == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    switch (mult)
    {
      case 1: occurenceString = I18N_NOOP("Every half month"); break;
      default: occurenceString = I18N_NOOP(static_cast<QString>("Every %1 half months").arg(mult));
    }
  else if(type == MyMoneySchedule::OCCUR_MONTHLY)
    switch (mult)
    {
      case 1: occurenceString = I18N_NOOP("Monthly"); break;
      case 2: occurenceString = I18N_NOOP("Every two months"); break;
      case 3: occurenceString = I18N_NOOP("Every three months"); break;
      case 4: occurenceString = I18N_NOOP("Every four months"); break;
      case 6: occurenceString = I18N_NOOP("Twice yearly"); break;
      default: occurenceString = I18N_NOOP(static_cast<QString>("Every %1 months").arg(mult));
    }
  else if(type == MyMoneySchedule::OCCUR_YEARLY)
    switch (mult)
    {
      case 1: occurenceString = I18N_NOOP("Yearly"); break;
      case 2: occurenceString = I18N_NOOP("Every other year"); break;
      default: occurenceString = I18N_NOOP(static_cast<QString>("Every %1 years").arg(mult));
    }
  return occurenceString;
}

QString MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::occurenceE type)
{
  QString occurenceString = I18N_NOOP("Any");

  if(type == MyMoneySchedule::OCCUR_ONCE)
    occurenceString = I18N_NOOP("Once");
  else if(type == MyMoneySchedule::OCCUR_DAILY)
    occurenceString = I18N_NOOP("Day");
  else if(type == MyMoneySchedule::OCCUR_WEEKLY)
    occurenceString = I18N_NOOP("Week");
  else if(type == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    occurenceString = I18N_NOOP("Half-month");
  else if(type == MyMoneySchedule::OCCUR_MONTHLY)
    occurenceString = I18N_NOOP("Month");
  else if(type == MyMoneySchedule::OCCUR_YEARLY)
    occurenceString = I18N_NOOP("Year");
  return occurenceString;
}

QString MyMoneySchedule::scheduleTypeToString(MyMoneySchedule::typeE type)
{
  QString text;

  switch (type) {
    case MyMoneySchedule::TYPE_BILL:
      text = I18N_NOOP("Bill");
      break;
    case MyMoneySchedule::TYPE_DEPOSIT:
      text = I18N_NOOP("Deposit");
      break;
    case MyMoneySchedule::TYPE_TRANSFER:
      text = I18N_NOOP("Transfer");
      break;
    case MyMoneySchedule::TYPE_LOANPAYMENT:
      text = I18N_NOOP("Loan payment");
      break;
    case MyMoneySchedule::TYPE_ANY:
    default:
      text = I18N_NOOP("Unknown");
  }
  return text;
}


QString MyMoneySchedule::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
{
  QString text;

  switch (paymentType) {
    case MyMoneySchedule::STYPE_DIRECTDEBIT:
      text = I18N_NOOP("Direct debit");
      break;
    case MyMoneySchedule::STYPE_DIRECTDEPOSIT:
      text = I18N_NOOP("Direct deposit");
      break;
    case MyMoneySchedule::STYPE_MANUALDEPOSIT:
      text = I18N_NOOP("Manual deposit");
      break;
    case MyMoneySchedule::STYPE_OTHER:
      text = I18N_NOOP("Other");
      break;
    case MyMoneySchedule::STYPE_WRITECHEQUE:
      text = I18N_NOOP("Write check");
      break;
    case MyMoneySchedule::STYPE_STANDINGORDER:
      text = I18N_NOOP("Standing order");
      break;
    case MyMoneySchedule::STYPE_BANKTRANSFER:
      text = I18N_NOOP("Bank transfer");
      break;
    case MyMoneySchedule::STYPE_ANY:
      text = I18N_NOOP("Any (Error)");
      break;
  }
  return text;
}

QString MyMoneySchedule::weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption)
{
  QString text;

  switch (weekendOption) {
    case MyMoneySchedule::MoveFriday:
      text = I18N_NOOP("Change the date to the previous Friday");
      break;
    case MyMoneySchedule::MoveMonday:
      text = I18N_NOOP("Change the date to the next Monday");
      break;
    case MyMoneySchedule::MoveNothing:
      text = I18N_NOOP("Do Nothing");
      break;
  }
  return text;
}

// until we don't have the means to store the value
// of the variation, we default to 10% in case this
// scheduled transaction is marked 'not fixed'.
//
// ipwizard 2009-04-18

int MyMoneySchedule::variation(void) const
{
  int rc = 0;
  if(!isFixed()) {
    rc = 10;
#if 0
    QString var = value("kmm-variation");
    if(!var.isEmpty())
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
  if(var != 0)
    setValue("kmm-variation", QString("%1").arg(var));
#endif
}

int MyMoneySchedule::eventsPerYear(MyMoneySchedule::occurenceE occurence)
{
  int rc = 0;

  switch(occurence) {
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
      qWarning("Occurence not supported by financial calculator");
  }

  return rc;
}

int MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::occurenceE occurence)
{
  int rc = 0;

  switch(occurence) {
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
      qWarning("Occurence not supported by financial calculator");
  }

 return rc;
}

QDate MyMoneySchedule::addHalfMonths( QDate date, int mult ) const
{
  QDate newdate = date;
  int d, dm;
  if ( mult > 0 )
  {
    d = newdate.day();
    if ( d <= 12 )
    {
      if ( mult % 2 == 0 )
        newdate = newdate.addMonths(mult>>1);
      else
        newdate = newdate.addMonths(mult>>1).addDays(15);
    }
    else
      for ( int i = 0; i < mult; i++ )
      {
        if ( d <= 13 )
          newdate = newdate.addDays(15);
        else
        {
          dm = newdate.daysInMonth();
          if ( d == 14 )
            newdate = newdate.addDays(( dm < 30 ) ? dm - d : 15);
          else if ( d == 15 ) 
            newdate = newdate.addDays(dm - d);
          else if ( d == dm )
            newdate = newdate.addDays(15 - d).addMonths(1);
          else
            newdate = newdate.addDays(-15).addMonths(1);
        }
        d = newdate.day();
      }
  }
  else if ( mult < 0 ) // Go backwards
    for ( int i = 0; i > mult; i-- )
    {
      d = newdate.day();
      dm = newdate.daysInMonth();
      if ( d > 15 )
      {
        dm = newdate.daysInMonth();
        newdate = newdate.addDays( (d == dm) ? 15 - dm : -15);
      }
      else if ( d <= 13 )
        newdate = newdate.addMonths(-1).addDays(15);
      else if ( d == 15 )
        newdate = newdate.addDays(-15);
      else // 14
      {
        newdate = newdate.addMonths(-1);
        dm = newdate.daysInMonth();
        newdate = newdate.addDays(( dm < 30 ) ? dm - d : 15 );
      } 
    }
  return newdate;
} 

MyMoneySchedule::occurenceE MyMoneySchedule::stringToOccurence(const QString& text)
{
  MyMoneySchedule::occurenceE occurence = MyMoneySchedule::OCCUR_ANY;
  QString tmp = text.toLower();

  if(tmp == i18n("Once").toLower())
    occurence = MyMoneySchedule::OCCUR_ONCE;
  else if(tmp == i18n("Daily").toLower())
    occurence = MyMoneySchedule::OCCUR_DAILY;
  else if(tmp == i18n("Weekly").toLower())
    occurence = MyMoneySchedule::OCCUR_WEEKLY;
  else if(tmp == i18n("Fortnightly").toLower())
    occurence = MyMoneySchedule::OCCUR_FORTNIGHTLY;
  else if(tmp == i18n("Every other week").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYOTHERWEEK;
  else if(tmp == i18n("Every half month").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYHALFMONTH;
  else if(tmp == i18n("Every three weeks").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYTHREEWEEKS;
  else if(tmp == i18n("Every four weeks").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYFOURWEEKS;
  else if(tmp == i18n("Every thirty days").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS;
  else if(tmp == i18n("Monthly").toLower())
    occurence = MyMoneySchedule::OCCUR_MONTHLY;
  else if(tmp == i18n("Every eight weeks").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS;
  else if(tmp == i18n("Every two months").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYOTHERMONTH;
  else if(tmp == i18n("Every three months").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYTHREEMONTHS;
  else if(tmp == i18n("Quarterly").toLower())
    occurence = MyMoneySchedule::OCCUR_QUARTERLY;
  else if(tmp == i18n("Every four months").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYFOURMONTHS;
  else if(tmp == i18n("Twice yearly").toLower())
    occurence = MyMoneySchedule::OCCUR_TWICEYEARLY;
  else if(tmp == i18n("Yearly").toLower())
    occurence = MyMoneySchedule::OCCUR_YEARLY;
  else if(tmp == i18n("Every other year").toLower())
    occurence = MyMoneySchedule::OCCUR_EVERYOTHERYEAR;

  return occurence;
}

/**
  * Helper method to convert simple occurence to compound occurence + multiplier
  *
  * @param multiplier Returned by reference.  Adjusted multiplier
  * @param occurence Returned by reference.  Occurence type
  */
void MyMoneySchedule::simpleToCompoundOccurence(int& multiplier,occurenceE& occurence)
{
  occurenceE newOcc = occurence;
  int newMulti = 1;
  if (occurence == MyMoneySchedule::OCCUR_ONCE ||
      occurence == MyMoneySchedule::OCCUR_DAILY ||
      occurence == MyMoneySchedule::OCCUR_WEEKLY ||
      occurence == MyMoneySchedule::OCCUR_EVERYHALFMONTH ||
      occurence == MyMoneySchedule::OCCUR_MONTHLY ||
      occurence == MyMoneySchedule::OCCUR_YEARLY )
  { // Already a base occurence and multiplier
  }
  else if(occurence == MyMoneySchedule::OCCUR_FORTNIGHTLY ||
          occurence == MyMoneySchedule::OCCUR_EVERYOTHERWEEK)
  {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 2;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS)
  {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 3;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYFOURWEEKS)
  {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 4;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS)
  {
    newOcc    = MyMoneySchedule::OCCUR_DAILY;
    newMulti  = 30;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS)
  {
    newOcc    = MyMoneySchedule::OCCUR_WEEKLY;
    newMulti  = 8;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERMONTH)
  {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 2;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS ||
          occurence == MyMoneySchedule::OCCUR_QUARTERLY )
  {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 3;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYFOURMONTHS)
  {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 4;
  }
  else if(occurence == MyMoneySchedule::OCCUR_TWICEYEARLY)
  {
    newOcc    = MyMoneySchedule::OCCUR_MONTHLY;
    newMulti  = 6;
  }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERYEAR)
  {
    newOcc    = MyMoneySchedule::OCCUR_YEARLY;
    newMulti  = 2;
  }
  else  // Unknown
  {
    newOcc    = MyMoneySchedule::OCCUR_ANY;
    newMulti  = 1;
  }
  if (newOcc != occurence)
  {
    occurence   = newOcc;
    multiplier  = newMulti == 1 ? multiplier : newMulti * multiplier;
  }
}

/**
  * Helper method to convert compound occurence + multiplier to simple occurence
  *
  * @param multiplier Returned by reference.  Adjusted multiplier
  * @param occurence Returned by reference.  Occurence type
  */
void MyMoneySchedule::compoundToSimpleOccurence(int& multiplier,occurenceE& occurence)
{
  occurenceE newOcc = occurence;
  if(occurence == MyMoneySchedule::OCCUR_ONCE)
  { // Nothing to do
  }
  else if(occurence == MyMoneySchedule::OCCUR_DAILY)
  {
    switch (multiplier)
    {
      case 1: break;
      case 30: newOcc = MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS; break;
    }
  }
  else if(newOcc == MyMoneySchedule::OCCUR_WEEKLY)
  {
    switch (multiplier)
    {
      case 1: break;
      case 2: newOcc = MyMoneySchedule::OCCUR_EVERYOTHERWEEK; break;
      case 3: newOcc = MyMoneySchedule::OCCUR_EVERYTHREEWEEKS; break;
      case 4: newOcc = MyMoneySchedule::OCCUR_EVERYFOURWEEKS; break;
      case 8: newOcc = MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS; break;
    }
  }
  else if(occurence == MyMoneySchedule::OCCUR_MONTHLY)
    switch (multiplier)
    {
      case 1: break;
      case 2: newOcc = MyMoneySchedule::OCCUR_EVERYOTHERMONTH; break;
      case 3: newOcc = MyMoneySchedule::OCCUR_EVERYTHREEMONTHS; break;
      case 4: newOcc = MyMoneySchedule::OCCUR_EVERYFOURMONTHS; break;
      case 6: newOcc = MyMoneySchedule::OCCUR_TWICEYEARLY; break;
    }
  else if(occurence == MyMoneySchedule::OCCUR_EVERYHALFMONTH)
    switch (multiplier)
    {
      case 1: break;
    }
  else if(occurence == MyMoneySchedule::OCCUR_YEARLY)
  {
    switch (multiplier)
    {
      case 1: break;
      case 2: newOcc = MyMoneySchedule::OCCUR_EVERYOTHERYEAR; break;
    }
  }
  if (occurence != newOcc ) // Changed to derived type
  {
    occurence = newOcc;
    multiplier = 1;
  }
}
