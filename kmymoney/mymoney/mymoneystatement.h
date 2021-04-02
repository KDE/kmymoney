/*
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSTATEMENT_H
#define MYMONEYSTATEMENT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QDate>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

class QDomElement;
class QDomDocument;

/**
Represents the electronic analog of the paper bank statement just like we used to get in the regular mail.  This class is designed to be easy to extend and easy to create with minimal dependencies.  So the header file should include as few project files as possible (preferably NONE).

@author ace jones
*/
class KMM_MYMONEY_EXPORT MyMoneyStatement
{
public:
    struct Split
    {
        QString      m_strCategoryName;
        QString      m_strMemo;
        QString      m_accountId;
        eMyMoney::Split::State m_reconcile = eMyMoney::Split::State::NotReconciled;
        MyMoneyMoney m_amount;
    };

    struct Transaction
    {
        QDate m_datePosted;
        QString m_strPayee;
        QString m_strMemo;
        QString m_strNumber;
        QString m_strBankID;
        MyMoneyMoney m_amount;
        eMyMoney::Split::State m_reconcile = eMyMoney::Split::State::NotReconciled;

        eMyMoney::Transaction::Action m_eAction = eMyMoney::Transaction::Action::None;
        MyMoneyMoney m_shares;
        MyMoneyMoney m_fees;
        MyMoneyMoney m_price;
        QString m_strInterestCategory;
        QString m_strBrokerageAccount;
        QString m_strSymbol;
        QString m_strSecurity;
        QList<Split> m_listSplits;
    };

    struct Price {
        QDate m_date;
        QString m_sourceName;
        QString m_strSecurity;
        QString m_strCurrency;
        MyMoneyMoney m_amount;
    };

    struct Security {
        QString m_strName;
        QString m_strSymbol;
        QString m_strId;
    };

    QString m_strAccountName;
    QString m_strAccountNumber;
    QString m_strRoutingNumber;

    /**
     * The statement provider's information for the statement reader how to find the
     * account. The provider usually leaves some value with a key unique to the provider in the KVP of the
     * MyMoneyAccount object when setting up the connection or at a later point in time.
     * Using the KMyMoneyPlugin::KMMStatementInterface::account() method it can retrieve the
     * MyMoneyAccount object for this key. The account id of that account should be returned
     * here. If no id is available, leave it empty.
     */
    QString m_accountId;

    QString m_strCurrency;
    QDate m_dateBegin;
    QDate m_dateEnd;
    MyMoneyMoney m_closingBalance = MyMoneyMoney::autoCalc;
    eMyMoney::Statement::Type m_eType = eMyMoney::Statement::Type::None;

    QList<Transaction> m_listTransactions;
    QList<Price> m_listPrices;
    QList<Security> m_listSecurities;

    bool m_skipCategoryMatching = false;

    void write(QDomElement&, QDomDocument*) const;
    bool read(const QDomElement&);

    /**
     * This method returns the date provided as the end date of the statement.
     * In case this is not provided, we return the date of the youngest transaction
     * instead. In case there are no transactions found, an invalid date is
     * returned.
     *
     * @sa m_dateEnd
     */
    QDate statementEndDate() const;

    static bool isStatementFile(const QString&);
    static bool readXMLFile(MyMoneyStatement&, const QString&);
    static void writeXMLFile(const MyMoneyStatement&, const QString&);
};

/**
  * Make it possible to hold @ref MyMoneyStatement objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyStatement)

#endif
