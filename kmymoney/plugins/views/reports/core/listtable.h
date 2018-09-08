/*
 * Copyright 2004-2005  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2008-2011  Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef LISTTABLE_H
#define LISTTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QVector>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "reporttable.h"

class MyMoneyReport;

namespace reports
{

class ReportAccount;

/**
  * Calculates a query of information about the transaction database.
  *
  * This is a middle-layer class, between the implementing classes and the engine. The
  * MyMoneyReport class holds only the CONFIGURATION parameters.  This
  * class has some common methods used by querytable and objectinfo classes
  *
  * @author Alvaro Soliverez
  *
  * @short
  **/

class ListTable : public ReportTable
{
public:
  explicit ListTable(const MyMoneyReport&);
  QString renderHTML() const final override;
  QString renderCSV() const final override;
  void drawChart(KReportChartView&) const final override {}
  void dump(const QString& file, const QString& context = QString()) const final override;
  void init();

public:
  enum cellTypeE /*{*/ /*Money*/
                  {ctValue, ctNetInvValue, ctMarketValue,
                   ctPrice, ctLastPrice, ctBuyPrice,
                   ctBuys, ctSells, ctBuysST, ctSellsST, ctBuysLT, ctSellsLT,
                   ctCapitalGain, ctCapitalGainST,ctCapitalGainLT,
                   ctCashIncome, ctReinvestIncome,
                   ctFees, ctInterest,
                   ctStartingBalance, ctEndingBalance, ctBalance, ctCurrentBalance,
                   ctBalanceWarning, ctMaxBalanceLimit, ctOpeningBalance,
                   ctCreditWarning, ctMaxCreditLimit,
                   ctLoanAmount, ctPeriodicPayment, ctFinalPayment, ctPayment,
                   /*Shares*/
                   ctShares,
                   /*Percent*/
                   ctReturn, ctReturnInvestment, ctInterestRate, ctPercentageGain,
                   /*Date*/
                   ctPostDate, ctEntryDate, ctNextDueDate, ctOpeningDate, ctNextInterestChange,
                   ctMonth, ctWeek, ctReconcileDate,
                   /*Misc*/
                   ctCurrency, ctCurrencyName, ctCommodity, ctID,
                   ctRank, ctSplit, ctMemo,
                   ctAccount, ctAccountID, ctTopAccount, ctInvestAccount, ctInstitution,
                   ctCategory, ctTopCategory, ctCategoryType,
                   ctNumber, ctReconcileFlag,
                   ctAction, ctTag, ctPayee, ctEquityType, ctType, ctName,
                   ctDepth, ctRowsCount, ctTax, ctFavorite, ctDescription, ctOccurrence, ctPaymentType
                 };
  /**
    * Contains a single row in the table.
    *
    * Each column is a key/value pair, both strings.  This class is just
    * a QMap with the added ability to specify which columns you'd like to
    * use as a sort key when you qHeapSort a list of these TableRows
    */
  class TableRow: public QMap<cellTypeE, QString>
  {
  public:
    bool operator< (const TableRow&) const;
    bool operator<= (const TableRow&) const;
    bool operator> (const TableRow&) const;
    bool operator== (const TableRow&) const;

    static void setSortCriteria(const QVector<cellTypeE>& _criteria) {
      m_sortCriteria = _criteria;
    }
  private:
    static QVector<cellTypeE> m_sortCriteria;
  };

  const QList<TableRow>& rows() {
    return m_rows;
  }

protected:
  void render(QString&, QString&) const;

  /**
   * If not in expert mode, include all subaccounts for each selected
   * investment account.
   * For investment-only reports, it will also exclude the subaccounts
   * that have a zero balance
   */
  void includeInvestmentSubAccounts();

  QList<TableRow> m_rows;

  QList<cellTypeE> m_group;
  /**
   * Comma-separated list of columns to place BEFORE the subtotal column
   */
  QList<cellTypeE> m_columns;
  /**
   * Name of the subtotal column
   */
  QList<cellTypeE> m_subtotal;
  /**
   * Comma-separated list of columns to place AFTER the subtotal column
   */
  QList<cellTypeE> m_postcolumns;

  virtual bool linkEntries() const = 0;

private:
  enum cellGroupE { cgMoney, cgShares, cgPercent, cgDate, cgPrice, cgMisc };
  static cellGroupE cellGroup(const cellTypeE cellType);
  static QString tableHeader(const cellTypeE cellType);
};

}

#endif

