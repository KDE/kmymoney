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
class CashFlowList;

namespace reports
{

class ReportAccount;

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
  QString helperIRR(const CashFlowList& all) const;
  void constructSplitsTable();
  bool linkEntries() const final override { return true; }
private:
  enum InvestmentValue {Buys = 0, Sells, BuysOfSells, SellsOfBuys, LongTermBuysOfSells, LongTermSellsOfBuys, BuysOfOwned, ReinvestIncome, CashIncome, End};

};

}

#endif // QUERYREPORT_H
