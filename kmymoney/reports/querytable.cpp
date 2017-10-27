/***************************************************************************
                          querytable.cpp
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004-2005 by Ace Jones <acejones@users.sourceforge.net>
                           (C) 2007 Sascha Pfau <MrPeacock@gmail.com>
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/****************************************************************************
  Contains code from the func_xirr and related methods of financial.cpp
  - KOffice 1.6 by Sascha Pfau.  Sascha agreed to relicense those methods under
  GPLv2 or later.
*****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "querytable.h"

#include <cmath>

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "mymoneyprice.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"
#include "mymoneyreport.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "reportaccount.h"

namespace reports
{

// ****************************************************************************
//
// CashFlowListItem implementation
//
//   Cash flow analysis tools for investment reports
//
// ****************************************************************************

QDate CashFlowListItem::m_sToday = QDate::currentDate();

MyMoneyMoney CashFlowListItem::NPV(double _rate) const
{
  double T = static_cast<double>(m_sToday.daysTo(m_date)) / 365.0;
  MyMoneyMoney result(m_value.toDouble() / pow(1 + _rate, T), 100);

  //qDebug() << "CashFlowListItem::NPV( " << _rate << " ) == " << result;

  return result;
}

// ****************************************************************************
//
// CashFlowList implementation
//
//   Cash flow analysis tools for investment reports
//
// ****************************************************************************

CashFlowListItem CashFlowList::mostRecent() const
{
  CashFlowList dupe(*this);

  qSort(dupe);

  //qDebug() << " CashFlowList::mostRecent() == " << dupe.back().date().toString(Qt::ISODate);

  return dupe.back();
}

MyMoneyMoney CashFlowList::NPV(double _rate) const
{
  MyMoneyMoney result;

  const_iterator it_cash = constBegin();
  while (it_cash != constEnd()) {
    result += (*it_cash).NPV(_rate);
    ++it_cash;
  }

  //qDebug() << "CashFlowList::NPV( " << _rate << " ) == " << result << "------------------------" << endl;

  return result;
}

double CashFlowList::calculateXIRR() const
{
  double resultRate = 0.00001;

  double resultZero = 0.00000;
  //if ( args.count() > 2 )
  //  resultRate = calc->conv()->asFloat ( args[2] ).asFloat();

// check pairs and count >= 2 and guess > -1.0
  //if ( args[0].count() != args[1].count() || args[1].count() < 2 || resultRate <= -1.0 )
  //  return Value::errorVALUE();

// define max epsilon
  static const double maxEpsilon = 1e-5;

// max number of iterations
  static const int maxIter = 50;

// Newton's method - try to find a res, with a accuracy of maxEpsilon
  double rateEpsilon, newRate, resultValue;
  int i = 0;
  bool contLoop;

  do {
    resultValue = xirrResult(resultRate);

    double resultDerive = xirrResultDerive(resultRate);

    //check what happens if xirrResultDerive is zero
    //Don't know if it is correct to dismiss the result
    if (resultDerive != 0) {
      newRate =  resultRate - resultValue / resultDerive;
    } else {

      newRate =  resultRate - resultValue;
    }

    rateEpsilon = fabs(newRate - resultRate);

    resultRate = newRate;
    contLoop = (rateEpsilon > maxEpsilon) && (fabs(resultValue) > maxEpsilon);
  } while (contLoop && (++i < maxIter));

  if (contLoop)
    return resultZero;

  return resultRate;
}

double CashFlowList::xirrResult(double& rate) const
{
  QDate date;

  double r = rate + 1.0;
  double res = 0.00000;//back().value().toDouble();

  QList<CashFlowListItem>::const_iterator list_it = constBegin();
  while (list_it != constEnd()) {
    double e_i = ((* list_it).today().daysTo((* list_it).date())) / 365.0;
    MyMoneyMoney val = (* list_it).value();

    if (e_i < 0) {
      res += val.toDouble() * pow(r, -e_i);
    } else {
      res += val.toDouble() / pow(r, e_i);
    }
    ++list_it;
  }

  return res;
}


double CashFlowList::xirrResultDerive(double& rate) const
{
  QDate date;

  double r = rate + 1.0;
  double res = 0.00000;

  QList<CashFlowListItem>::const_iterator list_it = constBegin();
  while (list_it != constEnd()) {
    double e_i = ((* list_it).today().daysTo((* list_it).date())) / 365.0;
    MyMoneyMoney val = (* list_it).value();

    res -= e_i * val.toDouble() / pow(r, e_i + 1.0);
    ++list_it;
  }

  return res;
}

double CashFlowList::IRR() const
{
  double result = 0.0;

  // set 'today', which is the most recent of all dates in the list
  CashFlowListItem::setToday(mostRecent().date());

  result = calculateXIRR();
  return result;
}

MyMoneyMoney CashFlowList::total() const
{
  MyMoneyMoney result;

  const_iterator it_cash = constBegin();
  while (it_cash != constEnd()) {
    result += (*it_cash).value();
    ++it_cash;
  }

  return result;
}

void CashFlowList::dumpDebug() const
{
  const_iterator it_item = constBegin();
  while (it_item != constEnd()) {
    qDebug() << (*it_item).date().toString(Qt::ISODate) << " " << (*it_item).value().toString();
    ++it_item;
  }
}

// ****************************************************************************
//
// QueryTable implementation
//
// ****************************************************************************

/**
  * TODO
  *
  * - Collapse 2- & 3- groups when they are identical
  * - Way more test cases (especially splits & transfers)
  * - Option to collapse splits
  * - Option to exclude transfers
  *
  */

QueryTable::QueryTable(const MyMoneyReport& _report): ListTable(_report)
{
  // separated into its own method to allow debugging (setting breakpoints
  // directly in ctors somehow does not work for me (ipwizard))
  // TODO: remove the init() method and move the code back to the ctor
  init();
}

void QueryTable::init()
{
  m_columns.clear();
  m_group.clear();
  m_subtotal.clear();
  m_postcolumns.clear();
  switch (m_config.rowType()) {
    case MyMoneyReport::eAccountByTopAccount:
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
      constructAccountTable();
      m_columns << ctAccount;
      break;

    case MyMoneyReport::eAccount:
      constructTransactionTable();
      m_columns << ctAccountID << ctPostDate;
      break;

    case MyMoneyReport::ePayee:
    case MyMoneyReport::eTag:
    case MyMoneyReport::eMonth:
    case MyMoneyReport::eWeek:
      constructTransactionTable();
      m_columns << ctPostDate << ctAccount;
      break;
    case MyMoneyReport::eCashFlow:
      constructSplitsTable();
      m_columns << ctPostDate;
      break;
    default:
      constructTransactionTable();
      m_columns << ctPostDate;
  }

  // Sort the data to match the report definition
  m_subtotal << ctValue;

  switch (m_config.rowType()) {
    case MyMoneyReport::eCashFlow:
      m_group << ctCategoryType << ctTopCategory << ctCategory;
      break;
    case MyMoneyReport::eCategory:
      m_group << ctCategoryType << ctTopCategory << ctCategory;
      break;
    case MyMoneyReport::eTopCategory:
      m_group << ctCategoryType << ctTopCategory;
      break;
    case MyMoneyReport::eTopAccount:
      m_group << ctTopAccount << ctAccount;
      break;
    case MyMoneyReport::eAccount:
      m_group << ctAccount;
      break;
    case MyMoneyReport::eAccountReconcile:
      m_group << ctAccount << ctReconcileFlag;
      break;
    case MyMoneyReport::ePayee:
      m_group << ctPayee;
      break;
    case MyMoneyReport::eTag:
      m_group << ctTag;
      break;
    case MyMoneyReport::eMonth:
      m_group << ctMonth;
      break;
    case MyMoneyReport::eWeek:
      m_group << ctWeek;
      break;
    case MyMoneyReport::eAccountByTopAccount:
      m_group << ctTopAccount;
      break;
    case MyMoneyReport::eEquityType:
      m_group << ctEquityType;
      break;
    case MyMoneyReport::eAccountType:
      m_group << ctType;
      break;
    case MyMoneyReport::eInstitution:
      m_group << ctInstitution << ctTopAccount;
      break;
    default:
      throw MYMONEYEXCEPTION("QueryTable::QueryTable(): unhandled row type");
  }

  QVector<cellTypeE> sort = QVector<cellTypeE>::fromList(m_group) << QVector<cellTypeE>::fromList(m_columns) << ctID << ctRank;

  m_columns.clear();
  switch (m_config.rowType()) {
    case MyMoneyReport::eAccountByTopAccount:
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
      m_columns << ctAccount;
      break;

    default:
      m_columns << ctPostDate;
  }

  unsigned qc = m_config.queryColumns();

  if (qc & MyMoneyReport::eQCnumber)
    m_columns << ctNumber;
  if (qc & MyMoneyReport::eQCpayee)
    m_columns << ctPayee;
  if (qc & MyMoneyReport::eQCtag)
    m_columns << ctTag;
  if (qc & MyMoneyReport::eQCcategory)
    m_columns << ctCategory;
  if (qc & MyMoneyReport::eQCaccount)
    m_columns << ctAccount;
  if (qc & MyMoneyReport::eQCreconciled)
    m_columns << ctReconcileFlag;
  if (qc & MyMoneyReport::eQCmemo)
    m_columns << ctMemo;
  if (qc & MyMoneyReport::eQCaction)
    m_columns << ctAction;
  if (qc & MyMoneyReport::eQCshares)
    m_columns << ctShares;
  if (qc & MyMoneyReport::eQCprice)
    m_columns << ctPrice;
  if (qc & MyMoneyReport::eQCperformance) {
    m_subtotal.clear();
    switch (m_config.investmentSum()) {
      case MyMoneyReport::eSumOwnedAndSold:
        m_columns << ctBuys << ctSells << ctReinvestIncome << ctCashIncome
                  << ctEndingBalance << ctReturn << ctReturnInvestment;
        m_subtotal << ctBuys << ctSells << ctReinvestIncome << ctCashIncome
                   << ctEndingBalance << ctReturn << ctReturnInvestment;
        break;
      case MyMoneyReport::eSumOwned:
        m_columns << ctBuys << ctReinvestIncome << ctMarketValue
                  << ctReturn << ctReturnInvestment;
        m_subtotal << ctBuys << ctReinvestIncome << ctMarketValue
                   << ctReturn << ctReturnInvestment;
        break;
      case MyMoneyReport::eSumSold:
        m_columns << ctBuys << ctSells << ctCashIncome
                  << ctReturn << ctReturnInvestment;
        m_subtotal << ctBuys << ctSells << ctCashIncome
                   << ctReturn << ctReturnInvestment;
        break;
      case MyMoneyReport::eSumPeriod:
      default:
        m_columns << ctStartingBalance << ctBuys << ctSells
                  << ctReinvestIncome << ctCashIncome << ctEndingBalance
                  << ctReturn << ctReturnInvestment;
        m_subtotal << ctStartingBalance << ctBuys << ctSells
                   << ctReinvestIncome << ctCashIncome << ctEndingBalance
                   << ctReturn << ctReturnInvestment;
        break;
    }
  }
  if (qc & MyMoneyReport::eQCcapitalgain) {
    m_subtotal.clear();
    switch (m_config.investmentSum()) {
      case MyMoneyReport::eSumOwned:
        m_columns << ctShares << ctBuyPrice << ctLastPrice
                  << ctBuys << ctMarketValue << ctPercentageGain
                  << ctCapitalGain;
        m_subtotal << ctShares << ctBuyPrice << ctLastPrice
                   << ctBuys << ctMarketValue << ctPercentageGain
                   << ctCapitalGain;
        break;
      case MyMoneyReport::eSumSold:
      default:
        m_columns << ctBuys << ctSells << ctCapitalGain;
        m_subtotal << ctBuys << ctSells << ctCapitalGain;
        if (m_config.isShowingSTLTCapitalGains()) {
          m_columns << ctBuysST << ctSellsST << ctCapitalGainST
                    << ctBuysLT << ctSellsLT << ctCapitalGainLT;
          m_subtotal << ctBuysST << ctSellsST << ctCapitalGainST
                     << ctBuysLT << ctSellsLT << ctCapitalGainLT;
        }
        break;
    }
  }
  if (qc & MyMoneyReport::eQCloan) {
    m_columns << ctPayment << ctInterest << ctFees;
    m_postcolumns << ctBalance;
  }
  if (qc & MyMoneyReport::eQCbalance)
    m_postcolumns << ctBalance;

  TableRow::setSortCriteria(sort);
  qSort(m_rows);
  if (m_config.isShowingColumnTotals())
    constructTotalRows(); // adds total rows to m_rows
}

