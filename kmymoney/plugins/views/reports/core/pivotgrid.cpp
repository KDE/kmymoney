/*
 * Copyright 2005-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2005-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#include <pivotgrid.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace reports
{

const unsigned PivotOuterGroup::m_kDefaultSortOrder = 100;

PivotCell::PivotCell(const MyMoneyMoney& value) :
    MyMoneyMoney(value),
    m_stockSplit(MyMoneyMoney::ONE),
    m_cellUsed(!value.isZero())
{
}

PivotCell::~PivotCell()
{
}

PivotCell PivotCell::operator += (const PivotCell& right)
{
  const MyMoneyMoney& r = static_cast<const MyMoneyMoney&>(right);
  *this += r;
  m_postSplit = m_postSplit * right.m_stockSplit;
  m_stockSplit = m_stockSplit * right.m_stockSplit;
  m_postSplit += right.m_postSplit;
  m_cellUsed |= right.m_cellUsed;
  return *this;
}

PivotCell PivotCell::operator += (const MyMoneyMoney& value)
{
  m_cellUsed |= !value.isZero();
  if (m_stockSplit != MyMoneyMoney::ONE)
    m_postSplit += value;
  else
    MyMoneyMoney::operator += (value);
  return *this;
}

PivotCell PivotCell::stockSplit(const MyMoneyMoney& factor)
{
  PivotCell s;
  s.m_stockSplit = factor;
  return s;
}

const QString PivotCell::formatMoney(int fraction, bool showThousandSeparator) const
{
  return formatMoney("", MyMoneyMoney::denomToPrec(fraction), showThousandSeparator);
}

const QString PivotCell::formatMoney(const QString& currency, const int prec, bool showThousandSeparator) const
{
  // construct the result
  MyMoneyMoney res = (*this * m_stockSplit) + m_postSplit;
  return res.formatMoney(currency, prec, showThousandSeparator);
}

MyMoneyMoney PivotCell::calculateRunningSum(const MyMoneyMoney& runningSum)
{
  MyMoneyMoney::operator += (runningSum);
  MyMoneyMoney::operator = ((*this * m_stockSplit) + m_postSplit);
  m_postSplit = MyMoneyMoney();
  m_stockSplit = MyMoneyMoney::ONE;
  return *this;
}

MyMoneyMoney PivotCell::cellBalance(const MyMoneyMoney& _balance)
{
  MyMoneyMoney balance(_balance);
  balance += *this;
  balance = (balance * m_stockSplit) + m_postSplit;
  return balance;
}

PivotGridRowSet::PivotGridRowSet(unsigned _numcolumns)
{
  insert(eActual, PivotGridRow(_numcolumns));
  insert(eBudget, PivotGridRow(_numcolumns));
  insert(eBudgetDiff, PivotGridRow(_numcolumns));
  insert(eForecast, PivotGridRow(_numcolumns));
  insert(eAverage, PivotGridRow(_numcolumns));
  insert(ePrice, PivotGridRow(_numcolumns));
}

PivotGridRowSet PivotGrid::rowSet(QString id)
{

  //go through the data and get the row that matches the id
  PivotGrid::iterator it_outergroup = begin();
  while (it_outergroup != end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        if (it_row.key().id() == id)
          return it_row.value();

        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
  return PivotGridRowSet();
}

} // namespace
