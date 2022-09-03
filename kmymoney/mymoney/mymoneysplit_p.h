/*
    SPDX-FileCopyrightText: 2002-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSPLIT_P_H
#define MYMONEYSPLIT_P_H

#include "mymoneysplit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QList>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneymoney.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

class MyMoneySplitPrivate : public MyMoneyObjectPrivate
{

public:
    MyMoneySplitPrivate() :
        m_reconcileFlag(eMyMoney::Split::State::NotReconciled),
        m_isMatched(false)
    {
    }

    /**
      * This member contains the ID of the payee
      */
    QString        m_payee;

    /**
      * This member contains a list of the IDs of the tags
      */
    QList<QString> m_tagList;

    /**
      * This member contains the ID of the account
      */
    QString        m_account;

    /**
     * This member contains the ID of the cost center
     */
    QString        m_costCenter;

    /**
      */
    MyMoneyMoney   m_shares;

    /**
      */
    MyMoneyMoney   m_value;

    /**
      * If the quotient of m_shares divided by m_values is not the correct price
      * because of truncation, the price can be stored in this member. For display
      * purpose and transaction edit this value can be used by the application.
      */
    MyMoneyMoney   m_price;

    QString        m_memo;

    /**
      * This member contains information about the reconciliation
      * state of the split. Possible values are
      *
      * @li NotReconciled
      * @li Cleared
      * @li Reconciled
      * @li Frozen
      *
      */
    eMyMoney::Split::State m_reconcileFlag;

    /**
      * In case the reconciliation flag is set to Reconciled or Frozen
      * this member contains the date of the reconciliation.
      */
    QDate          m_reconcileDate;

    /**
      * The m_action member is an arbitrary string, but is intended to
      * be conveniently limited to a menu of selections such as
      * "Buy", "Sell", "Interest", etc.
      */
    QString        m_action;

    /**
      * The m_number member is used to store a reference number to
      * the split supplied by the user (e.g. check number, etc.).
      */
    QString        m_number;

    /**
      * This member keeps the bank's unique ID for the split, so we can
      * avoid duplicates.  This is only used for electronic statement downloads.
      *
      * This should only be set on the split which refers to the account
      * that was downloaded.
      */
    QString        m_bankID;

    /**
      * This member keeps a backward id to the transaction that this
      * split can be found in. It is the purpose of the MyMoneyTransaction
      * object to maintain this member variable.
      */
    QString        m_transactionId;

    MyMoneyTransaction m_matchedTransaction;
    bool m_isMatched;

};

#endif
