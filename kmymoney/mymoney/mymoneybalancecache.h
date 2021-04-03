/*
    SPDX-FileCopyrightText: 2011 Fernando Vilas <fvilas@iname.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYBALANCECACHE_H
#define MYMONEYBALANCECACHE_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QDate>
#include <QHash>
#include <QMap>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

/**
 * This class associates a @ref MyMoneyMoney object with a QDate. It is
 * essentially a std::pair. The object is invalid if the QDate is invalid
 * and the balance is @ref MyMoneyMoney::minValue. A call to isValid
 * should be used in most cases before using an object.
 */
class KMM_MYMONEY_EXPORT MyMoneyBalanceCacheItem
{
public:

    /**
     * The object should be instantiated with a balance and a date.
     *
     * @param balance a MyMoneyMoney object that holds the balance at the
     * end of the day
     * @param date the date for the object
     */
    MyMoneyBalanceCacheItem(const MyMoneyMoney& balance, const QDate& date);

    /**
     * Accessor function for the balance as a MyMoneyMoney object.
     *
     * @return the balance in the object
     */
    const MyMoneyMoney& balance() const;

    /**
     * Accessor function for the date as a QDate object.
     *
     * @return the date in the object
     */
    const QDate& date() const;

    /**
     * Check the validity of the object
     *
     * @return true if the date is valid, false otherwise
     */
    bool isValid() const;

private:
    MyMoneyMoney m_balance;
    QDate m_date;
};

/**
 * This class provides a balance cache. It holds balances by account and date,
 * so the most recent balance could be obtained to reduce the number of
 * transactions required to calculate a balance for a later date. This class
 * is intended to be used at the @ref MyMoneyFile layer.
 */
class KMM_MYMONEY_EXPORT MyMoneyBalanceCache
{
public:

    /**
     * Remove all balances from the cache
     */
    void clear();

    /**
     * Remove all balances associated with a given account from the cache
     */
    void clear(const QString& accountId);

    /**
     * Remove all balances on or after the date associated with an account
     * from the cache
     */
    void clear(const QString& accountId, const QDate& date);

    /**
     * @return true if there are no balances in the cache, otherwise false
     */
    bool isEmpty() const;

    /**
     * This function returns the size of the cache.
     *
     * @note  It has linear complexity O(num of accounts), so do not
     * call it in a loop if there are many accounts stored in it or
     * performance is an issue.
     *
     * @return the number of balances currently in the cache
     */
    int size() const;

    /**
     * This function retrieves the balance from the cache. The balance is
     * invalid if the cache does not contain a balance on that date.
     *
     * @param accountId the account id to use to find the balance
     * @param date the date of the balance to retrieve
     *
     * @return the balance of the account at the end of the date given
     */
    MyMoneyBalanceCacheItem balance(const QString& accountId, const QDate& date) const;

    /**
     * This function retrieves the balance from the cache having a date less
     * than or equal to the date given. The balance is invalid if the cache
     * does not contain a balance on or before that date for the account given
     *
     * @param accountId the account id to use to find the balance
     * @param date the latest date of the balance to retrieve
     *
     * @return the balance of the account at the end the nearest day on or
     * before the date provided
     */
    MyMoneyBalanceCacheItem mostRecentBalance(const QString& accountId, const QDate& date) const;

    /**
     * This function inserts the balance into the cache for the account on
     * the date provided. If a balance already exists for that account and
     * date, it is updated to the one provided.
     *
     * @param accountId the account id
     * @param date the date of the balance
     * @param balance the balance of the account at the end the day
     */
    void insert(const QString& accountId, const QDate& date, const MyMoneyMoney& balance);

private:
    typedef QHash<QString, QMap<QDate, MyMoneyMoney> > BalanceCacheType;
    BalanceCacheType m_cache;
};

#endif
