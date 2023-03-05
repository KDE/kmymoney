/*
    SPDX-FileCopyrightText: 2001-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
        paymentReceived,
    };

    enum interestChangeUnitE {
        changeDaily = 0,
        changeWeekly,
        changeMonthly,
        changeYearly,
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
};

#endif
