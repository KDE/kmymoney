/*
 * Copyright 2005       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QUERYTABLE_H
#define QUERYTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "listtable.h"
#include "mymoneymoney.h"

class MyMoneyReport;

namespace reports
{

class ReportAccount;
class CashFlowList;

/**
  * Calculates a query of information about the transaction database.
  *
  * This is a middle-layer class, between the UI and the engine.  The
  * MyMoneyReport class holds only the CONFIGURATION parameters.  This
  * class actually does the work of retrieving the data from the engine
  * and formatting it for the user.
  *
  * @author Ace Jones
  *
  * @short
**/

class QueryTable : public ListTable
{
public:
  explicit QueryTable(const MyMoneyReport&);
  void init();

protected:
  void constructAccountTable();
  void constructTotalRows();
  void constructTransactionTable();
  void sumInvestmentValues(const ReportAccount &account, QList<CashFlowList> &cfList, QList<MyMoneyMoney> &shList) const;
  void constructPerformanceRow(const ReportAccount& account, TableRow& result, CashFlowList &all) const;
  void constructCapitalGainRow(const ReportAccount& account, TableRow& result) const;
  MyMoneyMoney helperROI(const MyMoneyMoney& buys, const MyMoneyMoney& sells, const MyMoneyMoney& startingBal, const MyMoneyMoney& endingBal, const MyMoneyMoney& cashIncome) const;
  MyMoneyMoney helperIRR(const CashFlowList& all) const;
  void constructSplitsTable();
private:
  enum InvestmentValue {Buys = 0, Sells, BuysOfSells, SellsOfBuys, LongTermBuysOfSells, LongTermSellsOfBuys, BuysOfOwned, ReinvestIncome, CashIncome, End};

};

//
// Cash Flow analysis tools for investment reports
//

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
  MyMoneyMoney NPV(double _rate) const;

  static void setToday(const QDate& _today) {
    m_sToday = _today;
  }
  const QDate& today() const {
    return m_sToday;
  }

private:
  QDate m_date;
  MyMoneyMoney m_value;

  static QDate m_sToday;
};

class CashFlowList: public QList<CashFlowListItem>
{
public:
  CashFlowList() {}
  MyMoneyMoney NPV(double rate) const;
  double IRR() const;
  MyMoneyMoney total() const;
  void dumpDebug() const;

  /**
   * Function: XIRR
   *
   * Compute the internal rate of return for a non-periodic series of cash flows.
   *
   * XIRR ( Values; Dates; [ Guess = 0.1 ] )
   **/
  double calculateXIRR() const;

protected:
  CashFlowListItem mostRecent() const;

private:
  /**
  * helper: xirrResult
  *
  * args[0] = values
  * args[1] = dates
   **/
  double xirrResult(double& rate) const;

  /**
  *
  * helper: xirrResultDerive
  *
  * args[0] = values
  * args[1] = dates
   **/
  double xirrResultDerive(double& rate) const;
};

}

#endif // QUERYREPORT_H
