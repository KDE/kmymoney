/*
 * Copyright 2011       Fernando Vilas <fvilas@iname.com>
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

#include "mymoneybalancecache.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

MyMoneyBalanceCacheItem::MyMoneyBalanceCacheItem(const MyMoneyMoney& balance, const QDate& date)
    : m_balance(balance), m_date(date)
{}

const MyMoneyMoney& MyMoneyBalanceCacheItem::balance() const
{
  return m_balance;
}

const QDate& MyMoneyBalanceCacheItem::date() const
{
  return m_date;
}

bool MyMoneyBalanceCacheItem::isValid() const
{
  return !(!m_date.isValid() && m_balance == MyMoneyMoney::minValue);
}

void MyMoneyBalanceCache::clear()
{
  m_cache.clear();
}

void MyMoneyBalanceCache::clear(const QString& accountId)
{
  m_cache.remove(accountId);
}

void MyMoneyBalanceCache::clear(const QString& accountId, const QDate& date)
{
  BalanceCacheType::Iterator acctPos = m_cache.find(accountId);
  if (m_cache.end() == acctPos)
    return;

  // Always remove QDate()
  BalanceCacheType::mapped_type::Iterator datePos = (*acctPos).find(QDate());
  if ((*acctPos).end() != datePos) {
    datePos = (*acctPos).erase(datePos);
  }

  // Now look for the actual value and remove it
  if (date.isValid()) {
    datePos = (*acctPos).lowerBound(date);

    while ((*acctPos).end() != datePos) {
      datePos = (*acctPos).erase(datePos);
    }
  }
}

bool MyMoneyBalanceCache::isEmpty() const
{
  return m_cache.isEmpty();
}

int MyMoneyBalanceCache::size() const
{
  int sum = 0;

  for (BalanceCacheType::ConstIterator i = m_cache.constBegin(); i != m_cache.constEnd(); ++i) {
    sum += (*i).size();
  }
  return sum;
}

void MyMoneyBalanceCache::insert(const QString& accountId, const QDate& date, const MyMoneyMoney& balance)
{
  m_cache[accountId].insert(date, balance);
}

MyMoneyBalanceCacheItem MyMoneyBalanceCache::balance(const QString& accountId, const QDate& date) const
{
  BalanceCacheType::ConstIterator acctPos = m_cache.constFind(accountId);
  if (m_cache.constEnd() == acctPos)
    return MyMoneyBalanceCacheItem(MyMoneyMoney::minValue, QDate());

  BalanceCacheType::mapped_type::ConstIterator datePos = (*acctPos).constFind(date);

  if ((*acctPos).constEnd() == datePos)
    return MyMoneyBalanceCacheItem(MyMoneyMoney::minValue, QDate());

  return MyMoneyBalanceCacheItem(datePos.value(), datePos.key());
}

MyMoneyBalanceCacheItem MyMoneyBalanceCache::mostRecentBalance(const QString& accountId, const QDate& date) const
{
  BalanceCacheType::ConstIterator acctPos = m_cache.constFind(accountId);
  if (m_cache.constEnd() == acctPos)
    return MyMoneyBalanceCacheItem(MyMoneyMoney::minValue, QDate());

  BalanceCacheType::mapped_type::ConstIterator datePos = (*acctPos).lowerBound(date);

  while ((*acctPos).constEnd() == datePos || ((*acctPos).constBegin() != datePos && datePos.key() > date)) {
    --datePos;
  }

  if ((*acctPos).constBegin() == datePos && datePos.key() > date)
    return MyMoneyBalanceCacheItem(MyMoneyMoney::minValue, QDate());

  return MyMoneyBalanceCacheItem(datePos.value(), datePos.key());
}