void QueryTable::constructTotalRows()
{
  if (m_rows.isEmpty())
    return;

  // qSort places grand total at last position, because it doesn't belong to any group
  for (int i = 0; i < m_rows.count(); ++i) {
    if (m_rows.at(0)[ctRank] == QLatin1String("4") || m_rows.at(0)[ctRank] == QLatin1String("5")) // it should be unlikely that total row is at the top of rows, so...
      m_rows.move(0, m_rows.count() - 1 - i);                       // ...move it at the bottom
    else
      break;
  }

  MyMoneyFile* file = MyMoneyFile::instance();
  QList<cellTypeE> subtotals = m_subtotal;
  QList<cellTypeE> groups = m_group;
  QList<cellTypeE> columns = m_columns;
  if (!m_subtotal.isEmpty() && subtotals.count() == 1)
    columns.append(m_subtotal);
  QList<cellTypeE> postcolumns = m_postcolumns;
  if (!m_postcolumns.isEmpty())
    columns.append(postcolumns);

  QMap<QString, QList<QMap<cellTypeE, MyMoneyMoney>>> totalCurrency;
  QList<QMap<cellTypeE, MyMoneyMoney>> totalGroups;
  QMap<cellTypeE, MyMoneyMoney> totalsValues;

  // initialize all total values under summed columns to be zero
  foreach (auto subtotal, subtotals) {
    totalsValues.insert(subtotal, MyMoneyMoney());
  }
  totalsValues.insert(ctRowsCount, MyMoneyMoney());

  // create total groups containing totals row for each group
  totalGroups.append(totalsValues);  // prepend with extra group for grand total
  for (int j = 0; j < groups.count(); ++j) {
    totalGroups.append(totalsValues);
  }

  QList<TableRow> stashedTotalRows;
  int iCurrentRow, iNextRow;
  for (iCurrentRow = 0; iCurrentRow < m_rows.count();) {
    iNextRow = iCurrentRow + 1;

    // total rows are useless at summing so remove whole block of them at once
    while (iNextRow != m_rows.count() && (m_rows.at(iNextRow).value(ctRank) == QLatin1String("4") || m_rows.at(iNextRow).value(ctRank) == QLatin1String("5"))) {
      stashedTotalRows.append(m_rows.takeAt(iNextRow)); // ...but stash them just in case
    }

    bool lastRow = (iNextRow == m_rows.count());

    // sum all subtotal values for lowest group
    QString currencyID = m_rows.at(iCurrentRow).value(ctCurrency);
    if (m_rows.at(iCurrentRow).value(ctRank) == QLatin1String("1")) { // don't sum up on balance (rank = 0 || rank = 3) and minor split (rank = 2)
      foreach (auto subtotal, subtotals) {
        if (!totalCurrency.contains(currencyID))
          totalCurrency[currencyID].append(totalGroups);
        totalCurrency[currencyID].last()[subtotal] += MyMoneyMoney(m_rows.at(iCurrentRow)[subtotal]);
      }
      totalCurrency[currencyID].last()[ctRowsCount] += MyMoneyMoney::ONE;
    }

    // iterate over groups from the lowest to the highest to find group change
    for (int i = groups.count() - 1; i >= 0 ; --i) {
      // if any of groups from next row changes (or next row is the last row), then it's time to put totals row
      if (lastRow || m_rows.at(iCurrentRow)[groups.at(i)] != m_rows.at(iNextRow)[groups.at(i)]) {
        bool isMainCurrencyTotal = true;
        QMap<QString, QList<QMap<cellTypeE, MyMoneyMoney>>>::iterator currencyGrp = totalCurrency.begin();
        while (currencyGrp != totalCurrency.end()) {
          if (!MyMoneyMoney((*currencyGrp).at(i + 1).value(ctRowsCount)).isZero()) {    // if no rows summed up, then no totals row
            TableRow totalsRow;
            // sum all subtotal values for higher groups (excluding grand total) and reset lowest group values
            QMap<cellTypeE, MyMoneyMoney>::iterator upperGrp = (*currencyGrp)[i].begin();
            QMap<cellTypeE, MyMoneyMoney>::iterator lowerGrp = (*currencyGrp)[i + 1].begin();

            while(upperGrp != (*currencyGrp)[i].end()) {
              totalsRow[lowerGrp.key()] = lowerGrp.value().toString();  // fill totals row with subtotal values...
              (*upperGrp) += (*lowerGrp);
              //          (*lowerGrp) = MyMoneyMoney();
              ++upperGrp;
              ++lowerGrp;
            }

            // custom total values calculations
            foreach (auto subtotal, subtotals) {
              if (subtotal == ctReturnInvestment)
                totalsRow[subtotal] = helperROI((*currencyGrp).at(i + 1).value(ctBuys) - (*currencyGrp).at(i + 1).value(ctReinvestIncome), (*currencyGrp).at(i + 1).value(ctSells),
                                                (*currencyGrp).at(i + 1).value(ctStartingBalance), (*currencyGrp).at(i + 1).value(ctEndingBalance) + (*currencyGrp).at(i + 1).value(ctMarketValue),
                                                (*currencyGrp).at(i + 1).value(ctCashIncome)).toString();
              else if (subtotal == ctPercentageGain)
                totalsRow[subtotal] = (((*currencyGrp).at(i + 1).value(ctBuys) + (*currencyGrp).at(i + 1).value(ctMarketValue)) / (*currencyGrp).at(i + 1).value(ctBuys).abs()).toString();
              else if (subtotal == ctPrice)
                totalsRow[subtotal] = MyMoneyMoney((*currencyGrp).at(i + 1).value(ctPrice) / (*currencyGrp).at(i + 1).value(ctRowsCount)).toString();
            }

            // total values that aren't calculated here, but are taken untouched from external source, e.g. constructPerformanceRow
            if (!stashedTotalRows.isEmpty()) {
              for (int j = 0; j < stashedTotalRows.count(); ++j) {
                if (stashedTotalRows.at(j).value(ctCurrency) != currencyID)
                  continue;
                foreach (auto subtotal, subtotals) {
                  if (subtotal == ctReturn)
                    totalsRow[ctReturn] = stashedTotalRows.takeAt(j)[ctReturn];
                }
                break;
              }
            }

            (*currencyGrp).replace(i + 1, totalsValues);
            for (int j = 0; j < groups.count(); ++j) {
              totalsRow[groups.at(j)] = m_rows.at(iCurrentRow)[groups.at(j)];   // ...and identification
            }

            QString currencyID = currencyGrp.key();
            if (currencyID.isEmpty() && totalCurrency.count() > 1)
              currencyID = file->baseCurrency().id();
            totalsRow[ctCurrency] = currencyID;
            if (isMainCurrencyTotal) {
              totalsRow[ctRank] = QLatin1Char('4');
              isMainCurrencyTotal = false;
            } else
              totalsRow[ctRank] = QLatin1Char('5');
            totalsRow[ctDepth] = QString::number(i);
            totalsRow.remove(ctRowsCount);

            m_rows.insert(iNextRow++, totalsRow);  // iCurrentRow and iNextRow can diverge here by more than one
          }
          ++currencyGrp;
        }
      }
    }

    // code to put grand total row
    if (lastRow) {
      bool isMainCurrencyTotal = true;
      QMap<QString, QList<QMap<cellTypeE, MyMoneyMoney>>>::iterator currencyGrp = totalCurrency.begin();
      while (currencyGrp != totalCurrency.end()) {
        TableRow totalsRow;
        QMap<cellTypeE, MyMoneyMoney>::const_iterator grandTotalGrp = (*currencyGrp)[0].constBegin();
        while(grandTotalGrp != (*currencyGrp)[0].constEnd()) {
          totalsRow[grandTotalGrp.key()] = grandTotalGrp.value().toString();
          ++grandTotalGrp;
        }

        foreach (auto subtotal, subtotals) {
          if (subtotal == ctReturnInvestment)
            totalsRow[subtotal] = helperROI((*currencyGrp).at(0).value(ctBuys) - (*currencyGrp).at(0).value(ctReinvestIncome), (*currencyGrp).at(0).value(ctSells),
                                            (*currencyGrp).at(0).value(ctStartingBalance), (*currencyGrp).at(0).value(ctEndingBalance) + (*currencyGrp).at(0).value(ctMarketValue),
                                            (*currencyGrp).at(0).value(ctCashIncome)).toString();
          else if (subtotal == ctPercentageGain)
            totalsRow[subtotal] = (((*currencyGrp).at(0).value(ctBuys) + (*currencyGrp).at(0).value(ctMarketValue)) / (*currencyGrp).at(0).value(ctBuys).abs()).toString();
          else if (subtotal == ctPrice)
            totalsRow[subtotal] = MyMoneyMoney((*currencyGrp).at(0).value(ctPrice) / (*currencyGrp).at(0).value(ctRowsCount)).toString();
        }

        if (!stashedTotalRows.isEmpty()) {
          for (int j = 0; j < stashedTotalRows.count(); ++j) {
            foreach (auto subtotal, subtotals) {
              if (subtotal == ctReturn)
                totalsRow[ctReturn] = stashedTotalRows.takeAt(j)[ctReturn];
            }
          }
        }

        for (int j = 0; j < groups.count(); ++j) {
          totalsRow[groups.at(j)] = QString();      // no identification
        }

        QString currencyID = currencyGrp.key();
        if (currencyID.isEmpty() && totalCurrency.count() > 1)
          currencyID = file->baseCurrency().id();
        totalsRow[ctCurrency] = currencyID;
        if (isMainCurrencyTotal) {
          totalsRow[ctRank] = QLatin1Char('4');
          isMainCurrencyTotal = false;
        } else
          totalsRow[ctRank] = QLatin1Char('5');
        totalsRow[ctDepth] = QString();

        m_rows.append(totalsRow);
        ++currencyGrp;
      }
      break;                                      // no use to loop further
    }
    iCurrentRow = iNextRow;                             // iCurrent makes here a leap forward by at least one
  }
}

