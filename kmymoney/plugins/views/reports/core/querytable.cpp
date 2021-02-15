/*
 * SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

#include "cashflowlist.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyinstitution.h"
#include "mymoneyprice.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyreport.h"
#include "mymoneyexception.h"
#include "mymoneyutils.h"
#include "kmymoneyutils.h"
#include "reportaccount.h"
#include "mymoneyenums.h"

namespace reports
{

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
    case eMyMoney::Report::RowType::AccountByTopAccount:
    case eMyMoney::Report::RowType::EquityType:
    case eMyMoney::Report::RowType::AccountType:
    case eMyMoney::Report::RowType::Institution:
      constructAccountTable();
      m_columns << ctAccount;
      break;

    case eMyMoney::Report::RowType::Account:
      constructTransactionTable();
      m_columns << ctAccountID << ctPostDate;
      break;

    case eMyMoney::Report::RowType::Payee:
    case eMyMoney::Report::RowType::Tag:
    case eMyMoney::Report::RowType::Month:
    case eMyMoney::Report::RowType::Week:
      constructTransactionTable();
      m_columns << ctPostDate << ctAccount;
      break;
    case eMyMoney::Report::RowType::CashFlow:
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
    case eMyMoney::Report::RowType::CashFlow:
      m_group << ctCategoryType << ctTopCategory << ctCategory;
      break;
    case eMyMoney::Report::RowType::Category:
      m_group << ctCategoryType << ctTopCategory << ctCategory;
      break;
    case eMyMoney::Report::RowType::TopCategory:
      m_group << ctCategoryType << ctTopCategory;
      break;
    case eMyMoney::Report::RowType::TopAccount:
      m_group << ctTopAccount << ctAccount;
      break;
    case eMyMoney::Report::RowType::Account:
      m_group << ctAccount;
      break;
    case eMyMoney::Report::RowType::AccountReconcile:
      m_group << ctAccount << ctReconcileFlag;
      break;
    case eMyMoney::Report::RowType::Payee:
      m_group << ctPayee;
      break;
    case eMyMoney::Report::RowType::Tag:
      m_group << ctTag;
      break;
    case eMyMoney::Report::RowType::Month:
      m_group << ctMonth;
      break;
    case eMyMoney::Report::RowType::Week:
      m_group << ctWeek;
      break;
    case eMyMoney::Report::RowType::AccountByTopAccount:
      m_group << ctTopAccount;
      break;
    case eMyMoney::Report::RowType::EquityType:
      m_group << ctEquityType;
      break;
    case eMyMoney::Report::RowType::AccountType:
      m_group << ctType;
      break;
    case eMyMoney::Report::RowType::Institution:
      m_group << ctInstitution << ctTopAccount;
      break;
    default:
      throw MYMONEYEXCEPTION_CSTRING("QueryTable::QueryTable(): unhandled row type");
  }

  QVector<cellTypeE> sort = QVector<cellTypeE>::fromList(m_group) << QVector<cellTypeE>::fromList(m_columns) << ctID << ctRank;

  m_columns.clear();
  switch (m_config.rowType()) {
    case eMyMoney::Report::RowType::AccountByTopAccount:
    case eMyMoney::Report::RowType::EquityType:
    case eMyMoney::Report::RowType::AccountType:
    case eMyMoney::Report::RowType::Institution:
      m_columns << ctAccount;
      break;

    default:
      m_columns << ctPostDate;
  }

  unsigned qc = m_config.queryColumns();

  if (qc & eMyMoney::Report::QueryColumn::Number)
    m_columns << ctNumber;
  if (qc & eMyMoney::Report::QueryColumn::Payee)
    m_columns << ctPayee;
  if (qc & eMyMoney::Report::QueryColumn::Tag)
    m_columns << ctTag;
  if (qc & eMyMoney::Report::QueryColumn::Category)
    m_columns << ctCategory;
  if (qc & eMyMoney::Report::QueryColumn::Account)
    m_columns << ctAccount;
  if (qc & eMyMoney::Report::QueryColumn::Reconciled)
    m_columns << ctReconcileFlag;
  if (qc & eMyMoney::Report::QueryColumn::Memo)
    m_columns << ctMemo;
  if (qc & eMyMoney::Report::QueryColumn::Action)
    m_columns << ctAction;
  if (qc & eMyMoney::Report::QueryColumn::Shares)
    m_columns << ctShares;
  if (qc & eMyMoney::Report::QueryColumn::Price)
    m_columns << ctPrice;
  if (qc & eMyMoney::Report::QueryColumn::Performance) {
    m_subtotal.clear();
    switch (m_config.investmentSum()) {
      case eMyMoney::Report::InvestmentSum::OwnedAndSold:
        m_columns << ctBuys << ctSells << ctReinvestIncome << ctCashIncome
                  << ctEndingBalance << ctReturn << ctReturnInvestment;
        m_subtotal << ctBuys << ctSells << ctReinvestIncome << ctCashIncome
                   << ctEndingBalance << ctReturn << ctReturnInvestment;
        break;
      case eMyMoney::Report::InvestmentSum::Owned:
        m_columns << ctBuys << ctReinvestIncome << ctMarketValue
                  << ctReturn << ctReturnInvestment;
        m_subtotal << ctBuys << ctReinvestIncome << ctMarketValue
                   << ctReturn << ctReturnInvestment;
        break;
      case eMyMoney::Report::InvestmentSum::Sold:
        m_columns << ctBuys << ctSells << ctCashIncome
                  << ctReturn << ctReturnInvestment;
        m_subtotal << ctBuys << ctSells << ctCashIncome
                   << ctReturn << ctReturnInvestment;
        break;
      case eMyMoney::Report::InvestmentSum::Period:
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
  if (qc & eMyMoney::Report::QueryColumn::CapitalGain) {
    m_subtotal.clear();
    switch (m_config.investmentSum()) {
      case eMyMoney::Report::InvestmentSum::Owned:
        m_columns << ctShares << ctBuyPrice << ctLastPrice
                  << ctBuys << ctMarketValue << ctPercentageGain
                  << ctCapitalGain;
        m_subtotal << ctShares << ctBuyPrice << ctLastPrice
                   << ctBuys << ctMarketValue << ctPercentageGain
                   << ctCapitalGain;
        break;
      case eMyMoney::Report::InvestmentSum::Sold:
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
  if (qc & eMyMoney::Report::QueryColumn::Loan) {
    m_columns << ctPayment << ctInterest << ctFees;
    m_postcolumns << ctBalance;
  }
  if (qc & eMyMoney::Report::QueryColumn::Balance)
    m_postcolumns << ctBalance;

  TableRow::setSortCriteria(sort);
  std::sort(m_rows.begin(), m_rows.end());
  if (m_config.isShowingColumnTotals())
    constructTotalRows(); // adds total rows to m_rows
}

void QueryTable::constructTotalRows()
{
  if (m_rows.isEmpty())
    return;

  // qSort places grand total at first positions, because it doesn't belong to any group
  // subtotals are placed in front of the topAccount rows
  const auto rows = m_rows.count();
  for (int i = 0; i < rows-1; ++i) {
    // it should be unlikely that total row is at the top of rows, so...
    if ((m_rows.at(i)[ctRank] == QLatin1String("5")) || (m_rows.at(i)[ctTopAccount].isEmpty())) {
      // check if there are other entries than totals so moving makes sense
      for (int j = i+1; j <= rows-1; ++j) {
        if ((m_rows.at(j)[ctRank] != QLatin1String("5")) && (!m_rows.at(j)[ctTopAccount].isEmpty())) {
          m_rows.move(i, rows - 1);                   // ...move it at the end
          --i;                                        // check the same slot again
          break;
        }
      }
    } else if (m_rows.at(i)[ctRank] == QLatin1String("4")) {
      // search last entry of same topAccount
      auto last = i+1;
      while ((m_rows.at(i)[ctTopAccount] == m_rows.at(last)[ctTopAccount]) && (last < (rows - 1))) {
        ++last;
      }
      // move subtotal to last entry
      m_rows.move(i, last - 1);                       // ...move to end of entries
      i = last-1;
    }
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

    auto levelToClose = groups.count();
    if (!lastRow) {
      for (int i = 0; i < groups.count(); ++i) {
        if (m_rows.at(iCurrentRow)[groups.at(i)] != m_rows.at(iNextRow)[groups.at(i)]) {
          levelToClose = i;
          break;
        }
      }
    } else {
      levelToClose = 0;   // all, we're done
    }
    // iterate over groups from the lowest to the highest to close groups
    for (int i = groups.count() - 1; i >= levelToClose ; --i) {
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
                                              (*currencyGrp).at(i + 1).value(ctCashIncome));
            else if (subtotal == ctPercentageGain) {
              const MyMoneyMoney denominator = (*currencyGrp).at(i + 1).value(ctBuys).abs();
              totalsRow[subtotal] = denominator.isZero() ? QString():
                  (((*currencyGrp).at(i + 1).value(ctBuys) + (*currencyGrp).at(i + 1).value(ctMarketValue)) / denominator).toString();
            } else if (subtotal == ctPrice)
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

          currencyID = currencyGrp.key();
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
                                            (*currencyGrp).at(0).value(ctCashIncome));
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

        currencyID = currencyGrp.key();
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
        if (!m_containsNonBaseCurrency && totalsRow[ctCurrency] != file->baseCurrency().id()) {
          m_containsNonBaseCurrency = true;
        }
        ++currencyGrp;
      }
      break;                                      // no use to loop further
    }
    iCurrentRow = iNextRow;                       // iCurrent makes here a leap forward by at least one
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
    case eMyMoney::Report::RowType::Category:
    case eMyMoney::Report::RowType::TopCategory:
      use_summary = false;
      use_transfers = report.isIncludingTransfers();
      report.setTreatTransfersAsIncomeExpense(use_transfers);
      hide_details = false;
      break;
    case eMyMoney::Report::RowType::Payee:
      use_summary = false;
      use_transfers = report.isIncludingTransfers();
      report.setTreatTransfersAsIncomeExpense(use_transfers);
      hide_details = (m_config.detailLevel() == eMyMoney::Report::DetailLevel::None);
      break;
    case eMyMoney::Report::RowType::Tag:
      use_summary = false;
      use_transfers = report.isIncludingTransfers();
      report.setTreatTransfersAsIncomeExpense(use_transfers);
      hide_details = (m_config.detailLevel() == eMyMoney::Report::DetailLevel::None);
      tag_special_case = true;
      break;
    default:
      use_summary = true;
      use_transfers = true;
      hide_details = (m_config.detailLevel() == eMyMoney::Report::DetailLevel::None);
      break;
  }

  // support for opening and closing balances
  QMap<QString, MyMoneyAccount> accts;

  //get all transactions for this report
  QList<MyMoneyTransaction> transactions = file->transactionList(report);
  for (QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.constBegin(); it_transaction != transactions.constEnd(); ++it_transaction) {

    TableRow qA, qS;
    QList<TableRow> qStack;

    QDate pd;
    QList<QString> tagIdListCache;

    qA[ctID] = qS[ctID] = (* it_transaction).id();
    qA[ctEntryDate] = qS[ctEntryDate] = (* it_transaction).entryDate().toString(Qt::ISODate);
    qA[ctPostDate] = qS[ctPostDate] = (* it_transaction).postDate().toString(Qt::ISODate);
    qA[ctCommodity] = qS[ctCommodity] = (* it_transaction).commodity();

    pd = (* it_transaction).postDate();
    qA[ctMonth] = qS[ctMonth] = i18n("Month of %1", QDate(pd.year(), pd.month(), 1).toString(Qt::ISODate));
    qA[ctWeek] = qS[ctWeek] = i18n("Week of %1", pd.addDays(1 - pd.dayOfWeek()).toString(Qt::ISODate));

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
      ReportAccount splitAcc((* it_split).accountId());
      // always put split with a "stock" account if it exists
      if (splitAcc.isInvest())
        break;

      // prefer to put splits with a "loan" account if it exists
      if (splitAcc.isLoan())
        myBegin = it_split;

      if ((myBegin == splits.end()) && ! splitAcc.isIncomeExpense()) {
        // continue if split references an unselected account
        if (report.includesAccount(splitAcc.id())) {
          myBegin = it_split;
        }
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
    if (m_config.queryColumns() & eMyMoney::Report::QueryColumn::Loan) {
      ReportAccount splitAcc((*it_split).accountId());
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
      ReportAccount splitAcc((* it_split).accountId());
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

      qA[ctTag].clear();

      if (it_split == myBegin && splits.count() > 1) {
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

          if (((*it_split).action() == MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares)) && shares.isNegative())
            qA[ctAction] = "Sell";

          qA[ctInvestAccount] = splitAcc.parent().name();

          MyMoneySplit stockSplit = (*it_split);
          MyMoneySplit assetAccountSplit;
          QList<MyMoneySplit> feeSplits;
          QList<MyMoneySplit> interestSplits;
          MyMoneySecurity currency;
          MyMoneySecurity security;
          eMyMoney::Split::InvestmentTransactionType transactionType;
          MyMoneyUtils::dissectTransaction((*it_transaction), stockSplit, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
          if (!(assetAccountSplit == MyMoneySplit())) {
            for (it_split = splits.begin(); it_split != splits.end(); ++it_split) {
              if ((*it_split) == assetAccountSplit) {
                splitAcc = ReportAccount(assetAccountSplit.accountId()); // switch over from stock split to asset split because amount in stock split doesn't take fees/interests into account
                myBegin = it_split;                       // set myBegin to asset split, so stock split can be listed in details under splits
                myBeginCurrency = (file->account((*myBegin).accountId())).currencyId();
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

        transaction_text = m_config.match((*it_split));

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
              if (!m_containsNonBaseCurrency && qA[ctCurrency] != file->baseCurrency().id()) {
                m_containsNonBaseCurrency = true;
              }
            } else if (splits.count() > 2) {
              // this applies when the transaction has more than 2 splits
              // and each is shown seperately
              switch (m_config.rowType()) {
                case eMyMoney::Report::RowType::Category:
                case eMyMoney::Report::RowType::TopCategory:
                case eMyMoney::Report::RowType::Tag:
                case eMyMoney::Report::RowType::Payee:
                  if (splitAcc.isAssetLiability()) {
                    qA[ctValue] = ((*it_split).shares() * xr).convert(fraction).toString(); // needed for category reports, in case of multicurrency transaction it breaks it
                    // make sure we use the right currency of the category
                    // (will be ignored when converting to base currency)
                    qA[ctCurrency] = splitAcc.currencyId();
                  }
                  break;
                default:
                  break;
              }
              qA[ctSplit].clear();
              qA[ctRank] = QLatin1Char('1');
              // keep it for now and don't add the data immediately
              // as we may find a better match in one of the other splits
              qStack += qA;
            }
          }
        }

      } else {

        if (include_me) {

          if (loan_special_case) {
            MyMoneyMoney value = (-(* it_split).shares() * xr).convert(fraction);

            if ((*it_split).action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization)) {
              // put the payment in the "payment" column and convert to lowest fraction
              qA[ctPayee] = value.toString();
            } else if ((*it_split).action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
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
            // qA after all of the split components have been processed.
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
              qA[ctTag] = "";
              QString delimiter = "";
              for (int i = 0; i < tagIdList.size(); i++) {
                qA[ctTag] += delimiter + file->tag(tagIdList[i]).name().simplified();
                delimiter = ", ";
              }
            } else {
              //this applies when the transaction has only 2 splits, or each split is going to be
              //shown separately, eg. transactions by category
              switch (m_config.rowType()) {
                case eMyMoney::Report::RowType::Category:
                case eMyMoney::Report::RowType::TopCategory:
                case eMyMoney::Report::RowType::Tag:
                case eMyMoney::Report::RowType::Payee:
                  if (splitAcc.isIncomeExpense()) {
                    qA[ctValue] = (-(*it_split).shares() * xr).convert(fraction).toString(); // needed for category reports, in case of multicurrency transaction it breaks it
                    // make sure we use the right currency of the category
                    // (will be ignored when converting to base currency)
                    qA[ctCurrency] = splitAcc.currencyId();
                  }
                  break;
                default:
                  break;
              }
              qA[ctSplit].clear();
              qA[ctRank] = QLatin1Char('1');
            }

            qA [ctMemo] = (*it_split).memo();

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
              qA [ctCategoryType] = MyMoneyAccount::accountTypeToString(splitAcc.accountGroup());
            }

            if (splits.count() > 1) {
              if (use_transfers || (splitAcc.isIncomeExpense() && m_config.includes(splitAcc))) {
                //if it matches the text of the main split of the transaction or
                //it matches this particular split, include it
                //otherwise, skip it
                //if the filter is "does not contain" exclude the split if it does not match
                //even it matches the whole split
                if ((m_config.isInvertingText() &&
                    m_config.match((*it_split)))
                    || (!m_config.isInvertingText()
                        && (transaction_text
                            || m_config.match((*it_split))))) {
                  if (tag_special_case) {
                    if (tagIdListCache.isEmpty()) {
                      qA[ctTag] = i18n("[No Tag]");
                    } else {
                      QString delimiter;
                      foreach(const auto tagId, tagIdListCache) {
                        qA[ctTag] += delimiter + file->tag(tagId).name().simplified();
                        delimiter = QLatin1Char(',');
                      }
                    }
                  }
                  m_rows += qA;
                  if (!m_containsNonBaseCurrency && qA[ctCurrency] != file->baseCurrency().id()) {
                    m_containsNonBaseCurrency = true;
                  }

                  // we don't need the stacked data
                  qStack.clear();
                }
              }
            }
          }
        }

        if ((m_config.includes(splitAcc) && use_transfers &&
            !(splitAcc.isInvest() && include_me)) || splits.count() == 1) { // otherwise stock split is displayed twice in report
          if (! splitAcc.isIncomeExpense()) {
            //multiply by currency and convert to lowest fraction
            qS[ctValue] = ((*it_split).shares() * xr).convert(fraction).toString();

            qS[ctRank] = QLatin1Char('1');

            qS[ctAccount] = splitAcc.name();
            qS[ctAccountID] = splitAcc.id();
            qS[ctTopAccount] = splitAcc.topParentName();

            if (splits.count() > 1) {
              qS[ctCategory] = ((*it_split).shares().isNegative())
                              ? i18n("Transfer to %1", a_fullname)
                              : i18n("Transfer from %1", a_fullname);
            } else {
              qS[ctCategory] = i18n("*** UNASSIGNED ***");
            }
            qS[ctInstitution] = institution.isEmpty()
                                ? i18n("No Institution")
                                : file->institution(institution).name();

            qS[ctMemo] = (*it_split).memo().isEmpty()
                         ? a_memo
                         : (*it_split).memo();

            //FIXME-ALEX When is used this? I can't find in which condition we arrive here... maybe this code is useless?
            if (tagIdList.isEmpty()) {
              qS[ctTag] = i18n("[No Tag]");
            } else {
              QString delimiter;
              foreach(const auto tagId, tagIdList) {
                qS[ctTag] += delimiter + file->tag(tagId).name().simplified();
                delimiter = QLatin1Char(',');
              }
            }

            qS[ctPayee] = payee.isEmpty()
                          ? qA[ctPayee]
                          : file->payee(payee).name().simplified();

            //check the specific split against the filter for text and amount
            //TODO this should be done at the engine, but I have no clear idea how -- asoliverez
            //if the filter is "does not contain" exclude the split if it does not match
            //even it matches the whole split
            if ((m_config.isInvertingText() &&
                 m_config.match((*it_split)))
                || (!m_config.isInvertingText()
                    && (transaction_text
                        || m_config.match((*it_split))))) {
              m_rows += qS;
              qStack.clear();
              if (!m_containsNonBaseCurrency && qS[ctCurrency] != file->baseCurrency().id()) {
                m_containsNonBaseCurrency = true;
              }

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
      if (!m_containsNonBaseCurrency && qA[ctCurrency] != file->baseCurrency().id()) {
        m_containsNonBaseCurrency = true;
      }
      qStack.clear();
    }
    // check if the stack contains a foreign currency
    for (const auto& row : qAsConst(qStack)) {
      if (!m_containsNonBaseCurrency && row[ctCurrency] != file->baseCurrency().id()) {
        m_containsNonBaseCurrency = true;
        break;
      }
    }
    m_rows += qStack;
  }

  // now run through our accts list and add opening and closing balances

  switch (m_config.rowType()) {
    case eMyMoney::Report::RowType::Account:
    case eMyMoney::Report::RowType::TopAccount:
      break;

      // case eMyMoney::Report::RowType::Category:
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

  for (auto it_account = accts.constBegin(); it_account != accts.constEnd(); ++it_account) {
    TableRow qA;

    ReportAccount account(*it_account);

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
    if (!m_containsNonBaseCurrency && qA[ctCurrency] != file->baseCurrency().id()) {
      m_containsNonBaseCurrency = true;
    }
  }
}

QString QueryTable::helperROI(const MyMoneyMoney &buys, const MyMoneyMoney &sells, const MyMoneyMoney &startingBal, const MyMoneyMoney &endingBal, const MyMoneyMoney &cashIncome) const
{
  MyMoneyMoney returnInvestment;
  if (!(startingBal - buys).isZero()) {
    returnInvestment = (sells + buys + cashIncome + endingBal - startingBal) / (startingBal - buys);
    return returnInvestment.convert(10000).toString();
  } else
    return QString();
}

QString QueryTable::helperIRR(const CashFlowList &all) const
{
  try {
    return MyMoneyMoney(all.XIRR(), 10000).toString();
  } catch (MyMoneyException &e) {
    qDebug() << e.what();
    all.dumpDebug();
    return QString();
  }
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

  if (report.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain) {
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
      eMyMoney::Split::InvestmentTransactionType transactionType;
      MyMoneyUtils::dissectTransaction((*it_t), shareSplit, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
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

      if (transactionType == eMyMoney::Split::InvestmentTransactionType::BuyShares) {
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
        } else if (shList.at(BuysOfOwned) >= shares) {              // subtract not-sold shares
          shList[BuysOfOwned] -= shares;
          cfList[BuysOfOwned].append(CashFlowListItem(postDate, value));
        } else {                                                    // subtract partially not-sold shares
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
      } else if (transactionType == eMyMoney::Split::InvestmentTransactionType::SellShares && reportedDateRange) {
        cfList[Sells].append(CashFlowListItem(postDate, value));
        shList[Sells] += shares;
      } else if (transactionType == eMyMoney::Split::InvestmentTransactionType::SplitShares) {          // shares variable is denominator of split ratio here
        for (int i = Buys; i <= InvestmentValue::BuysOfOwned; ++i)
          shList[i] /= shares;
      } else if (transactionType == eMyMoney::Split::InvestmentTransactionType::AddShares ||            // added shares, when sold give 100% capital gain
                 transactionType == eMyMoney::Split::InvestmentTransactionType::ReinvestDividend) {
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
        } else if (shList.at(BuysOfOwned) >= shares) {                    // subtract not-added/not-reinvested shares
          shList[BuysOfOwned] -= shares;
          cfList[BuysOfOwned].append(CashFlowListItem(postDate, value));
        } else {                                                          // subtract partially not-added/not-reinvested shares
          MyMoneyMoney tempVal = (shares - shList.at(BuysOfOwned));
          shList[BuysOfSells] += tempVal;
          if (postDate < termSeparator)
            shList[LongTermBuysOfSells] += tempVal;

          cfList[BuysOfOwned].append(CashFlowListItem(postDate, (shList.at(BuysOfOwned) / shares) * value));
          shList[BuysOfOwned] = MyMoneyMoney();
        }
        if (transactionType == eMyMoney::Split::InvestmentTransactionType::ReinvestDividend) {
          value = MyMoneyMoney();
          foreach (const auto split, interestSplits)
            value += split.value();
          value *= price;
          cfList[ReinvestIncome].append(CashFlowListItem(postDate, -value));
        }
      } else if (transactionType == eMyMoney::Split::InvestmentTransactionType::RemoveShares && reportedDateRange) // removed shares give no value in return so no capital gain on them
        shList[Sells] += shares;
      else if (transactionType == eMyMoney::Split::InvestmentTransactionType::Dividend || transactionType == eMyMoney::Split::InvestmentTransactionType::Yield)
        cfList[CashIncome].append(CashFlowListItem(postDate, value));

    }
    reportedDateRange = false;
    newEndingDate = newStartingDate;
    newStartingDate = newStartingDate.addYears(-1);
    report.setDateFilter(newStartingDate, newEndingDate); // search for matching buy transactions year earlier

  } while (
           (
             (report.investmentSum() == eMyMoney::Report::InvestmentSum::Owned && !shList[BuysOfOwned].isZero()) ||
             (report.investmentSum() == eMyMoney::Report::InvestmentSum::Sold && !shList.at(Sells).isZero() && shList.at(Sells).abs() > shList.at(BuysOfSells).abs()) ||
             (report.investmentSum() == eMyMoney::Report::InvestmentSum::OwnedAndSold && (!shList[BuysOfOwned].isZero() || (!shList.at(Sells).isZero() && shList.at(Sells).abs() > shList.at(BuysOfSells).abs())))
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
      eMyMoney::Split::InvestmentTransactionType transactionType;
      MyMoneyUtils::dissectTransaction(transaction, shareSplit, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
      QDate postDate = transaction.postDate();
      MyMoneyMoney price;
      if (m_config.isConvertCurrency())
        price = account.baseCurrencyPrice(postDate); //we only need base currency because the value is in deep currency
      else
        price = MyMoneyMoney::ONE;
      MyMoneyMoney value = assetAccountSplit.value() * price;
      MyMoneyMoney shares = shareSplit.shares();

      if (transactionType == eMyMoney::Split::InvestmentTransactionType::SellShares) {
        if ((shList.at(LongTermSellsOfBuys) + shares).abs() >= shList.at(LongTermBuysOfSells)) { // add partially sold long-term shares
          cfList[LongTermSellsOfBuys].append(CashFlowListItem(postDate, (shList.at(LongTermSellsOfBuys).abs() - shList.at(LongTermBuysOfSells)) / shares * value));
          shList[LongTermSellsOfBuys] = shList.at(LongTermBuysOfSells);
          break;
        } else {                      // add wholly sold long-term shares
          cfList[LongTermSellsOfBuys].append(CashFlowListItem(postDate, value));
          shList[LongTermSellsOfBuys] += shares;
        }
      } else if (transactionType == eMyMoney::Split::InvestmentTransactionType::RemoveShares) {
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
  case eMyMoney::Report::InvestmentSum::OwnedAndSold:
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
  case eMyMoney::Report::InvestmentSum::Owned:
    buysTotal = cfList.at(BuysOfOwned).total();
    startingBal = MyMoneyMoney();
    if (buysTotal.isZero() && endingBal.isZero())
      return;
    all.append(cfList.at(BuysOfOwned));
    all.append(CashFlowListItem(endingDate, endingBal));

    result[ctReinvestIncome] = reinvestIncomeTotal.toString();
    result[ctMarketValue] = endingBal.toString();
    break;
  case eMyMoney::Report::InvestmentSum::Sold:
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
  case eMyMoney::Report::InvestmentSum::Period:
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

  result[ctBuys] = buysTotal.toString();
  result[ctReturn] = helperIRR(all);
  result[ctReturnInvestment] = helperROI(buysTotal - reinvestIncomeTotal, sellsTotal, startingBal, endingBal, cashIncomeTotal);
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
  case eMyMoney::Report::InvestmentSum::Owned:
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
    result[ctPercentageGain] = buysTotal.isZero() ? QString() :
        ((buysTotal + endingBal)/buysTotal.abs()).toString();
    break;
  }
  case eMyMoney::Report::InvestmentSum::Sold:
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
    if ((*it_account).isAssetLiability() && m_config.includes((*it_account)) && (*it_account).accountType() != eMyMoney::Account::Type::Investment) {
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
        case eMyMoney::Report::QueryColumn::Performance:
        {
          constructPerformanceRow(account, qaccountrow, accountCashflow);
          if (!qaccountrow.isEmpty()) {
            // assuming that that report is grouped by topaccount
            qaccountrow[ctTopAccount] = account.topParentName();
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
        case eMyMoney::Report::QueryColumn::CapitalGain:
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

          const auto iid = account.institutionId();
          if (iid.isEmpty())
            qaccountrow[ctInstitution] = i18nc("No institution", "None");
          else
            qaccountrow[ctInstitution] = file->institution(iid).name();

          qaccountrow[ctType] = MyMoneyAccount::accountTypeToString(account.accountType());
        }
      }

      if (qaccountrow.isEmpty()) // don't add the account if there are no calculated values
        continue;

      qaccountrow[ctRank] = QLatin1Char('1');
      qaccountrow[ctAccount] = account.name();
      qaccountrow[ctAccountID] = account.id();
      qaccountrow[ctTopAccount] = account.topParentName();
      if (m_config.isConvertCurrency())
        qaccountrow[ctCurrency] = file->baseCurrency().id();
      else
        qaccountrow[ctCurrency] = account.currency().id();
      m_rows.append(qaccountrow);
      if (!m_containsNonBaseCurrency && qaccountrow[ctCurrency] != file->baseCurrency().id()) {
        m_containsNonBaseCurrency = true;
      }
    }
  }

  if (m_config.queryColumns() == eMyMoney::Report::QueryColumn::Performance && m_config.isShowingColumnTotals()) {
    TableRow qtotalsrow;
    qtotalsrow[ctRank] = QLatin1Char('4'); // add identification of row as total
    QMap<QString, CashFlowList> currencyGrandCashFlow;

    QMap<QString, QMap<QString, CashFlowList>>::iterator currencyAccGrp = currencyCashFlow.begin();
    while (currencyAccGrp != currencyCashFlow.end()) {
      // convert map of top accounts with cashflows to TableRow
      for (QMap<QString, CashFlowList>::iterator topAccount = (*currencyAccGrp).begin(); topAccount != (*currencyAccGrp).end(); ++topAccount) {
        qtotalsrow[ctTopAccount] = topAccount.key();
        qtotalsrow[ctReturn] = helperIRR(topAccount.value());
        qtotalsrow[ctCurrency] = currencyAccGrp.key();
        currencyGrandCashFlow[currencyAccGrp.key()] += topAccount.value();  // cumulative sum of cashflows of each topaccount
        m_rows.append(qtotalsrow);            // rows aren't sorted yet, so no problem with adding them randomly at the end
        if (!m_containsNonBaseCurrency && qtotalsrow[ctCurrency] != file->baseCurrency().id()) {
          m_containsNonBaseCurrency = true;
        }
      }
      ++currencyAccGrp;
    }
    QMap<QString, CashFlowList>::iterator currencyGrp = currencyGrandCashFlow.begin();
    qtotalsrow[ctTopAccount].clear();          // empty topaccount because it's grand cashflow
    while (currencyGrp != currencyGrandCashFlow.end()) {
      qtotalsrow[ctReturn] = helperIRR(currencyGrp.value());
      qtotalsrow[ctCurrency] = currencyGrp.key();
      m_rows.append(qtotalsrow);
      if (!m_containsNonBaseCurrency && qtotalsrow[ctCurrency] != file->baseCurrency().id()) {
        m_containsNonBaseCurrency = true;
      }
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
      ReportAccount splitAcc((* it_split).accountId());
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
    if (m_config.queryColumns() & eMyMoney::Report::QueryColumn::Loan) {
      ReportAccount splitAcc((*it_split).accountId());
      loan_special_case = splitAcc.isLoan();
    }

    // There is a slight chance that at this point myBegin is still pointing to splits.end() if the
    // transaction only has income and expense splits (which should not happen). In that case, point
    // it to the first split
    if (myBegin == splits.end()) {
      myBegin = splits.begin();
    }

    //the account of the beginning splits
    ReportAccount myBeginAcc((*myBegin).accountId());

    bool include_me = true;
    QString a_fullname;
    QString a_memo;
    int pass = 1;

    do {
      MyMoneyMoney xr;
      ReportAccount splitAcc((* it_split).accountId());

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

        if (((*it_split).action() == MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares)) && (*it_split).shares().isNegative())
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
            ReportAccount tempSplitAcc((*tempSplit).accountId());
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
        qA [ctCategoryType] = MyMoneyAccount::accountTypeToString(splitAcc.accountGroup());

        m_rows += qA;
        if (!m_containsNonBaseCurrency && qA[ctCurrency] != file->baseCurrency().id()) {
          m_containsNonBaseCurrency = true;
        }

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
      if (!m_containsNonBaseCurrency && qA[ctCurrency] != file->baseCurrency().id()) {
        m_containsNonBaseCurrency = true;
      }
    }
  }

  // now run through our accts list and add opening and closing balances

  switch (m_config.rowType()) {
    case eMyMoney::Report::RowType::Account:
    case eMyMoney::Report::RowType::TopAccount:
      break;

      // case eMyMoney::Report::RowType::Category:
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

  for (auto it_account = accts.constBegin(); it_account != accts.constEnd(); ++it_account) {
    TableRow qA;

    ReportAccount account((* it_account));

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
    if (!m_containsNonBaseCurrency && qA[ctCurrency] != file->baseCurrency().id()) {
      m_containsNonBaseCurrency = true;
    }
  }
}



}
