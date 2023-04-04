/*
    SPDX-FileCopyrightText: 2001-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyaccountloan.h"
#include "mymoneyaccount_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QMap>
#include <QRegularExpression>
#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

class MyMoneyAccountLoanPrivate : public MyMoneyAccountPrivate
{
public:
    MyMoneyAccountLoanPrivate(MyMoneyAccountLoan* qq)
        : MyMoneyAccountPrivate()
        , q(qq)
    {
    }

    void collectReferencedObjects() override
    {
        MyMoneyAccountPrivate::collectReferencedObjects();
        m_referencedObjects.insert(q->payee());
        m_referencedObjects.insert(q->schedule());
    }
    MyMoneyAccountLoan* q;
};

MyMoneyAccountLoan::MyMoneyAccountLoan(const MyMoneyAccount& acc)
    : MyMoneyAccount(new MyMoneyAccountLoanPrivate(this), acc)
{
    Q_D(MyMoneyAccountLoan);
    *(static_cast<MyMoneyAccountPrivate*>(d)) = *acc.d_func();
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

    if (!date.isValid())
        return rate;

    const auto key = QStringLiteral("ir-%1").arg(date.toString(Qt::ISODate));
    QString val;
    const QRegularExpression interestRateExp("ir-(\\d{4})-(\\d{2})-(\\d{2})");

    QMap<QString, QString>::ConstIterator it;
    for (it = pairs().constBegin(); it != pairs().constEnd(); ++it) {
        const auto interestRateDate(interestRateExp.match(it.key()));
        if (interestRateDate.hasMatch()) {
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

    const auto key = QStringLiteral("ir-%1").arg(date.toString(Qt::ISODate));
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

    const QRegularExpression nextChangeExp("(\\d{4})-(\\d{2})-(\\d{2})");
    const auto nextChange(nextChangeExp.match(value("interest-nextchange")));
    if (nextChange.hasMatch()) {
        rc.setDate(nextChange.captured(1).toInt(), nextChange.captured(2).toInt(), nextChange.captured(3).toInt());
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

    const QRegularExpression frequencyExp("(\\d+)/(\\d{1})");
    const auto frequency(frequencyExp.match(value("interest-changefrequency")));
    if (frequency.hasMatch()) {
        rc = frequency.captured(1).toInt();
        if (unit != 0) {
            *unit = frequency.captured(2).toInt();
        }
    }
    return rc;
}

void MyMoneyAccountLoan::setInterestChangeFrequency(const int amount, const int unit)
{
    const auto val = QStringLiteral("%1/%2").arg(amount).arg(unit);
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

void MyMoneyAccountLoan::setInterestCompounding(int frequency)
{
    setValue("compoundingFrequency", QString("%1").arg(frequency));
}

int MyMoneyAccountLoan::interestCompounding() const
{
    return value("compoundingFrequency").toInt();
}