void QueryTable::constructTransactionTable()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //make sure we have all subaccounts of investment accounts
  includeInvestmentSubAccounts();

  MyMoneyReport report(m_config);
  report.setReportAllSplits(false);
  report.setConsiderCategory(true);

  bool use_transfers;
  bool use_summary;
  bool hide_details;
  bool tag_special_case = false;

  switch (m_config.rowType()) {
    case MyMoneyReport::eCategory:
    case MyMoneyReport::eTopCategory:
      use_summary = false;
      use_transfers = false;
      hide_details = false;
      break;
    case MyMoneyReport::ePayee:
      use_summary = false;
      use_transfers = false;
      hide_details = (m_config.detailLevel() == MyMoneyReport::eDetailNone);
      break;
    case MyMoneyReport::eTag:
      use_summary = false;
      use_transfers = false;
      hide_details = (m_config.detailLevel() == MyMoneyReport::eDetailNone);
      tag_special_case = true;
      break;
    default:
      use_summary = true;
      use_transfers = true;
      hide_details = (m_config.detailLevel() == MyMoneyReport::eDetailNone);
      break;
  }

  // support for opening and closing balances
  QMap<QString, MyMoneyAccount> accts;

  //get all transactions for this report
  QList<MyMoneyTransaction> transactions = file->transactionList(report);
  for (QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.constBegin(); it_transaction != transactions.constEnd(); ++it_transaction) {

    TableRow qA, qS;
    QDate pd;
    QList<QString> tagIdListCache;

    qA[ctID] = qS[ctID] = (* it_transaction).id();
    qA[ctEntryDate] = qS[ctEntryDate] = (* it_transaction).entryDate().toString(Qt::ISODate);
    qA[ctPostDate] = qS[ctPostDate] = (* it_transaction).postDate().toString(Qt::ISODate);
    qA[ctCommodity] = qS[ctCommodity] = (* it_transaction).commodity();

    pd = (* it_transaction).postDate();
    qA[ctMonth] = qS[ctMonth] = i18n("Month of %1", QDate(pd.year(), pd.month(), 1).toString(Qt::ISODate));
    qA[ctWeek] = qS[ctWeek] = i18n("Week of %1", pd.addDays(1 - pd.dayOfWeek()).toString(Qt::ISODate));

    if (!m_containsNonBaseCurrency && (*it_transaction).commodity() != file->baseCurrency().id())
      m_containsNonBaseCurrency = true;
    if (report.isConvertCurrency())
      qA[ctCurrency] = qS[ctCurrency] = file->baseCurrency().id();
    else
      qA[ctCurrency] = qS[ctCurrency] = (*it_transaction).commodity();

    // to handle splits, we decide on which account to base the split
    // (a reference point or point of view so to speak). here we take the
    // first account that is a stock account or loan account (or the first account
    // that is not an income or expense account if there is no stock or loan account)
    // to be the account (qA) that will have the sub-item "split" entries. we add
    // one transaction entry (qS) for each subsequent entry in the split.

    const QList<MyMoneySplit>& splits = (*it_transaction).splits();
    QList<MyMoneySplit>::const_iterator myBegin, it_split;

    for (it_split = splits.constBegin(), myBegin = splits.constEnd(); it_split != splits.constEnd(); ++it_split) {
      ReportAccount splitAcc = (* it_split).accountId();
      // always put split with a "stock" account if it exists
      if (splitAcc.isInvest())
        break;

      // prefer to put splits with a "loan" account if it exists
      if (splitAcc.isLoan())
        myBegin = it_split;

      if ((myBegin == splits.end()) && ! splitAcc.isIncomeExpense()) {
        myBegin = it_split;
      }
    }

    // select our "reference" split
    if (it_split == splits.end()) {
      it_split = myBegin;
    } else {
      myBegin = it_split;
    }

    // skip this transaction if we didn't find a valid base account - see the above description
    // for the base account's description - if we don't find it avoid a crash by skipping the transaction
    if (myBegin == splits.end())
      continue;

    // if the split is still unknown, use the first one. I have seen this
    // happen with a transaction that has only a single split referencing an income or expense
    // account and has an amount and value of 0. Such a transaction will fall through
    // the above logic and leave 'it_split' pointing to splits.end() which causes the remainder
    // of this to end in an infinite loop.
    if (it_split == splits.end()) {
      it_split = splits.begin();
    }

    // for "loan" reports, the loan transaction gets special treatment.
    // the splits of a loan transaction are placed on one line in the
    // reference (loan) account (qA). however, we process the matching
    // split entries (qS) normally.

    bool loan_special_case = false;
    if (m_config.queryColumns() & MyMoneyReport::eQCloan) {
      ReportAccount splitAcc = (*it_split).accountId();
      loan_special_case = splitAcc.isLoan();
    }

    bool include_me = true;
    bool transaction_text = false; //indicates whether a text should be considered as a match for the transaction or for a split only
    QString a_fullname;
    QString a_memo;
    int pass = 1;

    QString myBeginCurrency;
    QString baseCurrency = file->baseCurrency().id();

    QMap<QString, MyMoneyMoney> xrMap; // container for conversion rates from given currency to myBeginCurrency
    do {
      MyMoneyMoney xr;
      ReportAccount splitAcc = (* it_split).accountId();
      QString splitCurrency;
      if (splitAcc.isInvest())
        splitCurrency = file->account(file->account((*it_split).accountId()).parentAccountId()).currencyId();
      else
        splitCurrency = file->account((*it_split).accountId()).currencyId();
      if (it_split == myBegin)
        myBeginCurrency = splitCurrency;

      //get fraction for account
      int fraction = splitAcc.currency().smallestAccountFraction();

      //use base currency fraction if not initialized
      if (fraction == -1)
        fraction = file->baseCurrency().smallestAccountFraction();

      QString institution = splitAcc.institutionId();
      QString payee = (*it_split).payeeId();

      const QList<QString> tagIdList = (*it_split).tagIdList();

      //convert to base currency
      if (m_config.isConvertCurrency()) {
        xr = xrMap.value(splitCurrency, xr);  // check if there is conversion rate to myBeginCurrency already stored...
        if (xr == MyMoneyMoney())             // ...if not...
            xr = (*it_split).price();         // ...take conversion rate to myBeginCurrency from split
        else if (splitAcc.isInvest())         // if it's stock split...
          xr *= (*it_split).price();          // ...multiply it by stock price stored in split

        if (!m_containsNonBaseCurrency && myBeginCurrency != baseCurrency)
          m_containsNonBaseCurrency = true;
        if (myBeginCurrency != baseCurrency) {                             // myBeginCurrency can differ from baseCurrency...
          MyMoneyPrice price = file->price(myBeginCurrency, baseCurrency,
                                           (*it_transaction).postDate());  // ...so check conversion rate...
          if (price.isValid()) {
            xr *= price.rate(baseCurrency);                                // ...and multiply it by current price...
            qA[ctCurrency] = qS[ctCurrency] = baseCurrency;
          } else
            qA[ctCurrency] = qS[ctCurrency] = myBeginCurrency;             // ...and set information about non-baseCurrency
        }
      } else if (splitAcc.isInvest())
        xr = (*it_split).price();
      else
        xr = MyMoneyMoney::ONE;

      if (it_split == myBegin) {
        include_me = m_config.includes(splitAcc);
        if (include_me)
          // track accts that will need opening and closing balances
          //FIXME in some cases it will show the opening and closing
          //balances but no transactions if the splits are all filtered out -- asoliverez
          accts.insert(splitAcc.id(), splitAcc);

        qA[ctAccount] = splitAcc.name();
        qA[ctAccountID] = splitAcc.id();
        qA[ctTopAccount] = splitAcc.topParentName();

        if (splitAcc.isInvest()) {
          // use the institution of the parent for stock accounts
          institution = splitAcc.parent().institutionId();
          MyMoneyMoney shares = (*it_split).shares();

          int pricePrecision = file->security(splitAcc.currencyId()).pricePrecision();
          qA[ctAction] = (*it_split).action();
          qA[ctShares] = shares.isZero() ? QString() : shares.toString();
          qA[ctPrice] = shares.isZero() ? QString() : xr.convertPrecision(pricePrecision).toString();

          if (((*it_split).action() == MyMoneySplit::ActionBuyShares) && shares.isNegative())
            qA[ctAction] = "Sell";

          qA[ctInvestAccount] = splitAcc.parent().name();

          MyMoneySplit stockSplit = (*it_split);
          MyMoneySplit assetAccountSplit;
          QList<MyMoneySplit> feeSplits;
          QList<MyMoneySplit> interestSplits;
          MyMoneySecurity currency;
          MyMoneySecurity security;
          MyMoneySplit::investTransactionTypeE transactionType;
          KMyMoneyUtils::dissectTransaction((*it_transaction), stockSplit, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
          if (!(assetAccountSplit == MyMoneySplit())) {
            for (it_split = splits.begin(); it_split != splits.end(); ++it_split) {
              if ((*it_split) == assetAccountSplit) {
                splitAcc = assetAccountSplit.accountId(); // switch over from stock split to asset split because amount in stock split doesn't take fees/interests into account
                myBegin = it_split;                       // set myBegin to asset split, so stock split can be listed in details under splits
                myBeginCurrency = (file->account((*myBegin).accountId())).currencyId();
                if (!m_containsNonBaseCurrency && myBeginCurrency != baseCurrency)
                  m_containsNonBaseCurrency = true;
                if (m_config.isConvertCurrency()) {
                  if (myBeginCurrency != baseCurrency) {
                    MyMoneyPrice price = file->price(myBeginCurrency, baseCurrency, (*it_transaction).postDate());
                    if (price.isValid()) {
                      xr = price.rate(baseCurrency);
                      qA[ctCurrency] = qS[ctCurrency] = baseCurrency;
                    } else
                      qA[ctCurrency] = qS[ctCurrency] = myBeginCurrency;
                  } else
                    xr = MyMoneyMoney::ONE;

                  qA[ctPrice] = shares.isZero() ? QString() : (stockSplit.price() * xr / (*it_split).price()).toString();
                  // put conversion rate for all splits with this currency, so...
                  // every split of transaction have the same conversion rate
                  xrMap.insert(splitCurrency, MyMoneyMoney::ONE / (*it_split).price());
                } else
                  xr = (*it_split).price();
                break;
              }
            }
          }
        } else
          qA[ctPrice] = xr.toString();

        a_fullname = splitAcc.fullName();
        a_memo = (*it_split).memo();

        transaction_text = m_config.match(&(*it_split));

        qA[ctInstitution] = institution.isEmpty()
                            ? i18n("No Institution")
                            : file->institution(institution).name();

        qA[ctPayee] = payee.isEmpty()
                      ? i18n("[Empty Payee]")
                      : file->payee(payee).name().simplified();

        if (tag_special_case) {
          tagIdListCache = tagIdList;
        } else {
          QString delimiter;
          foreach(const auto tagId, tagIdList) {
            qA[ctTag] += delimiter + file->tag(tagId).name().simplified();
            delimiter = QLatin1Char(',');
          }
        }
        qA[ctReconcileDate] = (*it_split).reconcileDate().toString(Qt::ISODate);
        qA[ctReconcileFlag] = KMyMoneyUtils::reconcileStateToString((*it_split).reconcileFlag(), true);
        qA[ctNumber] = (*it_split).number();

        qA[ctMemo] = a_memo;

        qA[ctValue] = ((*it_split).shares() * xr).convert(fraction).toString();

        qS[ctReconcileDate] = qA[ctReconcileDate];
        qS[ctReconcileFlag] = qA[ctReconcileFlag];
        qS[ctNumber] = qA[ctNumber];

        qS[ctTopCategory] = splitAcc.topParentName();
        qS[ctCategoryType] = i18n("Transfer");

        // only include the configured accounts
        if (include_me) {

          if (loan_special_case) {

            // put the principal amount in the "value" column and convert to lowest fraction
            qA[ctValue] = (-(*it_split).shares() * xr).convert(fraction).toString();

            qA[ctRank] = QLatin1Char('1');
            qA[ctSplit].clear();

          } else {
            if ((splits.count() > 2) && use_summary) {

              // add the "summarized" split transaction
              // this is the sub-total of the split detail
              // convert to lowest fraction
              qA[ctRank] = QLatin1Char('1');
              qA[ctCategory] = i18n("[Split Transaction]");
              qA[ctTopCategory] = i18nc("Split transaction", "Split");
              qA[ctCategoryType] = i18nc("Split transaction", "Split");

              m_rows += qA;
            }
          }
        }

      } else {

        if (include_me) {

          if (loan_special_case) {
            MyMoneyMoney value = (-(* it_split).shares() * xr).convert(fraction);

            if ((*it_split).action() == MyMoneySplit::ActionAmortization) {
              // put the payment in the "payment" column and convert to lowest fraction
              qA[ctPayee] = value.toString();
            } else if ((*it_split).action() == MyMoneySplit::ActionInterest) {
              // put the interest in the "interest" column and convert to lowest fraction
              qA[ctInterest] = value.toString();
            } else if (splits.count() > 2) {
              // [dv: This comment carried from the original code. I am
              // not exactly clear on what it means or why we do this.]
              // Put the initial pay-in nowhere (that is, ignore it). This
              // is dangerous, though. The only way I can tell the initial
              // pay-in apart from fees is if there are only 2 splits in
              // the transaction.  I wish there was a better way.
            } else {
              // accumulate everything else in the "fees" column
              MyMoneyMoney n0 = MyMoneyMoney(qA[ctFees]);
              qA[ctFees] = (n0 + value).toString();
            }
            // we don't add qA here for a loan transaction. we'll add one
            // qA afer all of the split components have been processed.
            // (see below)

          }

          //--- special case to hide split transaction details
          else if (hide_details && (splits.count() > 2)) {
            // essentially, don't add any qA entries
          }
          //--- default case includes all transaction details
          else {

            //this is when the splits are going to be shown as children of the main split
            if ((splits.count() > 2) && use_summary) {
              qA[ctValue].clear();

              //convert to lowest fraction
              qA[ctSplit] = (-(*it_split).shares() * xr).convert(fraction).toString();
              qA[ctRank] = QLatin1Char('2');
            } else {
              //this applies when the transaction has only 2 splits, or each split is going to be
              //shown separately, eg. transactions by category
              switch (m_config.rowType()) {
                case MyMoneyReport::eCategory:
                case MyMoneyReport::eTopCategory:
                  if (splitAcc.isIncomeExpense())
                    qA[ctValue] = (-(*it_split).shares() * xr).convert(fraction).toString(); // needed for category reports, in case of multicurrency transaction it breaks it
                  break;
                default:
                  break;
              }
              qA[ctSplit].clear();
              qA[ctRank] = QLatin1Char('1');
            }

            qA [ctMemo] = (*it_split).memo();

            if (!m_containsNonBaseCurrency && splitAcc.currencyId() != file->baseCurrency().id())
              m_containsNonBaseCurrency = true;
            if (report.isConvertCurrency())
              qS[ctCurrency] = file->baseCurrency().id();
            else
              qS[ctCurrency] = splitAcc.currency().id();

            if (! splitAcc.isIncomeExpense()) {
              qA[ctCategory] = ((*it_split).shares().isNegative()) ?
                               i18n("Transfer from %1", splitAcc.fullName())
                               : i18n("Transfer to %1", splitAcc.fullName());
              qA[ctTopCategory] = splitAcc.topParentName();
              qA[ctCategoryType] = i18n("Transfer");
            } else {
              qA [ctCategory] = splitAcc.fullName();
              qA [ctTopCategory] = splitAcc.topParentName();
              qA [ctCategoryType] = KMyMoneyUtils::accountTypeToString(splitAcc.accountGroup());
            }

            if (use_transfers || (splitAcc.isIncomeExpense() && m_config.includes(splitAcc))) {
              //if it matches the text of the main split of the transaction or
              //it matches this particular split, include it
              //otherwise, skip it
              //if the filter is "does not contain" exclude the split if it does not match
              //even it matches the whole split
              if ((m_config.isInvertingText() &&
                   m_config.match(&(*it_split)))
                  || (!m_config.isInvertingText()
                      && (transaction_text
                          || m_config.match(&(*it_split))))) {
                if (tag_special_case) {
                  if (!tagIdListCache.size())
                    qA[ctTag] = i18n("[No Tag]");
                  else
                    for (int i = 0; i < tagIdListCache.size(); i++) {
                      qA[ctTag] = file->tag(tagIdListCache[i]).name().simplified();
                      m_rows += qA;
                    }
                } else {
                  m_rows += qA;
                }
              }
            }
          }
        }

        if (m_config.includes(splitAcc) && use_transfers &&
            !(splitAcc.isInvest() && include_me)) { // otherwise stock split is displayed twice in report
          if (! splitAcc.isIncomeExpense()) {
            //multiply by currency and convert to lowest fraction
            qS[ctValue] = ((*it_split).shares() * xr).convert(fraction).toString();

            qS[ctRank] = QLatin1Char('1');

            qS[ctAccount] = splitAcc.name();
            qS[ctAccountID] = splitAcc.id();
            qS[ctTopAccount] = splitAcc.topParentName();

            qS[ctCategory] = ((*it_split).shares().isNegative())
                             ? i18n("Transfer to %1", a_fullname)
                             : i18n("Transfer from %1", a_fullname);

            qS[ctInstitution] = institution.isEmpty()
                                ? i18n("No Institution")
                                : file->institution(institution).name();

            qS[ctMemo] = (*it_split).memo().isEmpty()
                         ? a_memo
                         : (*it_split).memo();

            //FIXME-ALEX When is used this? I can't find in which condition we arrive here... maybe this code is useless?
            QString delimiter;
            for (int i = 0; i < tagIdList.size(); i++) {
              qA[ctTag] += delimiter + file->tag(tagIdList[i]).name().simplified();
              delimiter = "+";
            }

            qS[ctPayee] = payee.isEmpty()
                          ? qA[ctPayee]
                          : file->payee(payee).name().simplified();

            //check the specific split against the filter for text and amount
            //TODO this should be done at the engine, but I have no clear idea how -- asoliverez
            //if the filter is "does not contain" exclude the split if it does not match
            //even it matches the whole split
            if ((m_config.isInvertingText() &&
                 m_config.match(&(*it_split)))
                || (!m_config.isInvertingText()
                    && (transaction_text
                        || m_config.match(&(*it_split))))) {
              m_rows += qS;

              // track accts that will need opening and closing balances
              accts.insert(splitAcc.id(), splitAcc);
            }
          }
        }
      }

      ++it_split;

      // look for wrap-around
      if (it_split == splits.end())
        it_split = splits.begin();

      // but terminate if this transaction has only a single split
      if (splits.count() < 2)
        break;

      //check if there have been more passes than there are splits
      //this is to prevent infinite loops in cases of data inconsistency -- asoliverez
      ++pass;
      if (pass > splits.count())
        break;

    } while (it_split != myBegin);
    if (loan_special_case) {
      m_rows += qA;
    }
  }

  // now run through our accts list and add opening and closing balances

  switch (m_config.rowType()) {
    case MyMoneyReport::eAccount:
    case MyMoneyReport::eTopAccount:
      break;

      // case MyMoneyReport::eCategory:
      // case MyMoneyReport::eTopCategory:
      // case MyMoneyReport::ePayee:
      // case MyMoneyReport::eMonth:
      // case MyMoneyReport::eWeek:
    default:
      return;
  }

  QDate startDate, endDate;

  report.validDateRange(startDate, endDate);
  QString strStartDate = startDate.toString(Qt::ISODate);
  QString strEndDate = endDate.toString(Qt::ISODate);
  startDate = startDate.addDays(-1);

  QMap<QString, MyMoneyAccount>::const_iterator it_account, accts_end;
  for (it_account = accts.constBegin(); it_account != accts.constEnd(); ++it_account) {
    TableRow qA;

    ReportAccount account = (* it_account);

    //get fraction for account
    int fraction = account.currency().smallestAccountFraction();

    //use base currency fraction if not initialized
    if (fraction == -1)
      fraction = file->baseCurrency().smallestAccountFraction();

    QString institution = account.institutionId();

    // use the institution of the parent for stock accounts
    if (account.isInvest())
      institution = account.parent().institutionId();

    MyMoneyMoney startBalance, endBalance, startPrice, endPrice;
    MyMoneyMoney startShares, endShares;

    //get price and convert currency if necessary
    if (m_config.isConvertCurrency()) {
      startPrice = (account.deepCurrencyPrice(startDate) * account.baseCurrencyPrice(startDate)).reduce();
      endPrice = (account.deepCurrencyPrice(endDate) * account.baseCurrencyPrice(endDate)).reduce();
    } else {
      startPrice = account.deepCurrencyPrice(startDate).reduce();
      endPrice = account.deepCurrencyPrice(endDate).reduce();
    }
    startShares = file->balance(account.id(), startDate);
    endShares = file->balance(account.id(), endDate);

    //get starting and ending balances
    startBalance = startShares * startPrice;
    endBalance = endShares * endPrice;

    //starting balance
    // don't show currency if we're converting or if it's not foreign
    if (!m_containsNonBaseCurrency && account.currency().id() != file->baseCurrency().id())
      m_containsNonBaseCurrency = true;
    if (m_config.isConvertCurrency())
      qA[ctCurrency] = file->baseCurrency().id();
    else
      qA[ctCurrency] = account.currency().id();

    qA[ctAccountID] = account.id();
    qA[ctAccount] = account.name();
    qA[ctTopAccount] = account.topParentName();
    qA[ctInstitution] = institution.isEmpty() ? i18n("No Institution") : file->institution(institution).name();
    qA[ctRank] = QLatin1Char('0');

    qA[ctPrice] = startPrice.convertPrecision(account.currency().pricePrecision()).toString();
    if (account.isInvest()) {
      qA[ctShares] = startShares.toString();
    }

    qA[ctPostDate] = strStartDate;
    qA[ctBalance] = startBalance.convert(fraction).toString();
    qA[ctValue].clear();
    qA[ctID] = QLatin1Char('A');
    m_rows += qA;

    //ending balance
    qA[ctPrice] = endPrice.convertPrecision(account.currency().pricePrecision()).toString();

    if (account.isInvest()) {
      qA[ctShares] = endShares.toString();
    }

    qA[ctPostDate] = strEndDate;
    qA[ctBalance] = endBalance.toString();
    qA[ctRank] = QLatin1Char('3');
    qA[ctID] = QLatin1Char('Z');
    m_rows += qA;
  }
}

