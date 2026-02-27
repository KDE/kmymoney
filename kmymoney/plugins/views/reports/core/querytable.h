/*
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
class MyMoneySplit;
class MyMoneyTransaction;
class CashFlowList;

namespace reports
{

class ReportAccount;
class ReportState;

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
    QString toXml() const override;
    void init();

protected:
    void constructAccountTable();
    void constructTotalRows();
    void constructTransactionTable();
    static QString splitCurrencyId(const MyMoneySplit& split);
    static bool findTaxAccount(const QList<MyMoneySplit>& splits);
    static QList<MyMoneySplit>::const_iterator selectReferenceSplit(const QList<MyMoneySplit>& splits, const MyMoneyReport& report);
    void addRow(const ListTable::TableRow& row);
    bool includeReferenceSplitAccount(const ReportAccount& splitAcc) const;
    bool splitMatchesTransactionFilter(const MyMoneySplit& split, bool transactionTextMatches) const;
    void updateNonBaseCurrencyStatus(const TableRow& row);
    void addTransactionRow(const TableRow& row);
    void setupReferenceSplitRow(const MyMoneySplit& split, const ReportAccount& splitAcc, const MyMoneyMoney& xr, const MyMoneyMoney& rateXr, const MyMoneyMoney& valueXr, int fraction, ReportState& state);
    void processIncludedReferenceSplit(const MyMoneySplit& split, const ReportAccount& splitAcc, const MyMoneyMoney& valueXr, int fraction, int splitCount, ReportState& state);
    void processFurtherSplit(const MyMoneyTransaction& t, const MyMoneySplit& referenceSplit, const MyMoneySplit& split, const ReportAccount& splitAcc, const MyMoneyMoney& xr, const MyMoneyMoney& valueXr, int fraction, int splitCount, ReportState& state);
    void processTransferSplit(const MyMoneySplit& split, const ReportAccount& splitAcc, const MyMoneyMoney& xr, const MyMoneyMoney& rateXr, const MyMoneyMoney& valueXr, int fraction, int splitCount, const QString& institution, const QString& payee, const QList<QString>& tagIdList, ReportState& state);
    void addPendingTransactionRows(ReportState& state);
    void processTransaction(const MyMoneyTransaction& t, ReportState& state);
    void addOpeningClosingBalances(ReportState& state);
    void sumInvestmentValues(const ReportAccount &account, QList<CashFlowList> &cfList, QList<MyMoneyMoney> &shList) const;
    void constructPerformanceRow(const ReportAccount& account, TableRow& result, CashFlowList &all) const;
    void constructCapitalGainRow(const ReportAccount& account, TableRow& result) const;
    MyMoneyMoney returnValue(const MyMoneyMoney& buys,
                             const MyMoneyMoney& sells,
                             const MyMoneyMoney& reinvestIncome,
                             const MyMoneyMoney& cashIncome,
                             const MyMoneyMoney& startingBalance,
                             const MyMoneyMoney& endingBalance) const;
    bool ROI(MyMoneyMoney& returnInvestment,
             const MyMoneyMoney& buys,
             const MyMoneyMoney& sells,
             const MyMoneyMoney& reinvestIncome,
             const MyMoneyMoney& cashIncome,
             const MyMoneyMoney& startingBalance,
             const MyMoneyMoney& endingBalance) const;
    QString helperAROI(const MyMoneyMoney& buys,
                       const MyMoneyMoney& sells,
                       const MyMoneyMoney& reinvestIncome,
                       const MyMoneyMoney& cashIncome,
                       const MyMoneyMoney& startingBalance,
                       const MyMoneyMoney& endingBalance,
                       const QDate& startingDate,
                       const QDate& endingDate) const;
    QString helperROI(const MyMoneyMoney& buys,
                      const MyMoneyMoney& sells,
                      const MyMoneyMoney& reinvestIncome,
                      const MyMoneyMoney& cashIncome,
                      const MyMoneyMoney& startingBalance,
                      const MyMoneyMoney& endingBalance) const;
    QString helperReturnValue(const MyMoneyMoney& buys,
                              const MyMoneyMoney& sells,
                              const MyMoneyMoney& reinvestIncome,
                              const MyMoneyMoney& cashIncome,
                              const MyMoneyMoney& startingBalance,
                              const MyMoneyMoney& endingBalance) const;

    /**
     * Calculates the extended internal rate of return
     * @param all list with cash flow items
     * @return string with extended internal rate of return
     */
    QString helperXIRR(const CashFlowList& all) const;

    void constructSplitsTable();
    bool linkEntries() const final override {
        return true;
    }

private:
    enum InvestmentValue {Buys = 0, Sells, BuysOfSells, SellsOfBuys, LongTermBuysOfSells, LongTermSellsOfBuys, BuysOfOwned, ReinvestIncome, CashIncome, End};

};

}

#endif // QUERYREPORT_H
