/*
 * Copyright 2007 Sascha Pfau <MrPeacock@gmail.com>
 * Copyright 2018 Ralf Habacker <ralf.habacker@freenet.de>
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

/*
 * This file contains code from the func_xirr and related methods of
 * financial.cpp from KOffice by Sascha Pfau. Sascha agreed to relicense
 * those methods under GPLv2 or later.
 */

#ifndef CASHFLOWLIST_H
#define CASHFLOWLIST_H

#include "mymoneymoney.h"

#include <QDate>
#include <QList>

class CashFlowListItem
{
public:
  CashFlowListItem() {}
  CashFlowListItem(const QDate& _date, const MyMoneyMoney& _value): m_date(_date), m_value(_value) {}
  bool operator<(const CashFlowListItem& _second) const {
    return m_date < _second.m_date;
  }
  bool operator<=(const CashFlowListItem& _second) const {
    return m_date <= _second.m_date;
  }
  bool operator>(const CashFlowListItem& _second) const {
    return m_date > _second.m_date;
  }
  const QDate& date() const {
    return m_date;
  }
  const MyMoneyMoney& value() const {
    return m_value;
  }

private:
  QDate m_date;
  MyMoneyMoney m_value;
};

/**
 * Cash flow analysis tools for investment reports
 */
class CashFlowList: public QList<CashFlowListItem>
{
public:
  CashFlowList() {}
  double XIRR(double rate = 0.1) const;
  MyMoneyMoney total() const;
  void dumpDebug() const;

private:
  double xirrResult(double rate) const;
  double xirrResultDerive(double rate) const;
};

#endif // CASHFLOWLIST_H
