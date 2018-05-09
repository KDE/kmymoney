
/***************************************************************************
                          mymoneyaccountloan.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef MYMONEYACCOUNTLOAN_H
#define MYMONEYACCOUNTLOAN_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"

/**
  * This class is a convenience class to access data for loan accounts.
  * It does contain the same member variables as a MyMoneyAccount object,
  * but serves a set of getter/setter methods to ease the access to
  * laon relevant data stored in the key value container of the MyMoneyAccount
  * object.
  */
class MyMoneyMoney;
class KMM_MYMONEY_EXPORT MyMoneyAccountLoan : public MyMoneyAccount
{
public:
  enum interestDueE {
    paymentDue = 0,
    paymentReceived
  };

  enum interestChangeUnitE {
    changeDaily = 0,
    changeWeekly,
    changeMonthly,
    changeYearly
  };

  MyMoneyAccountLoan() {}
  MyMoneyAccountLoan(const MyMoneyAccount&); // krazy:exclude=explicit
  ~MyMoneyAccountLoan() {}

  const MyMoneyMoney loanAmount() const;
  void setLoanAmount(const MyMoneyMoney& amount);
  const MyMoneyMoney interestRate(const QDate& date) const;
  void setInterestRate(const QDate& date, const MyMoneyMoney& rate);
  interestDueE interestCalculation() const;
  void setInterestCalculation(const interestDueE onReception);
  const QDate nextInterestChange() const;
  void setNextInterestChange(const QDate& date);
  const QString schedule() const;
  void setSchedule(const QString& sched);
  bool fixedInterestRate() const;
  void setFixedInterestRate(const bool fixed);
  const MyMoneyMoney finalPayment() const;
  void setFinalPayment(const MyMoneyMoney& finalPayment);
  unsigned int term() const;
  void setTerm(const unsigned int payments);
  int interestChangeFrequency(int* unit = 0) const;
  void setInterestChangeFrequency(const int amount, const int unit);
  const MyMoneyMoney periodicPayment() const;
  void setPeriodicPayment(const MyMoneyMoney& payment);
  int interestCompounding() const;
  void setInterestCompounding(int frequency);
  const QString payee() const;
  void setPayee(const QString& payee);
  const QString interestAccountId() const;
  void setInterestAccountId(const QString& id);

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  bool hasReferenceTo(const QString& id) const final override;

};

#endif
