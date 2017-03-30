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
#include <QFile>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneytransaction.h"
#include "mymoneyreport.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "reportaccount.h"
#include "reportdebug.h"
#include "kmymoneyglobalsettings.h"

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

  const_iterator it_cash = begin();
  while (it_cash != end()) {
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

  QList<CashFlowListItem>::const_iterator list_it = begin();
  while (list_it != end()) {
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

  QList<CashFlowListItem>::const_iterator list_it = begin();
  while (list_it != end()) {
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

  const_iterator it_cash = begin();
  while (it_cash != end()) {
    result += (*it_cash).value();
    ++it_cash;
  }

  return result;
}

void CashFlowList::dumpDebug() const
{
  const_iterator it_item = begin();
  while (it_item != end()) {
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
  switch (m_config.rowType()) {
    case MyMoneyReport::eAccountByTopAccount:
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
      constructAccountTable();
      m_columns = "account";
      break;

    case MyMoneyReport::eAccount:
      constructTransactionTable();
      m_columns = "accountid,postdate";
      break;

    case MyMoneyReport::ePayee:
    case MyMoneyReport::eTag:
    case MyMoneyReport::eMonth:
    case MyMoneyReport::eWeek:
      constructTransactionTable();
      m_columns = "postdate,account";
      break;
    case MyMoneyReport::eCashFlow:
      constructSplitsTable();
      m_columns = "postdate";
      break;
    default:
      constructTransactionTable();
      m_columns = "postdate";
  }

  // Sort the data to match the report definition
  m_subtotal = "value";

  switch (m_config.rowType()) {
    case MyMoneyReport::eCashFlow:
      m_group = "categorytype,topcategory,category";
      break;
    case MyMoneyReport::eCategory:
      m_group = "categorytype,topcategory,category";
      break;
    case MyMoneyReport::eTopCategory:
      m_group = "categorytype,topcategory";
      break;
    case MyMoneyReport::eTopAccount:
      m_group = "topaccount,account";
      break;
    case MyMoneyReport::eAccount:
      m_group = "account";
      break;
    case MyMoneyReport::eAccountReconcile:
      m_group = "account,reconcileflag";
      break;
    case MyMoneyReport::ePayee:
      m_group = "payee";
      break;
    case MyMoneyReport::eTag:
      m_group = "tag";
      break;
    case MyMoneyReport::eMonth:
      m_group = "month";
      break;
    case MyMoneyReport::eWeek:
      m_group = "week";
      break;
    case MyMoneyReport::eAccountByTopAccount:
      m_group = "topaccount";
      break;
    case MyMoneyReport::eEquityType:
      m_group = "equitytype";
      break;
    case MyMoneyReport::eAccountType:
      m_group = "type";
      break;
    case MyMoneyReport::eInstitution:
      m_group = "institution,topaccount";
      break;
    default:
      throw MYMONEYEXCEPTION("QueryTable::QueryTable(): unhandled row type");
  }

  QString sort = m_group + ",id,rank," + m_columns;

  switch (m_config.rowType()) {
    case MyMoneyReport::eAccountByTopAccount:
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
      m_columns = "account";
      break;

    default:
      m_columns = "postdate";
  }

  unsigned qc = m_config.queryColumns();

  if (qc & MyMoneyReport::eQCnumber)
    m_columns += ",number";
  if (qc & MyMoneyReport::eQCpayee)
    m_columns += ",payee";
  if (qc & MyMoneyReport::eQCtag)
    m_columns += ",tag";
  if (qc & MyMoneyReport::eQCcategory)
    m_columns += ",category";
  if (qc & MyMoneyReport::eQCaccount)
    m_columns += ",account";
  if (qc & MyMoneyReport::eQCreconciled)
    m_columns += ",reconcileflag";
  if (qc & MyMoneyReport::eQCmemo)
    m_columns += ",memo";
  if (qc & MyMoneyReport::eQCaction)
    m_columns += ",action";
  if (qc & MyMoneyReport::eQCshares)
    m_columns += ",shares";
  if (qc & MyMoneyReport::eQCprice)
    m_columns += ",price";
  if (qc & MyMoneyReport::eQCperformance) {
    m_columns += ",startingbal,buys,sells,reinvestincome,cashincome,return,returninvestment,endingbal";
    m_subtotal = "startingbal,buys,sells,reinvestincome,cashincome,return,returninvestment,endingbal";
  }
  if (qc & MyMoneyReport::eQCcapitalgain) {
    m_columns += ",buys,sells,capitalgain";
    m_subtotal = "buys,sells,capitalgain";
  }
  if (qc & MyMoneyReport::eQCloan) {
    m_columns += ",payment,interest,fees";
    m_postcolumns = "balance";
  }
  if (qc & MyMoneyReport::eQCbalance)
    m_postcolumns = "balance";

  TableRow::setSortCriteria(sort);
  qSort(m_rows);

  constructTotalRows(); // adds total rows to m_rows
}

void QueryTable::constructTotalRows()
{
  if (m_rows.isEmpty())
    return;

  // qSort places grand total at last position, because it doesn't belong to any group
  if (m_rows.at(0)["rank"] == "4")      // it should be unlikely that total row is at the top of rows, so...
    m_rows.move(0, m_rows.count() - 1); // ...move it at the bottom

  QStringList subtotals = m_subtotal.split(',');
  QStringList groups = m_group.split(',');
  QStringList columns = m_columns.split(',');
  if (!m_subtotal.isEmpty() && subtotals.count() == 1)
    columns += m_subtotal;
  QStringList postcolumns = m_postcolumns.split(',');
  if (!m_postcolumns.isEmpty())
    columns += postcolumns;

  QList<QMap<QString, MyMoneyMoney>> totalGroups;
  QMap<QString, MyMoneyMoney> totalsValues;

  // initialize all total values under summed columns to be zero
  foreach (auto subtotal, subtotals) {
    totalsValues.insert(subtotal, MyMoneyMoney());
  }

  // create total groups containing totals row for each group
  totalGroups.append(totalsValues);  // prepend with extra group for grand total
  for (int j = 0; j < groups.count(); ++j) {
    totalGroups.append(totalsValues);
  }

  QList<TableRow> stashedTotalRows;
  int iCurrent, iNext;
  for (iCurrent = 0; iCurrent < m_rows.count();) {
    iNext = iCurrent + 1;

    // total rows are useless at summing so remove whole block of them at once
    while (iNext != m_rows.count() && m_rows.at(iNext)["rank"] == "4") {
      stashedTotalRows.append(m_rows.takeAt(iNext)); // ...but stash them just in case
    }

    bool lastRow = (iNext == m_rows.count());

    // sum all subtotal values for lowest group
    foreach (auto subtotal, subtotals) {
      totalGroups.last()[subtotal] += MyMoneyMoney(m_rows.at(iCurrent)[subtotal]);
    }

    // iterate over groups from the lowest to the highest to find group change
    for (int i = groups.count() - 1; i >= 0 ; --i) {
      // if any of groups from next row changes (or next row is the last row), then it's time to put totals row
      if (lastRow || m_rows.at(iCurrent)[groups.at(i)] != m_rows.at(iNext)[groups.at(i)]) {
        TableRow totalsRow;
        // custom total values calculations
        foreach (auto subtotal, subtotals) {
          if (subtotal == "returninvestment")
            totalGroups[i + 1]["returninvestment"] = helperROI(totalGroups[i + 1]["buys"], totalGroups[i + 1]["sells"],
                                                          totalGroups[i + 1]["startingbal"], totalGroups[i + 1]["endingbal"],
                                                          totalGroups[i + 1]["cashincome"]);
        }

        // total values that aren't calculated here, but are taken untouched from external source, e.g. constructPerformanceRow
        if (!stashedTotalRows.isEmpty()) {
          foreach (auto subtotal, subtotals) {
            if (subtotal == "return")
              totalsRow["return"] = stashedTotalRows.first()["return"];
          }
          stashedTotalRows.removeFirst();
        }

        // sum all subtotal values for higher groups (excluding grand total) and reset lowest group values
        QMap<QString, MyMoneyMoney>::iterator upperGrp = totalGroups[i].begin();
        QMap<QString, MyMoneyMoney>::iterator lowerGrp = totalGroups[i + 1].begin();

        while(upperGrp != totalGroups[i].end()) {
          totalsRow[lowerGrp.key()] = lowerGrp.value().toString();  // fill totals row with subtotal values...
          (*upperGrp) += (*lowerGrp);
          (*lowerGrp) = MyMoneyMoney();
          ++upperGrp;
          ++lowerGrp;
        }

        for (int j = 0; j < groups.count(); ++j) {
          totalsRow[groups.at(j)] = m_rows.at(iCurrent)[groups.at(j)];   // ...and identification
        }

        totalsRow["rank"] = "4";
        totalsRow["depth"] = QString::number(i);

        m_rows.insert(iNext++, totalsRow); // iCurrent and iNext can diverge here by more than one
      }
    }

    // code to put grand total row
    if (lastRow) {
      TableRow totalsRow;

      foreach (auto subtotal, subtotals) {
        if (subtotal == "returninvestment")
          totalGroups[0]["returninvestment"] = helperROI(totalGroups[0]["buys"], totalGroups[0]["sells"],
                                                        totalGroups[0]["startingbal"], totalGroups[0]["endingbal"],
                                                        totalGroups[0]["cashincome"]);
      }

      if (!stashedTotalRows.isEmpty()) {
        foreach (auto subtotal, subtotals) {
          if (subtotal == "return")
            totalsRow["return"] = stashedTotalRows.first()["return"];
        }
        stashedTotalRows.removeFirst();
      }

      QMap<QString, MyMoneyMoney>::const_iterator grandTotalGrp = totalGroups[0].begin();
      while(grandTotalGrp != totalGroups[0].end()) {
        totalsRow[grandTotalGrp.key()] = grandTotalGrp.value().toString();
        ++grandTotalGrp;
      }

      for (int j = 0; j < groups.count(); ++j) {
        totalsRow[groups.at(j)] = QString();      // no identification
      }

      totalsRow["rank"] = "4";
      totalsRow["depth"] = "";
      m_rows.append(totalsRow);
      break;                                      // no use to loop further
    }
    iCurrent = iNext;                             // iCurrent makes here a leap forward by at least one
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

    qA["id"] = qS["id"] = (* it_transaction).id();
    qA["entrydate"] = qS["entrydate"] = (* it_transaction).entryDate().toString(Qt::ISODate);
    qA["postdate"] = qS["postdate"] = (* it_transaction).postDate().toString(Qt::ISODate);
    qA["commodity"] = qS["commodity"] = (* it_transaction).commodity();

    pd = (* it_transaction).postDate();
    qA["month"] = qS["month"] = i18n("Month of %1", QDate(pd.year(), pd.month(), 1).toString(Qt::ISODate));
    qA["week"] = qS["week"] = i18n("Week of %1", pd.addDays(1 - pd.dayOfWeek()).toString(Qt::ISODate));

    qA["currency"] = qS["currency"] = "";

    if ((* it_transaction).commodity() != file->baseCurrency().id()) {
      if (!report.isConvertCurrency()) {
        qA["currency"] = qS["currency"] = (*it_transaction).commodity();
      }
    }

    // to handle splits, we decide on which account to base the split
    // (a reference point or point of view so to speak). here we take the
    // first account that is a stock account or loan account (or the first account
    // that is not an income or expense account if there is no stock or loan account)
    // to be the account (qA) that will have the sub-item "split" entries. we add
    // one transaction entry (qS) for each subsequent entry in the split.

    const QList<MyMoneySplit>& splits = (*it_transaction).splits();
    QList<MyMoneySplit>::const_iterator myBegin, it_split;

    for (it_split = splits.begin(), myBegin = splits.end(); it_split != splits.end(); ++it_split) {
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
    QString a_fullname = "";
    QString a_memo = "";
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

        if (myBeginCurrency != baseCurrency) {                             // myBeginCurrency can differ from baseCurrency...
          MyMoneyPrice price = file->price(myBeginCurrency, baseCurrency,
                                           (*it_transaction).postDate());  // ...so check conversion rate...
          if (price.isValid())
            xr *= price.rate(baseCurrency);                                // ...and multiply it by current price...
          else
            qA["currency"] = qS["currency"] = myBeginCurrency;             // ...or set information about non-baseCurrency
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

        qA["account"] = splitAcc.name();
        qA["accountid"] = splitAcc.id();
        qA["topaccount"] = splitAcc.topParentName();

        if (splitAcc.isInvest()) {
          // use the institution of the parent for stock accounts
          institution = splitAcc.parent().institutionId();
          MyMoneyMoney shares = (*it_split).shares();

          qA["action"] = (*it_split).action();
          qA["shares"] = shares.isZero() ? "" : shares.toString();
          qA["price"] = shares.isZero() ? "" : xr.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();

          if (((*it_split).action() == MyMoneySplit::ActionBuyShares) && shares.isNegative())
            qA["action"] = "Sell";

          qA["investaccount"] = splitAcc.parent().name();

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
                if (m_config.isConvertCurrency()) {
                  if (myBeginCurrency != baseCurrency) {
                    MyMoneyPrice price = file->price(myBeginCurrency, baseCurrency, (*it_transaction).postDate());
                    if (price.isValid()) {
                      xr = price.rate(baseCurrency);
                      qA["currency"] = qS["currency"] = "";
                    } else
                      qA["currency"] = qS["currency"] = myBeginCurrency;
                  } else
                    xr = MyMoneyMoney::ONE;

                  qA["price"] = shares.isZero() ? "" : (stockSplit.price() * xr / (*it_split).price()).toString();
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
          qA["price"] = xr.toString();

        a_fullname = splitAcc.fullName();
        a_memo = (*it_split).memo();

        transaction_text = m_config.match(&(*it_split));

        qA["institution"] = institution.isEmpty()
                            ? i18n("No Institution")
                            : file->institution(institution).name();

        qA["payee"] = payee.isEmpty()
                      ? i18n("[Empty Payee]")
                      : file->payee(payee).name().simplified();

        if (tag_special_case) {
          tagIdListCache = tagIdList;
        } else {
          QString delimiter = "";
          for (int i = 0; i < tagIdList.size(); i++) {
            qA["tag"] += delimiter + file->tag(tagIdList[i]).name().simplified();
            delimiter = ", ";
          }
        }
        qA["reconciledate"] = (*it_split).reconcileDate().toString(Qt::ISODate);
        qA["reconcileflag"] = KMyMoneyUtils::reconcileStateToString((*it_split).reconcileFlag(), true);
        qA["number"] = (*it_split).number();

        qA["memo"] = a_memo;

        qA["value"] = ((*it_split).shares() * xr).convert(fraction).toString();

        qS["reconciledate"] = qA["reconciledate"];
        qS["reconcileflag"] = qA["reconcileflag"];
        qS["number"] = qA["number"];

        qS["topcategory"] = splitAcc.topParentName();
        qS["categorytype"] = i18n("Transfer");

        // only include the configured accounts
        if (include_me) {

          if (loan_special_case) {

            // put the principal amount in the "value" column and convert to lowest fraction
            qA["value"] = (-(*it_split).shares() * xr).convert(fraction).toString();

            qA["rank"] = '1';
            qA["split"] = "";

          } else {
            if ((splits.count() > 2) && use_summary) {

              // add the "summarized" split transaction
              // this is the sub-total of the split detail
              // convert to lowest fraction
              qA["rank"] = '1';
              qA["category"] = i18n("[Split Transaction]");
              qA["topcategory"] = i18nc("Split transaction", "Split");
              qA["categorytype"] = i18nc("Split transaction", "Split");

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
              qA["payment"] = value.toString();
            } else if ((*it_split).action() == MyMoneySplit::ActionInterest) {
              // put the interest in the "interest" column and convert to lowest fraction
              qA["interest"] = value.toString();
            } else if (splits.count() > 2) {
              // [dv: This comment carried from the original code. I am
              // not exactly clear on what it means or why we do this.]
              // Put the initial pay-in nowhere (that is, ignore it). This
              // is dangerous, though. The only way I can tell the initial
              // pay-in apart from fees is if there are only 2 splits in
              // the transaction.  I wish there was a better way.
            } else {
              // accumulate everything else in the "fees" column
              MyMoneyMoney n0 = MyMoneyMoney(qA["fees"]);
              qA["fees"] = (n0 + value).toString();
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
              qA["value"] = "";

              //convert to lowest fraction
              qA["split"] = (-(*it_split).shares() * xr).convert(fraction).toString();
              qA["rank"] = '2';
            } else {
              //this applies when the transaction has only 2 splits, or each split is going to be
              //shown separately, eg. transactions by category

              qA["split"] = "";
              qA["rank"] = '1';
            }

            qA ["memo"] = (*it_split).memo();

            // if different from base currency and not converting
            // show the currency of the split
            if (splitAcc.currencyId() != file->baseCurrency().id()) {
              if (!report.isConvertCurrency()) {
                qS["currency"] = splitAcc.currencyId();
              }
            } else {
              qS["currency"] = "";
            }

            if (! splitAcc.isIncomeExpense()) {
              qA["category"] = ((*it_split).shares().isNegative()) ?
                               i18n("Transfer from %1", splitAcc.fullName())
                               : i18n("Transfer to %1", splitAcc.fullName());
              qA["topcategory"] = splitAcc.topParentName();
              qA["categorytype"] = i18n("Transfer");
            } else {
              qA ["category"] = splitAcc.fullName();
              qA ["topcategory"] = splitAcc.topParentName();
              qA ["categorytype"] = KMyMoneyUtils::accountTypeToString(splitAcc.accountGroup());
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
                    qA["tag"] = i18n("[No Tag]");
                  else
                    for (int i = 0; i < tagIdListCache.size(); i++) {
                      qA["tag"] = file->tag(tagIdListCache[i]).name().simplified();
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
            qS["value"] = ((*it_split).shares() * xr).convert(fraction).toString();

            qS["rank"] = '1';

            qS["account"] = splitAcc.name();
            qS["accountid"] = splitAcc.id();
            qS["topaccount"] = splitAcc.topParentName();

            qS["category"] = ((*it_split).shares().isNegative())
                             ? i18n("Transfer to %1", a_fullname)
                             : i18n("Transfer from %1", a_fullname);

            qS["institution"] = institution.isEmpty()
                                ? i18n("No Institution")
                                : file->institution(institution).name();

            qS["memo"] = (*it_split).memo().isEmpty()
                         ? a_memo
                         : (*it_split).memo();

            //FIXME-ALEX When is used this? I can't find in which condition we arrive here... maybe this code is useless?
            QString delimiter = "";
            for (int i = 0; i < tagIdList.size(); i++) {
              qA["tag"] += delimiter + file->tag(tagIdList[i]).name().simplified();
              delimiter = '+';
            }

            qS["payee"] = payee.isEmpty()
                          ? qA["payee"]
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
    qA["currency"] = (m_config.isConvertCurrency() || ! account.isForeignCurrency()) ? "" : account.currency().id();

    qA["accountid"] = account.id();
    qA["account"] = account.name();
    qA["topaccount"] = account.topParentName();
    qA["institution"] = institution.isEmpty() ? i18n("No Institution") : file->institution(institution).name();
    qA["rank"] = "0";

    qA["price"] = startPrice.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();
    if (account.isInvest()) {
      qA["shares"] = startShares.toString();
    }

    qA["postdate"] = strStartDate;
    qA["balance"] = startBalance.convert(fraction).toString();
    qA["value"].clear();
    qA["id"] = 'A';
    m_rows += qA;

    //ending balance
    qA["price"] = endPrice.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();

    if (account.isInvest()) {
      qA["shares"] = endShares.toString();
    }

    qA["postdate"] = strEndDate;
    qA["balance"] = endBalance.toString();
    qA["rank"] = "3";
    qA["id"] = 'Z';
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

void QueryTable::constructPerformanceRow(const ReportAccount& account, TableRow& result, CashFlowList &all) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity security;

  //get fraction depending on type of account
  int fraction = account.currency().smallestAccountFraction();

  //
  // Calculate performance
  //

  // The following columns are created:
  //    Account, Value on <Opening>, Buys, Sells, Income, Value on <Closing>, Return%

  MyMoneyReport report = m_config;
  QDate startingDate;
  QDate endingDate;
  MyMoneyMoney price;
  report.validDateRange(startingDate, endingDate);
  startingDate = startingDate.addDays(-1);

  //calculate starting balance
  if (m_config.isConvertCurrency()) {
    price = account.deepCurrencyPrice(startingDate) * account.baseCurrencyPrice(startingDate);
  } else {
    price = account.deepCurrencyPrice(startingDate);
  }

  //work around if there is no price for the starting balance
  if (!(file->balance(account.id(), startingDate)).isZero()
      && account.deepCurrencyPrice(startingDate) == MyMoneyMoney::ONE) {
    MyMoneyTransactionFilter filter;
    //get the transactions for the time before the report
    filter.setDateFilter(QDate(), startingDate);
    filter.addAccount(account.id());
    filter.setReportAllSplits(true);

    QList<MyMoneyTransaction> startTransactions = file->transactionList(filter);
    if (startTransactions.size() > 0) {
      //get the last transaction
      MyMoneyTransaction startTrans = startTransactions.back();
      MyMoneySplit s = startTrans.splitByAccount(account.id());
      //get the price from the split of that account
      price = s.price();
      if (m_config.isConvertCurrency())
        price = price * account.baseCurrencyPrice(startingDate);
    }
  }
  if (m_config.isConvertCurrency()) {
    price = account.deepCurrencyPrice(startingDate) * account.baseCurrencyPrice(startingDate);
  } else {
    price = account.deepCurrencyPrice(startingDate);
  }

  MyMoneyMoney startingBal = file->balance(account.id(), startingDate) * price;

  //convert to lowest fraction
  startingBal = startingBal.convert(fraction);

  //calculate ending balance
  if (m_config.isConvertCurrency()) {
    price = account.deepCurrencyPrice(endingDate) * account.baseCurrencyPrice(endingDate);
  } else {
    price = account.deepCurrencyPrice(endingDate);
  }
  MyMoneyMoney endingBal = file->balance((account).id(), endingDate) * price;

  //convert to lowest fraction
  endingBal = endingBal.convert(fraction);

  CashFlowList buys;
  CashFlowList sells;
  CashFlowList reinvestincome;
  CashFlowList cashincome;

  report.setReportAllSplits(false);
  report.setConsiderCategory(true);
  report.clearAccountFilter();
  report.addAccount(account.id());
  QList<MyMoneyTransaction> transactions = file->transactionList(report);
  QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.constBegin();
  while (it_transaction != transactions.constEnd()) {
    // s is the split for the stock account
    MyMoneySplit s = (*it_transaction).splitByAccount(account.id());

    MyMoneySplit assetAccountSplit;
    QList<MyMoneySplit> feeSplits;
    QList<MyMoneySplit> interestSplits;
    MyMoneySecurity currency;
    MyMoneySplit::investTransactionTypeE transactionType;
    KMyMoneyUtils::dissectTransaction((*it_transaction), s, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);

    //get price for the day of the transaction if we have to calculate base currency
    //we are using the value of the split which is in deep currency
    if (m_config.isConvertCurrency()) {
      price = account.baseCurrencyPrice((*it_transaction).postDate()); //we only need base currency because the value is in deep currency
    } else {
      price = MyMoneyMoney::ONE;
    }

    MyMoneyMoney value = assetAccountSplit.value() * price;

    if (transactionType == MyMoneySplit::BuyShares)
      buys += CashFlowListItem((*it_transaction).postDate(), value);
    else if (transactionType == MyMoneySplit::SellShares)
      sells += CashFlowListItem((*it_transaction).postDate(), value);
    else if (transactionType == MyMoneySplit::ReinvestDividend) {
      value = interestSplits.first().value() * price;
      reinvestincome += CashFlowListItem((*it_transaction).postDate(), -value);
    } else if (transactionType == MyMoneySplit::Dividend || transactionType == MyMoneySplit::Yield)
      cashincome += CashFlowListItem((*it_transaction).postDate(), value);
    ++it_transaction;
  }
  // Note that reinvested dividends are not included , because these do not
  // represent a cash flow event.
  all += buys;
  all += sells;
  all += cashincome;
  all += CashFlowListItem(startingDate, -startingBal);
  all += CashFlowListItem(endingDate, endingBal);

  MyMoneyMoney buysTotal = buys.total();
  MyMoneyMoney sellsTotal = sells.total();
  MyMoneyMoney cashIncomeTotal = cashincome.total();
  MyMoneyMoney reinvestIncomeTotal = reinvestincome.total();

  MyMoneyMoney returnInvestment = helperROI(buysTotal, sellsTotal, startingBal, endingBal, cashIncomeTotal);
  MyMoneyMoney annualReturn = helperIRR(all);

  // check if there are any meaningfull values before adding them to results
  if (!(buysTotal.isZero() && sellsTotal.isZero() &&
        cashIncomeTotal.isZero() && reinvestIncomeTotal.isZero() &&
        startingBal.isZero() && endingBal.isZero())) {
    result["return"] = annualReturn.toString();
    result["returninvestment"] = returnInvestment.toString();
    result["equitytype"] = KMyMoneyUtils::securityTypeToString(security.securityType());
    result["buys"] = buysTotal.toString();
    result["sells"] = sellsTotal.toString();
    result["cashincome"] = cashIncomeTotal.toString();
    result["reinvestincome"] = reinvestIncomeTotal.toString();
    result["startingbal"] = startingBal.toString();
    result["endingbal"] = endingBal.toString();
  }
}

void QueryTable::constructCapitalGainRow(const ReportAccount& account, TableRow& result) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity security;
  MyMoneyMoney price;
  MyMoneyMoney sellValue;
  MyMoneyMoney buyValue;
  MyMoneyMoney sellShares;
  MyMoneyMoney buyShares;

  //
  // Calculate capital gain
  //

  // The following columns are created:
  //    Account, Buys, Sells, Capital Gain

  MyMoneyReport report = m_config;
  QDate startingDate;
  QDate endingDate;
  QDate newStartingDate;
  QDate newEndingDate;
  report.validDateRange(startingDate, endingDate);
  newStartingDate = startingDate;
  newEndingDate = endingDate;
  MyMoneyMoney endingShares = file->balance(account.id(), endingDate); // get how many shares there are over zero value

  bool reportedDateRange = true;  // flag marking sell transactions between startingDate and endingDate
  report.setReportAllSplits(false);
  report.setConsiderCategory(true);
  report.clearAccountFilter();
  report.addAccount(account.id());

  do {
    QList<MyMoneyTransaction> transactions = file->transactionList(report);
    for (QList<MyMoneyTransaction>::const_reverse_iterator  it_t = transactions.crbegin(); it_t != transactions.crend(); ++it_t) {
      MyMoneySplit shareSplit = (*it_t).splitByAccount(account.id());
      MyMoneySplit assetAccountSplit;
      QList<MyMoneySplit> feeSplits;
      QList<MyMoneySplit> interestSplits;
      MyMoneySecurity currency;
      MyMoneySplit::investTransactionTypeE transactionType;
      KMyMoneyUtils::dissectTransaction((*it_t), shareSplit, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
      //get price for the day of the transaction if we have to calculate base currency
      //we are using the value of the split which is in deep currency
      if (m_config.isConvertCurrency())
        price = account.baseCurrencyPrice((*it_t).postDate()); //we only need base currency because the value is in deep currency
      else
        price = MyMoneyMoney::ONE;

      MyMoneyMoney value = assetAccountSplit.value() * price;
      MyMoneyMoney shares = shareSplit.shares();

      if (transactionType == MyMoneySplit::BuyShares) {
        if (endingShares.isZero()) {    // add sold shares
          if (buyShares + shares > sellShares.abs()) { // add partially sold shares
            buyValue += (((sellShares.abs() - buyShares)) / shares) * value;
            buyShares = sellShares.abs();
          } else {                      // add wholly sold shares
            buyValue += value;
            buyShares += shares;
          }
        } else if (endingShares >= shares) { // substract not-sold shares
          endingShares -= shares;
        } else {                        // substract partially not-sold shares
          buyValue += ((shares - endingShares) / shares) * value;
          buyShares += (shares - endingShares);
          endingShares = MyMoneyMoney(0);
        }
      } else if (transactionType == MyMoneySplit::SellShares && reportedDateRange) {
        sellValue += value;
        sellShares += shares;
      } else if (transactionType == MyMoneySplit::SplitShares) { // shares variable is denominator of split ratio here
        sellShares /= shares;
        buyShares /= shares;
      } else if (transactionType == MyMoneySplit::AddShares) { // added shares, when sold give 100% capital gain
        if (endingShares.isZero()) {    // add added shares
          if (buyShares + shares > sellShares.abs()) { // add partially added shares
            buyShares = sellShares.abs();
          } else {                      // add wholly added shares
            buyShares += shares;
          }
        } else if (endingShares >= shares) { // substract not-added shares
          endingShares -= shares;
        } else {                        // substract partially not-added shares
          buyShares += (shares - endingShares);
          endingShares = MyMoneyMoney(0);
        }
      } else if (transactionType == MyMoneySplit::RemoveShares && reportedDateRange) { // removed shares give no value in return so no capital gain on them
        sellShares += shares;
      }
    }
    reportedDateRange = false;
    newEndingDate = newStartingDate;
    newStartingDate = newStartingDate.addYears(-1);
    report.setDateFilter(newStartingDate, newEndingDate); // search for matching buy transactions year earlier
  } while (!sellShares.isZero() && account.openingDate() <= newEndingDate && sellShares.abs() > buyShares.abs());

  // check if there are any meaningfull values before adding them to results
  if (!(buyValue.isZero() && sellValue.isZero())) {
    result["equitytype"] = KMyMoneyUtils::securityTypeToString(security.securityType());
    result["buys"] = buyValue.toString();
    result["sells"] = sellValue.toString();
    result["capitalgain"] = (buyValue + sellValue).toString();
  }
  report.setDateFilter(startingDate, endingDate); // reset data filter for next security
}

void QueryTable::constructAccountTable()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //make sure we have all subaccounts of investment accounts
  includeInvestmentSubAccounts();

  QMap<QString, CashFlowList> topAccounts; // for total calculation
  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);
  for (auto it_account = accounts.constBegin(); it_account != accounts.constEnd(); ++it_account) {
    // Note, "Investment" accounts are never included in account rows because
    // they don't contain anything by themselves.  In reports, they are only
    // useful as a "topaccount" aggregator of stock accounts
    if ((*it_account).isAssetLiability() && m_config.includes((*it_account)) && (*it_account).accountType() != MyMoneyAccount::Investment) {
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
            qaccountrow["topaccount"] = account.topParentName();
            if (!topAccounts.contains(qaccountrow["topaccount"]))
              topAccounts.insert(qaccountrow["topaccount"], accountCashflow);   // create cashflow for unknown account...
            else
              topAccounts[qaccountrow["topaccount"]] += accountCashflow;        // ...or add cashflow for known account
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

          qaccountrow["price"] = netprice.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();
          qaccountrow["value"] = (netprice * shares).convert(fraction).toString();
          qaccountrow["shares"] = shares.toString();

          QString iid = account.institutionId();

          // If an account does not have an institution, get it from the top-parent.
          if (iid.isEmpty() && !account.isTopLevel())
            iid = account.topParent().institutionId();

          if (iid.isEmpty())
            qaccountrow["institution"] = i18nc("No institution", "None");
          else
            qaccountrow["institution"] = file->institution(iid).name();

          qaccountrow["type"] = KMyMoneyUtils::accountTypeToString(account.accountType());
        }
      }

      if (qaccountrow.isEmpty()) // don't add the account if there are no calculated values
        continue;

      qaccountrow["rank"] = '1';
      qaccountrow["account"] = account.name();
      qaccountrow["accountid"] = account.id();
      qaccountrow["topaccount"] = account.topParentName();
      if (!m_config.isConvertCurrency())
        qaccountrow["currency"] = account.currency().id();
      m_rows.append(qaccountrow);
    }
  }

  if (m_config.queryColumns() == MyMoneyReport::eQCperformance) {
    TableRow qtotalsrow;
    qtotalsrow["rank"] = "4"; // add identification of row as total
    CashFlowList grandCashflow;

    // convert map of top accounts with cashflows to TableRow
    for (QMap<QString, CashFlowList>::iterator topAccount = topAccounts.begin(); topAccount != topAccounts.end(); ++topAccount) {
      qtotalsrow["topaccount"] = topAccount.key();
      qtotalsrow["return"] = helperIRR(topAccount.value()).toString();
      grandCashflow += topAccount.value();  // cumulative sum of cashflows of each topaccount
      m_rows.append(qtotalsrow);            // rows aren't sorted yet, so no problem with adding them randomly at the end
    }
    qtotalsrow["topaccount"] = "";          // empty topaccount because it's grand cashflow
    qtotalsrow["return"] = helperIRR(grandCashflow).toString();
    m_rows.append(qtotalsrow);
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

    qA["id"] = qS["id"] = (* it_transaction).id();
    qA["entrydate"] = qS["entrydate"] = (* it_transaction).entryDate().toString(Qt::ISODate);
    qA["postdate"] = qS["postdate"] = (* it_transaction).postDate().toString(Qt::ISODate);
    qA["commodity"] = qS["commodity"] = (* it_transaction).commodity();

    pd = (* it_transaction).postDate();
    qA["month"] = qS["month"] = i18n("Month of %1", QDate(pd.year(), pd.month(), 1).toString(Qt::ISODate));
    qA["week"] = qS["week"] = i18n("Week of %1", pd.addDays(1 - pd.dayOfWeek()).toString(Qt::ISODate));

    qA["currency"] = qS["currency"] = "";

    if ((* it_transaction).commodity() != file->baseCurrency().id()) {
      if (!report.isConvertCurrency()) {
        qA["currency"] = qS["currency"] = (*it_transaction).commodity();
      }
    }

    // to handle splits, we decide on which account to base the split
    // (a reference point or point of view so to speak). here we take the
    // first account that is a stock account or loan account (or the first account
    // that is not an income or expense account if there is no stock or loan account)
    // to be the account (qA) that will have the sub-item "split" entries. we add
    // one transaction entry (qS) for each subsequent entry in the split.
    const QList<MyMoneySplit>& splits = (*it_transaction).splits();
    QList<MyMoneySplit>::const_iterator myBegin, it_split;
    //S_end = splits.end();

    for (it_split = splits.begin(), myBegin = splits.end(); it_split != splits.end(); ++it_split) {
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
    QString a_fullname = "";
    QString a_memo = "";
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

        qA["action"] = (*it_split).action();
        qA["shares"] = shares.isZero() ? "" : (*it_split).shares().toString();
        qA["price"] = shares.isZero() ? "" : xr.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();

        if (((*it_split).action() == MyMoneySplit::ActionBuyShares) && (*it_split).shares().isNegative())
          qA["action"] = "Sell";

        qA["investaccount"] = splitAcc.parent().name();
      }

      include_me = m_config.includes(splitAcc);
      a_fullname = splitAcc.fullName();
      a_memo = (*it_split).memo();

      qA["price"] = xr.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();
      qA["account"] = splitAcc.name();
      qA["accountid"] = splitAcc.id();
      qA["topaccount"] = splitAcc.topParentName();

      qA["institution"] = institution.isEmpty()
                          ? i18n("No Institution")
                          : file->institution(institution).name();

      //FIXME-ALEX Is this useless? Isn't constructSplitsTable called only for cashflow type report?
      QString delimiter = "";
      for (int i = 0; i < tagIdList.size(); i++) {
        qA["tag"] += delimiter + file->tag(tagIdList[i]).name().simplified();
        delimiter = ',';
      }

      qA["payee"] = payee.isEmpty()
                    ? i18n("[Empty Payee]")
                    : file->payee(payee).name().simplified();

      qA["reconciledate"] = (*it_split).reconcileDate().toString(Qt::ISODate);
      qA["reconcileflag"] = KMyMoneyUtils::reconcileStateToString((*it_split).reconcileFlag(), true);
      qA["number"] = (*it_split).number();

      qA["memo"] = a_memo;

      qS["reconciledate"] = qA["reconciledate"];
      qS["reconcileflag"] = qA["reconcileflag"];
      qS["number"] = qA["number"];

      qS["topcategory"] = splitAcc.topParentName();

      // only include the configured accounts
      if (include_me) {
        // add the "summarized" split transaction
        // this is the sub-total of the split detail
        // convert to lowest fraction
        qA["value"] = ((*it_split).shares() * xr).convert(fraction).toString();
        qA["rank"] = '1';

        //fill in account information
        if (! splitAcc.isIncomeExpense() && it_split != myBegin) {
          qA["account"] = ((*it_split).shares().isNegative()) ?
                          i18n("Transfer to %1", myBeginAcc.fullName())
                          : i18n("Transfer from %1", myBeginAcc.fullName());
        } else if (it_split == myBegin) {
          //handle the main split
          if ((splits.count() > 2)) {
            //if it is the main split and has multiple splits, note that
            qA["account"] = i18n("[Split Transaction]");
          } else {
            //fill the account name of the second split
            QList<MyMoneySplit>::const_iterator tempSplit = splits.begin();

            //there are supposed to be only 2 splits if we ever get here
            if (tempSplit == myBegin && splits.count() > 1)
              ++tempSplit;

            //show the name of the category, or "transfer to/from" if it as an account
            ReportAccount tempSplitAcc = (*tempSplit).accountId();
            if (! tempSplitAcc.isIncomeExpense()) {
              qA["account"] = ((*it_split).shares().isNegative()) ?
                              i18n("Transfer to %1", tempSplitAcc.fullName())
                              : i18n("Transfer from %1", tempSplitAcc.fullName());
            } else {
              qA["account"] = tempSplitAcc.fullName();
            }
          }
        } else {
          //in any other case, fill in the account name of the main split
          qA["account"] = myBeginAcc.fullName();
        }

        //category data is always the one of the split
        qA ["category"] = splitAcc.fullName();
        qA ["topcategory"] = splitAcc.topParentName();
        qA ["categorytype"] = KMyMoneyUtils::accountTypeToString(splitAcc.accountGroup());

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
    qA["currency"] = (m_config.isConvertCurrency() || ! account.isForeignCurrency()) ? "" : account.currency().id();

    qA["accountid"] = account.id();
    qA["account"] = account.name();
    qA["topaccount"] = account.topParentName();
    qA["institution"] = institution.isEmpty() ? i18n("No Institution") : file->institution(institution).name();
    qA["rank"] = "0";

    qA["price"] = startPrice.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();
    if (account.isInvest()) {
      qA["shares"] = startShares.toString();
    }

    qA["postdate"] = strStartDate;
    qA["balance"] = startBalance.convert(fraction).toString();
    qA["value"].clear();
    qA["id"] = 'A';
    m_rows += qA;

    qA["rank"] = "3";
    //ending balance
    qA["price"] = endPrice.convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();

    if (account.isInvest()) {
      qA["shares"] = endShares.toString();
    }

    qA["postdate"] = strEndDate;
    qA["balance"] = endBalance.toString();
    qA["id"] = 'Z';
    m_rows += qA;
  }
}



}
