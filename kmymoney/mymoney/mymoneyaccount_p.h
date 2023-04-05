/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYACCOUNT_P_H
#define MYMONEYACCOUNT_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

class MyMoneyAccountPrivate : public MyMoneyObjectPrivate
{
public:
    MyMoneyAccountPrivate()
        : MyMoneyObjectPrivate()
        , m_accountType(Account::Type::Unknown)
        , m_fraction(-1)
    {
    }

    MyMoneyAccountPrivate(const MyMoneyAccountPrivate& right)
        : MyMoneyObjectPrivate(right)
    {
        *this = right;
    }

    MyMoneyAccountPrivate& operator=(const MyMoneyAccountPrivate& right)
    {
        MyMoneyObjectPrivate::operator=(right);
        m_accountType = right.m_accountType;
        m_institution = right.m_institution;
        m_name = right.m_name;
        m_number = right.m_number;
        m_description = right.m_description;
        m_lastModified = right.m_lastModified;
        m_openingDate = right.m_openingDate;
        m_lastReconciliationDate = right.m_lastReconciliationDate;
        m_accountList = right.m_accountList;
        m_parentAccount = right.m_parentAccount;
        m_currencyId = right.m_currencyId;
        m_balance = right.m_balance;
        m_totalPostedValue = right.m_totalPostedValue;
        m_postedValue = right.m_postedValue;
        m_onlineBankingSettings = right.m_onlineBankingSettings;
        m_fraction = right.m_fraction;
        m_reconciliationHistory = right.m_reconciliationHistory;
        return *this;
    }

    void collectReferencedObjects() override
    {
        m_referencedObjects = {m_institution, m_parentAccount, m_currencyId};
    }

    /**
      * This member variable identifies the type of account
      */
    eMyMoney::Account::Type m_accountType;

    /**
      * This member variable keeps the ID of the MyMoneyInstitution object
      * that this object belongs to.
      */
    QString m_institution;

    /**
      * This member variable keeps the name of the account
      * It is solely for documentation purposes and it's contents is not
      * used otherwise by the mymoney-engine.
      */
    QString m_name;

    /**
      * This member variable keeps the account number at the institution
      * It is solely for documentation purposes and it's contents is not
      * used otherwise by the mymoney-engine.
      */
    QString m_number;

    /**
      * This member variable is a description of the account.
      * It is solely for documentation purposes and it's contents is not
      * used otherwise by the mymoney-engine.
      */
    QString m_description;

    /**
      * This member variable keeps the date when the account
      * was last modified.
      */
    QDate m_lastModified;

    /**
      * This member variable keeps the date when the
      * account was created as an object in a MyMoneyFile
      */
    QDate m_openingDate;

    /**
      * This member variable keeps the date of the last
      * reconciliation of this account
      */
    QDate m_lastReconciliationDate;

    /**
      * This member holds the ID's of all sub-ordinate accounts
      */
    QStringList m_accountList;

    /**
      * This member contains the ID of the parent account
      */
    QString m_parentAccount;

    /**
      * This member contains the ID of the currency associated with this account
      */
    QString m_currencyId;

    /**
      * This member holds the balance of all transactions stored in the journal
      * for this account.
      */
    MyMoneyMoney    m_balance;

    /**
     * This member keeps the value of the account and all subaccounts
     */
    MyMoneyMoney    m_totalPostedValue;

    /**
     * This member keeps the value of the account without the subaccounts
     */
    MyMoneyMoney    m_postedValue;

    /**
      * This member variable keeps the set of kvp's needed to establish
      * online banking sessions to this account.
      */
    MyMoneyKeyValueContainer m_onlineBankingSettings;

    /**
      * This member keeps the fraction for the account. It is filled by MyMoneyFile
      * when set to -1. See also @sa fraction(const MyMoneySecurity&).
      */
    int             m_fraction;

    /**
      * This member keeps the reconciliation history
      */
    QMap<QDate, MyMoneyMoney> m_reconciliationHistory;

};

#endif