MyMoneyMoney QueryTable::helperROI(const MyMoneyMoney &buys, const MyMoneyMoney &sells, const MyMoneyMoney &startingBal, const MyMoneyMoney &endingBal, const MyMoneyMoney &cashIncome) const
{
  MyMoneyMoney returnInvestment;
  if (!buys.isZero() || !startingBal.isZero()) {
    returnInvestment = (sells + buys + cashIncome + endingBal - startingBal) / (startingBal - buys);
    returnInvestment = returnInvestment.convert(10000);
  } else
    returnInvestment = MyMoneyMoney(); // if no investment then no return on investment
  return returnInvestment;
}

MyMoneyMoney QueryTable::helperIRR(const CashFlowList &all) const
{
  MyMoneyMoney annualReturn;
  try {
    double irr = all.IRR();
#ifdef Q_CC_MSVC
    annualReturn = MyMoneyMoney(_isnan(irr) ? 0 : irr, 10000);
#else
    annualReturn = MyMoneyMoney(std::isnan(irr) ? 0 : irr, 10000);
#endif
  } catch (QString e) {
    qDebug() << e;
  }
  return annualReturn;
}

void QueryTable::sumInvestmentValues(const ReportAccount& account, QList<CashFlowList>& cfList, QList<MyMoneyMoney>& shList) const
{
  for (int i = InvestmentValue::Buys; i < InvestmentValue::End; ++i)
    cfList.append(CashFlowList());
  for (int i = InvestmentValue::Buys; i <= InvestmentValue::BuysOfOwned; ++i)
    shList.append(MyMoneyMoney());

  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyReport report = m_config;
  QDate startingDate;
  QDate endingDate;
  QDate newStartingDate;
  QDate newEndingDate;
  const bool isSTLT = report.isShowingSTLTCapitalGains();
  const int settlementPeriod = report.settlementPeriod();
  QDate termSeparator = report.termSeparator().addDays(-settlementPeriod);
  report.validDateRange(startingDate, endingDate);
  newStartingDate = startingDate;
  newEndingDate = endingDate;

  if (report.queryColumns() & MyMoneyReport::eQCcapitalgain) {
    // Saturday and Sunday aren't valid settlement dates
    if (endingDate.dayOfWeek() == Qt::Saturday)
      endingDate = endingDate.addDays(-1);
    else if (endingDate.dayOfWeek() == Qt::Sunday)
      endingDate = endingDate.addDays(-2);

    if (termSeparator.dayOfWeek() == Qt::Saturday)
      termSeparator = termSeparator.addDays(-1);
    else if (termSeparator.dayOfWeek() == Qt::Sunday)
      termSeparator = termSeparator.addDays(-2);
    if (startingDate.daysTo(endingDate) <= settlementPeriod)        // no days to check for
      return;
    termSeparator = termSeparator.addDays(-settlementPeriod);
    newEndingDate = endingDate.addDays(-settlementPeriod);
  }

  shList[BuysOfOwned] = file->balance(account.id(), newEndingDate); // get how many shares there are at the end of period
  MyMoneyMoney stashedBuysOfOwned = shList.at(BuysOfOwned);

  bool reportedDateRange = true;  // flag marking sell transactions between startingDate and endingDate
  report.setReportAllSplits(false);
  report.setConsiderCategory(true);
  report.clearAccountFilter();
  report.addAccount(account.id());
  report.setDateFilter(newStartingDate, newEndingDate);

  do {
    QList<MyMoneyTransaction> transactions = file->transactionList(report);
    for (QList<MyMoneyTransaction>::const_reverse_iterator  it_t = transactions.crbegin(); it_t != transactions.crend(); ++it_t) {
      MyMoneySplit shareSplit = (*it_t).splitByAccount(account.id());
      MyMoneySplit assetAccountSplit;
      QList<MyMoneySplit> feeSplits;
      QList<MyMoneySplit> interestSplits;
      MyMoneySecurity security;
      MyMoneySecurity currency;
      MyMoneySplit::investTransactionTypeE transactionType;
      KMyMoneyUtils::dissectTransaction((*it_t), shareSplit, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
      QDate postDate = (*it_t).postDate();
      MyMoneyMoney price;
      //get price for the day of the transaction if we have to calculate base currency
      //we are using the value of the split which is in deep currency
      if (m_config.isConvertCurrency())
        price = account.baseCurrencyPrice(postDate); //we only need base currency because the value is in deep currency
      else
        price = MyMoneyMoney::ONE;
      MyMoneyMoney value = assetAccountSplit.value() * price;
      MyMoneyMoney shares = shareSplit.shares();

      if (transactionType == MyMoneySplit::BuyShares) {
        if (reportedDateRange) {
          cfList[Buys].append(CashFlowListItem(postDate, value));
          shList[Buys] += shares;
        }

        if (shList.at(BuysOfOwned).isZero()) {                      // add sold shares
          if (shList.at(BuysOfSells) + shares > shList.at(Sells).abs()) { // add partially sold shares
            MyMoneyMoney tempVal = (((shList.at(Sells).abs() - shList.at(BuysOfSells))) / shares) * value;
            cfList[BuysOfSells].append(CashFlowListItem(postDate, tempVal));
            shList[BuysOfSells] = shList.at(Sells).abs();
            if (isSTLT && postDate < termSeparator) {
              cfList[LongTermBuysOfSells].append(CashFlowListItem(postDate, tempVal));
              shList[LongTermBuysOfSells] = shList.at(BuysOfSells);
            }
          } else {                                                  // add wholly sold shares
            cfList[BuysOfSells].append(CashFlowListItem(postDate, value));
            shList[BuysOfSells] += shares;
            if (isSTLT && postDate < termSeparator) {
              cfList[LongTermBuysOfSells].append(CashFlowListItem(postDate, value));
              shList[LongTermBuysOfSells] += shares;
            }
          }
        } else if (shList.at(BuysOfOwned) >= shares) {              // substract not-sold shares
          shList[BuysOfOwned] -= shares;
          cfList[BuysOfOwned].append(CashFlowListItem(postDate, value));
        } else {                                                    // substract partially not-sold shares
          MyMoneyMoney tempVal = ((shares - shList.at(BuysOfOwned)) / shares) * value;
          MyMoneyMoney tempVal2 = (shares - shList.at(BuysOfOwned));
          cfList[BuysOfSells].append(CashFlowListItem(postDate, tempVal));
          shList[BuysOfSells] += tempVal2;
          if (isSTLT && postDate < termSeparator) {
            cfList[LongTermBuysOfSells].append(CashFlowListItem(postDate, tempVal));
            shList[LongTermBuysOfSells] += tempVal2;
          }
          cfList[BuysOfOwned].append(CashFlowListItem(postDate, (shList.at(BuysOfOwned) / shares) * value));
          shList[BuysOfOwned] = MyMoneyMoney();
        }
      } else if (transactionType == MyMoneySplit::SellShares && reportedDateRange) {
        cfList[Sells].append(CashFlowListItem(postDate, value));
        shList[Sells] += shares;
      } else if (transactionType == MyMoneySplit::SplitShares) {          // shares variable is denominator of split ratio here
        for (int i = Buys; i <= InvestmentValue::BuysOfOwned; ++i)
          shList[i] /= shares;
      } else if (transactionType == MyMoneySplit::AddShares ||            // added shares, when sold give 100% capital gain
                 transactionType == MyMoneySplit::ReinvestDividend) {
        if (shList.at(BuysOfOwned).isZero()) {                            // add added/reinvested shares
          if (shList.at(BuysOfSells) + shares > shList.at(Sells).abs()) { // add partially added/reinvested shares
            shList[BuysOfSells] = shList.at(Sells).abs();
            if (postDate < termSeparator)
              shList[LongTermBuysOfSells] = shList[BuysOfSells];
          } else {                                                        // add wholly added/reinvested shares
            shList[BuysOfSells] += shares;
            if (postDate < termSeparator)
              shList[LongTermBuysOfSells] += shares;
          }
        } else if (shList.at(BuysOfOwned) >= shares) {                    // substract not-added/not-reinvested shares
          shList[BuysOfOwned] -= shares;
          cfList[BuysOfOwned].append(CashFlowListItem(postDate, value));
        } else {                                                          // substract partially not-added/not-reinvested shares
          MyMoneyMoney tempVal = (shares - shList.at(BuysOfOwned));
          shList[BuysOfSells] += tempVal;
          if (postDate < termSeparator)
            shList[LongTermBuysOfSells] += tempVal;

          cfList[BuysOfOwned].append(CashFlowListItem(postDate, (shList.at(BuysOfOwned) / shares) * value));
          shList[BuysOfOwned] = MyMoneyMoney();
        }
        if (transactionType == MyMoneySplit::ReinvestDividend) {
          value = MyMoneyMoney();
          foreach (const auto split, interestSplits)
            value += split.value();
          value *= price;
          cfList[ReinvestIncome].append(CashFlowListItem(postDate, -value));
        }
      } else if (transactionType == MyMoneySplit::RemoveShares && reportedDateRange) // removed shares give no value in return so no capital gain on them
        shList[Sells] += shares;
      else if (transactionType == MyMoneySplit::Dividend || transactionType == MyMoneySplit::Yield)
        cfList[CashIncome].append(CashFlowListItem(postDate, value));

    }
    reportedDateRange = false;
    newEndingDate = newStartingDate;
    newStartingDate = newStartingDate.addYears(-1);
    report.setDateFilter(newStartingDate, newEndingDate); // search for matching buy transactions year earlier

  } while (
           (
             (report.investmentSum() == MyMoneyReport::eSumOwned && !shList[BuysOfOwned].isZero()) ||
             (report.investmentSum() == MyMoneyReport::eSumSold && !shList.at(Sells).isZero() && shList.at(Sells).abs() > shList.at(BuysOfSells).abs()) ||
             (report.investmentSum() == MyMoneyReport::eSumOwnedAndSold && (!shList[BuysOfOwned].isZero() || (!shList.at(Sells).isZero() && shList.at(Sells).abs() > shList.at(BuysOfSells).abs())))
           ) && account.openingDate() <= newEndingDate
          );

  // we've got buy value and no sell value of long-term shares, so get them
  if (isSTLT && !shList[LongTermBuysOfSells].isZero()) {
    newStartingDate = startingDate;
    newEndingDate = endingDate.addDays(-settlementPeriod);
    report.setDateFilter(newStartingDate, newEndingDate); // search for matching buy transactions year earlier
    QList<MyMoneyTransaction> transactions = file->transactionList(report);
    shList[BuysOfOwned] = shList[LongTermBuysOfSells];

    foreach (const auto transaction, transactions) {
      MyMoneySplit shareSplit = transaction.splitByAccount(account.id());
      MyMoneySplit assetAccountSplit;
      QList<MyMoneySplit> feeSplits;
      QList<MyMoneySplit> interestSplits;
      MyMoneySecurity security;
      MyMoneySecurity currency;
      MyMoneySplit::investTransactionTypeE transactionType;
      KMyMoneyUtils::dissectTransaction(transaction, shareSplit, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
      QDate postDate = transaction.postDate();
      MyMoneyMoney price;
      if (m_config.isConvertCurrency())
        price = account.baseCurrencyPrice(postDate); //we only need base currency because the value is in deep currency
      else
        price = MyMoneyMoney::ONE;
      MyMoneyMoney value = assetAccountSplit.value() * price;
      MyMoneyMoney shares = shareSplit.shares();

      if (transactionType == MyMoneySplit::SellShares) {
        if ((shList.at(LongTermSellsOfBuys) + shares).abs() >= shList.at(LongTermBuysOfSells)) { // add partially sold long-term shares
          cfList[LongTermSellsOfBuys].append(CashFlowListItem(postDate, (shList.at(LongTermSellsOfBuys).abs() - shList.at(LongTermBuysOfSells)) / shares * value));
          shList[LongTermSellsOfBuys] = shList.at(LongTermBuysOfSells);
          break;
        } else {                      // add wholly sold long-term shares
          cfList[LongTermSellsOfBuys].append(CashFlowListItem(postDate, value));
          shList[LongTermSellsOfBuys] += shares;
        }
      } else if (transactionType == MyMoneySplit::RemoveShares) {
        if ((shList.at(LongTermSellsOfBuys) + shares).abs() >= shList.at(LongTermBuysOfSells)) {
          shList[LongTermSellsOfBuys] = shList.at(LongTermBuysOfSells);
          break;
        } else
          shList[LongTermSellsOfBuys] += shares;
      }
    }
  }

  shList[BuysOfOwned] = stashedBuysOfOwned;
  report.setDateFilter(startingDate, endingDate); // reset data filter for next security
  return;
}

void QueryTable::constructPerformanceRow(const ReportAccount& account, TableRow& result, CashFlowList &all) const
{
  MyMoneyReport report = m_config;
  QDate startingDate;
  QDate endingDate;
  report.validDateRange(startingDate, endingDate);
  startingDate = startingDate.addDays(-1);

  MyMoneyFile* file = MyMoneyFile::instance();
  //get fraction depending on type of account
  int fraction = account.currency().smallestAccountFraction();
  MyMoneyMoney price;
  if (m_config.isConvertCurrency())
    price = account.deepCurrencyPrice(startingDate) * account.baseCurrencyPrice(startingDate);
  else
    price = account.deepCurrencyPrice(startingDate);

  MyMoneyMoney startingBal = file->balance(account.id(), startingDate) * price;

  //convert to lowest fraction
  startingBal = startingBal.convert(fraction);

  //calculate ending balance
  if (m_config.isConvertCurrency())
    price = account.deepCurrencyPrice(endingDate) * account.baseCurrencyPrice(endingDate);
  else
    price = account.deepCurrencyPrice(endingDate);

  MyMoneyMoney endingBal = file->balance((account).id(), endingDate) * price;

  //convert to lowest fraction
  endingBal = endingBal.convert(fraction);

  QList<CashFlowList> cfList;
  QList<MyMoneyMoney> shList;
  sumInvestmentValues(account, cfList, shList);

  MyMoneyMoney buysTotal;
  MyMoneyMoney sellsTotal;
  MyMoneyMoney cashIncomeTotal;
  MyMoneyMoney reinvestIncomeTotal;

  switch (m_config.investmentSum()) {
  case MyMoneyReport::eSumOwnedAndSold:
    buysTotal = cfList.at(BuysOfSells).total() + cfList.at(BuysOfOwned).total();
    sellsTotal = cfList.at(Sells).total();
    cashIncomeTotal = cfList.at(CashIncome).total();
    reinvestIncomeTotal = cfList.at(ReinvestIncome).total();
    startingBal = MyMoneyMoney();
    if (buysTotal.isZero() && sellsTotal.isZero() &&
        cashIncomeTotal.isZero() && reinvestIncomeTotal.isZero())
      return;

    all.append(cfList.at(BuysOfSells));
    all.append(cfList.at(BuysOfOwned));
    all.append(cfList.at(Sells));
    all.append(cfList.at(CashIncome));

    result[ctSells] = sellsTotal.toString();
    result[ctCashIncome] = cashIncomeTotal.toString();
    result[ctReinvestIncome] = reinvestIncomeTotal.toString();
    result[ctEndingBalance] = endingBal.toString();
    break;
  case MyMoneyReport::eSumOwned:
    buysTotal = cfList.at(BuysOfOwned).total();
    startingBal = MyMoneyMoney();
    if (buysTotal.isZero() && endingBal.isZero())
      return;
    all.append(cfList.at(BuysOfOwned));
    all.append(CashFlowListItem(endingDate, endingBal));

    result[ctReinvestIncome] = reinvestIncomeTotal.toString();
    result[ctMarketValue] = endingBal.toString();
    break;
  case MyMoneyReport::eSumSold:
    buysTotal = cfList.at(BuysOfSells).total();
    sellsTotal = cfList.at(Sells).total();
    cashIncomeTotal = cfList.at(CashIncome).total();
    startingBal = endingBal = MyMoneyMoney();
    // check if there are any meaningfull values before adding them to results
    if (buysTotal.isZero() && sellsTotal.isZero() && cashIncomeTotal.isZero())
      return;
    all.append(cfList.at(BuysOfSells));
    all.append(cfList.at(Sells));
    all.append(cfList.at(CashIncome));

    result[ctSells] = sellsTotal.toString();
    result[ctCashIncome] = cashIncomeTotal.toString();
    break;
  case MyMoneyReport::eSumPeriod:
  default:
    buysTotal = cfList.at(Buys).total();
    sellsTotal = cfList.at(Sells).total();
    cashIncomeTotal = cfList.at(CashIncome).total();
    reinvestIncomeTotal = cfList.at(ReinvestIncome).total();
    if (buysTotal.isZero() && sellsTotal.isZero() &&
        cashIncomeTotal.isZero() && reinvestIncomeTotal.isZero() &&
        startingBal.isZero() && endingBal.isZero())
      return;

    all.append(cfList.at(Buys));
    all.append(cfList.at(Sells));
    all.append(cfList.at(CashIncome));
    all.append(CashFlowListItem(startingDate, -startingBal));
    all.append(CashFlowListItem(endingDate, endingBal));

    result[ctSells] = sellsTotal.toString();
    result[ctCashIncome] = cashIncomeTotal.toString();
    result[ctReinvestIncome] = reinvestIncomeTotal.toString();
    result[ctStartingBalance] = startingBal.toString();
    result[ctEndingBalance] = endingBal.toString();
    break;
  }

  MyMoneyMoney returnInvestment = helperROI(buysTotal - reinvestIncomeTotal, sellsTotal, startingBal, endingBal, cashIncomeTotal);
  MyMoneyMoney annualReturn = helperIRR(all);

  result[ctBuys] = buysTotal.toString();
  result[ctReturn] = annualReturn.toString();
  result[ctReturnInvestment] = returnInvestment.toString();
  result[ctEquityType] = MyMoneySecurity::securityTypeToString(file->security(account.currencyId()).securityType());
}

void QueryTable::constructCapitalGainRow(const ReportAccount& account, TableRow& result) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<CashFlowList> cfList;
  QList<MyMoneyMoney> shList;
  sumInvestmentValues(account, cfList, shList);

  MyMoneyMoney buysTotal = cfList.at(BuysOfSells).total();
  MyMoneyMoney sellsTotal = cfList.at(Sells).total();
  MyMoneyMoney longTermBuysOfSellsTotal = cfList.at(LongTermBuysOfSells).total();
  MyMoneyMoney longTermSellsOfBuys = cfList.at(LongTermSellsOfBuys).total();

  switch (m_config.investmentSum()) {
  case MyMoneyReport::eSumOwned:
  {
    if (shList.at(BuysOfOwned).isZero())
      return;

    MyMoneyReport report = m_config;
    QDate startingDate;
    QDate endingDate;
    report.validDateRange(startingDate, endingDate);

    //get fraction depending on type of account
    int fraction = account.currency().smallestAccountFraction();
    MyMoneyMoney price;

    //calculate ending balance
    if (m_config.isConvertCurrency())
      price = account.deepCurrencyPrice(endingDate) * account.baseCurrencyPrice(endingDate);
    else
      price = account.deepCurrencyPrice(endingDate);

    MyMoneyMoney endingBal = shList.at(BuysOfOwned) * price;

    //convert to lowest fraction
    endingBal = endingBal.convert(fraction);

    buysTotal = cfList.at(BuysOfOwned).total() - cfList.at(ReinvestIncome).total();

    int pricePrecision = file->security(account.currencyId()).pricePrecision();
    result[ctBuys] = buysTotal.toString();
    result[ctShares] = shList.at(BuysOfOwned).toString();
    result[ctBuyPrice] = (buysTotal.abs() / shList.at(BuysOfOwned)).convertPrecision(pricePrecision).toString();
    result[ctLastPrice] = price.toString();
    result[ctMarketValue] = endingBal.toString();
    result[ctCapitalGain] = (buysTotal + endingBal).toString();
    result[ctPercentageGain] = ((buysTotal + endingBal)/buysTotal.abs()).toString();
    break;
  }
  case MyMoneyReport::eSumSold:
  default:
    buysTotal = cfList.at(BuysOfSells).total() - cfList.at(ReinvestIncome).total();
    sellsTotal = cfList.at(Sells).total();
    longTermBuysOfSellsTotal = cfList.at(LongTermBuysOfSells).total();
    longTermSellsOfBuys = cfList.at(LongTermSellsOfBuys).total();
    // check if there are any meaningfull values before adding them to results
    if (buysTotal.isZero() && sellsTotal.isZero() &&
        longTermBuysOfSellsTotal.isZero() && longTermSellsOfBuys.isZero())
      return;

    result[ctBuys] = buysTotal.toString();
    result[ctSells] = sellsTotal.toString();
    result[ctCapitalGain] = (buysTotal + sellsTotal).toString();
    if (m_config.isShowingSTLTCapitalGains()) {
      result[ctBuysLT] = longTermBuysOfSellsTotal.toString();
      result[ctSellsLT] = longTermSellsOfBuys.toString();
      result[ctCapitalGainLT] = (longTermBuysOfSellsTotal + longTermSellsOfBuys).toString();
      result[ctBuysST] = (buysTotal - longTermBuysOfSellsTotal).toString();
      result[ctSellsST] = (sellsTotal - longTermSellsOfBuys).toString();
      result[ctCapitalGainST] = ((buysTotal - longTermBuysOfSellsTotal) + (sellsTotal - longTermSellsOfBuys)).toString();
    }
    break;
  }

  result[ctEquityType] = MyMoneySecurity::securityTypeToString(file->security(account.currencyId()).securityType());
}

void QueryTable::constructAccountTable()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //make sure we have all subaccounts of investment accounts
  includeInvestmentSubAccounts();

  QMap<QString, QMap<QString, CashFlowList>> currencyCashFlow; // for total calculation
  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);
  for (auto it_account = accounts.constBegin(); it_account != accounts.constEnd(); ++it_account) {
    // Note, "Investment" accounts are never included in account rows because
    // they don't contain anything by themselves.  In reports, they are only
    // useful as a "topaccount" aggregator of stock accounts
    if ((*it_account).isAssetLiability() && m_config.includes((*it_account)) && (*it_account).accountType() != eMyMoney::Account::Investment) {
      // don't add the account if it is closed. In fact, the business logic
      // should prevent that an account can be closed with a balance not equal
      // to zero, but we never know.
      MyMoneyMoney shares = file->balance((*it_account).id(), m_config.toDate());
      if (shares.isZero() && (*it_account).isClosed())
        continue;

      ReportAccount account(*it_account);
      TableRow qaccountrow;
      CashFlowList accountCashflow; // for total calculation
      switch(m_config.queryColumns()) {
        case MyMoneyReport::eQCperformance:
        {
          constructPerformanceRow(account, qaccountrow, accountCashflow);
          if (!qaccountrow.isEmpty()) {
            // assuming that that report is grouped by topaccount
            qaccountrow[ctTopAccount] = account.topParentName();
            if (!m_containsNonBaseCurrency && account.currency().id() != file->baseCurrency().id())
              m_containsNonBaseCurrency = true;
            if (m_config.isConvertCurrency())
              qaccountrow[ctCurrency] = file->baseCurrency().id();
            else
              qaccountrow[ctCurrency] = account.currency().id();

            if (!currencyCashFlow.value(qaccountrow.value(ctCurrency)).contains(qaccountrow.value(ctTopAccount)))
              currencyCashFlow[qaccountrow.value(ctCurrency)].insert(qaccountrow.value(ctTopAccount), accountCashflow);   // create cashflow for unknown account...
            else
              currencyCashFlow[qaccountrow.value(ctCurrency)][qaccountrow.value(ctTopAccount)] += accountCashflow;        // ...or add cashflow for known account
          }
          break;
        }
        case MyMoneyReport::eQCcapitalgain:
          constructCapitalGainRow(account, qaccountrow);
          break;
        default:
        {
          //get fraction for account
          int fraction = account.currency().smallestAccountFraction() != -1 ?
                account.currency().smallestAccountFraction() : file->baseCurrency().smallestAccountFraction();

          MyMoneyMoney netprice = account.deepCurrencyPrice(m_config.toDate());
          if (m_config.isConvertCurrency() && account.isForeignCurrency())
            netprice *= account.baseCurrencyPrice(m_config.toDate()); // display currency is base currency, so set the price

          netprice = netprice.reduce();
          shares = shares.reduce();
          int pricePrecision = file->security(account.currencyId()).pricePrecision();
          qaccountrow[ctPrice] = netprice.convertPrecision(pricePrecision).toString();
          qaccountrow[ctValue] = (netprice * shares).convert(fraction).toString();
          qaccountrow[ctShares] = shares.toString();

          QString iid = account.institutionId();

          // If an account does not have an institution, get it from the top-parent.
          if (iid.isEmpty() && !account.isTopLevel())
            iid = account.topParent().institutionId();

          if (iid.isEmpty())
            qaccountrow[ctInstitution] = i18nc("No institution", "None");
          else
            qaccountrow[ctInstitution] = file->institution(iid).name();

          qaccountrow[ctType] = KMyMoneyUtils::accountTypeToString(account.accountType());
        }
      }

      if (qaccountrow.isEmpty()) // don't add the account if there are no calculated values
        continue;

      qaccountrow[ctRank] = QLatin1Char('1');
      qaccountrow[ctAccount] = account.name();
      qaccountrow[ctAccountID] = account.id();
      qaccountrow[ctTopAccount] = account.topParentName();
      if (!m_containsNonBaseCurrency && account.currency().id() != file->baseCurrency().id())
        m_containsNonBaseCurrency = true;
      if (m_config.isConvertCurrency())
        qaccountrow[ctCurrency] = file->baseCurrency().id();
      else
        qaccountrow[ctCurrency] = account.currency().id();
      m_rows.append(qaccountrow);
    }
  }

  if (m_config.queryColumns() == MyMoneyReport::eQCperformance && m_config.isShowingColumnTotals()) {
    TableRow qtotalsrow;
    qtotalsrow[ctRank] = QLatin1Char('4'); // add identification of row as total
    QMap<QString, CashFlowList> currencyGrandCashFlow;

    QMap<QString, QMap<QString, CashFlowList>>::iterator currencyAccGrp = currencyCashFlow.begin();
    while (currencyAccGrp != currencyCashFlow.end()) {
      // convert map of top accounts with cashflows to TableRow
      for (QMap<QString, CashFlowList>::iterator topAccount = (*currencyAccGrp).begin(); topAccount != (*currencyAccGrp).end(); ++topAccount) {
        qtotalsrow[ctTopAccount] = topAccount.key();
        qtotalsrow[ctReturn] = helperIRR(topAccount.value()).toString();
        qtotalsrow[ctCurrency] = currencyAccGrp.key();
        currencyGrandCashFlow[currencyAccGrp.key()] += topAccount.value();  // cumulative sum of cashflows of each topaccount
        m_rows.append(qtotalsrow);            // rows aren't sorted yet, so no problem with adding them randomly at the end
      }
      ++currencyAccGrp;
    }
    QMap<QString, CashFlowList>::iterator currencyGrp = currencyGrandCashFlow.begin();
    qtotalsrow[ctTopAccount].clear();          // empty topaccount because it's grand cashflow
    while (currencyGrp != currencyGrandCashFlow.end()) {
      qtotalsrow[ctReturn] = helperIRR(currencyGrp.value()).toString();
      qtotalsrow[ctCurrency] = currencyGrp.key();
      m_rows.append(qtotalsrow);
      ++currencyGrp;
    }
  }
}

void QueryTable::constructSplitsTable()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //make sure we have all subaccounts of investment accounts
  includeInvestmentSubAccounts();

  MyMoneyReport report(m_config);
  report.setReportAllSplits(false);
  report.setConsiderCategory(true);

  // support for opening and closing balances
  QMap<QString, MyMoneyAccount> accts;

  //get all transactions for this report
  QList<MyMoneyTransaction> transactions = file->transactionList(report);
  for (QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.constBegin(); it_transaction != transactions.constEnd(); ++it_transaction) {

    TableRow qA, qS;
    QDate pd;

    qA[ctID] = qS[ctID] = (* it_transaction).id();
    qA[ctEntryDate] = qS[ctEntryDate] = (* it_transaction).entryDate().toString(Qt::ISODate);
    qA[ctPostDate] = qS[ctPostDate] = (* it_transaction).postDate().toString(Qt::ISODate);
    qA[ctCommodity] = qS[ctCommodity] = (* it_transaction).commodity();

    pd = (* it_transaction).postDate();
    qA[ctMonth] = qS[ctMonth] = i18n("Month of %1", QDate(pd.year(), pd.month(), 1).toString(Qt::ISODate));
    qA[ctWeek] = qS[ctWeek] = i18n("Week of %1", pd.addDays(1 - pd.dayOfWeek()).toString(Qt::ISODate));

    if (!m_containsNonBaseCurrency && (*it_transaction).commodity() != file->baseCurrency().id())
      m_containsNonBaseCurrency = true;
    if (report.isConvertCurrency())
      qA[ctCurrency] = qS[ctCurrency] = file->baseCurrency().id();
    else
      qA[ctCurrency] = qS[ctCurrency] = (*it_transaction).commodity();

    // to handle splits, we decide on which account to base the split
    // (a reference point or point of view so to speak). here we take the
    // first account that is a stock account or loan account (or the first account
    // that is not an income or expense account if there is no stock or loan account)
    // to be the account (qA) that will have the sub-item "split" entries. we add
    // one transaction entry (qS) for each subsequent entry in the split.
    const QList<MyMoneySplit>& splits = (*it_transaction).splits();
    QList<MyMoneySplit>::const_iterator myBegin, it_split;
    //S_end = splits.end();

    for (it_split = splits.constBegin(), myBegin = splits.constEnd(); it_split != splits.constEnd(); ++it_split) {
      ReportAccount splitAcc = (* it_split).accountId();
      // always put split with a "stock" account if it exists
      if (splitAcc.isInvest())
        break;

      // prefer to put splits with a "loan" account if it exists
      if (splitAcc.isLoan())
        myBegin = it_split;

      if ((myBegin == splits.end()) && ! splitAcc.isIncomeExpense()) {
        myBegin = it_split;
      }
    }

    // select our "reference" split
    if (it_split == splits.end()) {
      it_split = myBegin;
    } else {
      myBegin = it_split;
    }

    // if the split is still unknown, use the first one. I have seen this
    // happen with a transaction that has only a single split referencing an income or expense
    // account and has an amount and value of 0. Such a transaction will fall through
    // the above logic and leave 'it_split' pointing to splits.end() which causes the remainder
    // of this to end in an infinite loop.
    if (it_split == splits.end()) {
      it_split = splits.begin();
    }

    // for "loan" reports, the loan transaction gets special treatment.
    // the splits of a loan transaction are placed on one line in the
    // reference (loan) account (qA). however, we process the matching
    // split entries (qS) normally.
    bool loan_special_case = false;
    if (m_config.queryColumns() & MyMoneyReport::eQCloan) {
      ReportAccount splitAcc = (*it_split).accountId();
      loan_special_case = splitAcc.isLoan();
    }

    // There is a slight chance that at this point myBegin is still pointing to splits.end() if the
    // transaction only has income and expense splits (which should not happen). In that case, point
    // it to the first split
    if (myBegin == splits.end()) {
      myBegin = splits.begin();
    }

    //the account of the beginning splits
    ReportAccount myBeginAcc = (*myBegin).accountId();

    bool include_me = true;
    QString a_fullname;
    QString a_memo;
    int pass = 1;

    do {
      MyMoneyMoney xr;
      ReportAccount splitAcc = (* it_split).accountId();

      //get fraction for account
      int fraction = splitAcc.currency().smallestAccountFraction();

      //use base currency fraction if not initialized
      if (fraction == -1)
        fraction = file->baseCurrency().smallestAccountFraction();

      QString institution = splitAcc.institutionId();
      QString payee = (*it_split).payeeId();

      const QList<QString> tagIdList = (*it_split).tagIdList();

      if (m_config.isConvertCurrency()) {
        xr = (splitAcc.deepCurrencyPrice((*it_transaction).postDate()) * splitAcc.baseCurrencyPrice((*it_transaction).postDate())).reduce();
      } else {
        xr = splitAcc.deepCurrencyPrice((*it_transaction).postDate()).reduce();
      }

      // reverse the sign of incomes and expenses to keep consistency in the way it is displayed in other reports
      if (splitAcc.isIncomeExpense()) {
        xr = -xr;
      }

      if (splitAcc.isInvest()) {

        // use the institution of the parent for stock accounts
        institution = splitAcc.parent().institutionId();
        MyMoneyMoney shares = (*it_split).shares();
        int pricePrecision = file->security(splitAcc.currencyId()).pricePrecision();
        qA[ctAction] = (*it_split).action();
        qA[ctShares] = shares.isZero() ? QString() : (*it_split).shares().toString();
        qA[ctPrice] = shares.isZero() ? QString() : xr.convertPrecision(pricePrecision).toString();

        if (((*it_split).action() == MyMoneySplit::ActionBuyShares) && (*it_split).shares().isNegative())
          qA[ctAction] = "Sell";

        qA[ctInvestAccount] = splitAcc.parent().name();
      }

      include_me = m_config.includes(splitAcc);
      a_fullname = splitAcc.fullName();
      a_memo = (*it_split).memo();

      int pricePrecision = file->security(splitAcc.currencyId()).pricePrecision();
      qA[ctPrice] = xr.convertPrecision(pricePrecision).toString();
      qA[ctAccount] = splitAcc.name();
      qA[ctAccountID] = splitAcc.id();
      qA[ctTopAccount] = splitAcc.topParentName();

      qA[ctInstitution] = institution.isEmpty()
                          ? i18n("No Institution")
                          : file->institution(institution).name();

      //FIXME-ALEX Is this useless? Isn't constructSplitsTable called only for cashflow type report?
      QString delimiter;
      foreach(const auto tagId, tagIdList) {
        qA[ctTag] += delimiter + file->tag(tagId).name().simplified();
        delimiter = QLatin1Char(',');
      }

      qA[ctPayee] = payee.isEmpty()
                    ? i18n("[Empty Payee]")
                    : file->payee(payee).name().simplified();

      qA[ctReconcileDate] = (*it_split).reconcileDate().toString(Qt::ISODate);
      qA[ctReconcileFlag] = KMyMoneyUtils::reconcileStateToString((*it_split).reconcileFlag(), true);
      qA[ctNumber] = (*it_split).number();

      qA[ctMemo] = a_memo;

      qS[ctReconcileDate] = qA[ctReconcileDate];
      qS[ctReconcileFlag] = qA[ctReconcileFlag];
      qS[ctNumber] = qA[ctNumber];

      qS[ctTopCategory] = splitAcc.topParentName();

      // only include the configured accounts
      if (include_me) {
        // add the "summarized" split transaction
        // this is the sub-total of the split detail
        // convert to lowest fraction
        qA[ctValue] = ((*it_split).shares() * xr).convert(fraction).toString();
        qA[ctRank] = QLatin1Char('1');

        //fill in account information
        if (! splitAcc.isIncomeExpense() && it_split != myBegin) {
          qA[ctAccount] = ((*it_split).shares().isNegative()) ?
                          i18n("Transfer to %1", myBeginAcc.fullName())
                          : i18n("Transfer from %1", myBeginAcc.fullName());
        } else if (it_split == myBegin) {
          //handle the main split
          if ((splits.count() > 2)) {
            //if it is the main split and has multiple splits, note that
            qA[ctAccount] = i18n("[Split Transaction]");
          } else {
            //fill the account name of the second split
            QList<MyMoneySplit>::const_iterator tempSplit = splits.constBegin();

            //there are supposed to be only 2 splits if we ever get here
            if (tempSplit == myBegin && splits.count() > 1)
              ++tempSplit;

            //show the name of the category, or "transfer to/from" if it as an account
            ReportAccount tempSplitAcc = (*tempSplit).accountId();
            if (! tempSplitAcc.isIncomeExpense()) {
              qA[ctAccount] = ((*it_split).shares().isNegative()) ?
                              i18n("Transfer to %1", tempSplitAcc.fullName())
                              : i18n("Transfer from %1", tempSplitAcc.fullName());
            } else {
              qA[ctAccount] = tempSplitAcc.fullName();
            }
          }
        } else {
          //in any other case, fill in the account name of the main split
          qA[ctAccount] = myBeginAcc.fullName();
        }

        //category data is always the one of the split
        qA [ctCategory] = splitAcc.fullName();
        qA [ctTopCategory] = splitAcc.topParentName();
        qA [ctCategoryType] = KMyMoneyUtils::accountTypeToString(splitAcc.accountGroup());

        m_rows += qA;

        // track accts that will need opening and closing balances
        accts.insert(splitAcc.id(), splitAcc);
      }
      ++it_split;

      // look for wrap-around
      if (it_split == splits.end())
        it_split = splits.begin();

      //check if there have been more passes than there are splits
      //this is to prevent infinite loops in cases of data inconsistency -- asoliverez
      ++pass;
      if (pass > splits.count())
        break;

    } while (it_split != myBegin);

    if (loan_special_case) {
      m_rows += qA;
    }
  }

  // now run through our accts list and add opening and closing balances

  switch (m_config.rowType()) {
    case MyMoneyReport::eAccount:
    case MyMoneyReport::eTopAccount:
      break;

      // case MyMoneyReport::eCategory:
      // case MyMoneyReport::eTopCategory:
      // case MyMoneyReport::ePayee:
      // case MyMoneyReport::eMonth:
      // case MyMoneyReport::eWeek:
    default:
      return;
  }

  QDate startDate, endDate;

  report.validDateRange(startDate, endDate);
  QString strStartDate = startDate.toString(Qt::ISODate);
  QString strEndDate = endDate.toString(Qt::ISODate);
  startDate = startDate.addDays(-1);

  QMap<QString, MyMoneyAccount>::const_iterator it_account, accts_end;
  for (it_account = accts.constBegin(); it_account != accts.constEnd(); ++it_account) {
    TableRow qA;

    ReportAccount account = (* it_account);

    //get fraction for account
    int fraction = account.currency().smallestAccountFraction();

    //use base currency fraction if not initialized
    if (fraction == -1)
      fraction = file->baseCurrency().smallestAccountFraction();

    QString institution = account.institutionId();

    // use the institution of the parent for stock accounts
    if (account.isInvest())
      institution = account.parent().institutionId();

    MyMoneyMoney startBalance, endBalance, startPrice, endPrice;
    MyMoneyMoney startShares, endShares;

    //get price and convert currency if necessary
    if (m_config.isConvertCurrency()) {
      startPrice = (account.deepCurrencyPrice(startDate) * account.baseCurrencyPrice(startDate)).reduce();
      endPrice = (account.deepCurrencyPrice(endDate) * account.baseCurrencyPrice(endDate)).reduce();
    } else {
      startPrice = account.deepCurrencyPrice(startDate).reduce();
      endPrice = account.deepCurrencyPrice(endDate).reduce();
    }
    startShares = file->balance(account.id(), startDate);
    endShares = file->balance(account.id(), endDate);

    //get starting and ending balances
    startBalance = startShares * startPrice;
    endBalance = endShares * endPrice;

    //starting balance
    // don't show currency if we're converting or if it's not foreign
    if (!m_containsNonBaseCurrency && account.currency().id() != file->baseCurrency().id())
      m_containsNonBaseCurrency = true;
    if (m_config.isConvertCurrency())
      qA[ctCurrency] = file->baseCurrency().id();
    else
      qA[ctCurrency] = account.currency().id();

    qA[ctAccountID] = account.id();
    qA[ctAccount] = account.name();
    qA[ctTopAccount] = account.topParentName();
    qA[ctInstitution] = institution.isEmpty() ? i18n("No Institution") : file->institution(institution).name();
    qA[ctRank] = QLatin1Char('0');

    int pricePrecision = file->security(account.currencyId()).pricePrecision();
    qA[ctPrice] = startPrice.convertPrecision(pricePrecision).toString();
    if (account.isInvest()) {
      qA[ctShares] = startShares.toString();
    }

    qA[ctPostDate] = strStartDate;
    qA[ctBalance] = startBalance.convert(fraction).toString();
    qA[ctValue].clear();
    qA[ctID] = QLatin1Char('A');
    m_rows += qA;

    qA[ctRank] = QLatin1Char('3');
    //ending balance
    qA[ctPrice] = endPrice.convertPrecision(pricePrecision).toString();

    if (account.isInvest()) {
      qA[ctShares] = endShares.toString();
    }

    qA[ctPostDate] = strEndDate;
    qA[ctBalance] = endBalance.toString();
    qA[ctID] = QLatin1Char('Z');
    m_rows += qA;
  }
}



}
