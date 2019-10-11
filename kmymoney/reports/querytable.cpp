/***************************************************************************
                          querytable.cpp
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004-2005 by Ace Jones <acejones@users.sourceforge.net>

***************************************************************************/

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

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "cashflowlist.h"
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
    case MyMoneyReport::Row::AccountByTopAccount:
    case MyMoneyReport::Row::EquityType:
    case MyMoneyReport::Row::AccountType:
    case MyMoneyReport::Row::Institution:
      constructAccountTable();
      m_columns = "account";
      break;

    case MyMoneyReport::Row::Account:
      constructTransactionTable();
      m_columns = "accountid,postdate";
      break;

    case MyMoneyReport::Row::Payee:
    case MyMoneyReport::Row::Tag:
    case MyMoneyReport::Row::Month:
    case MyMoneyReport::Row::Week:
      constructTransactionTable();
      m_columns = "postdate,account";
      break;
    case MyMoneyReport::Row::CashFlow:
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
    case MyMoneyReport::Row::CashFlow:
      m_group = "categorytype,topcategory,category";
      break;
    case MyMoneyReport::Row::Category:
      m_group = "categorytype,topcategory,category";
      break;
    case MyMoneyReport::Row::TopCategory:
      m_group = "categorytype,topcategory";
      break;
    case MyMoneyReport::Row::TopAccount:
      m_group = "topaccount,account";
      break;
    case MyMoneyReport::Row::Account:
      m_group = "account";
      break;
    case MyMoneyReport::Row::AccountReconcile:
      m_group = "account,reconcileflag";
      break;
    case MyMoneyReport::Row::Payee:
      m_group = "payee";
      break;
    case MyMoneyReport::Row::Tag:
      m_group = "tag";
      break;
    case MyMoneyReport::Row::Month:
      m_group = "month";
      break;
    case MyMoneyReport::Row::Week:
      m_group = "week";
      break;
    case MyMoneyReport::Row::AccountByTopAccount:
      m_group = "topaccount";
      break;
    case MyMoneyReport::Row::EquityType:
      m_group = "equitytype";
      break;
    case MyMoneyReport::Row::AccountType:
      m_group = "type";
      break;
    case MyMoneyReport::Row::Institution:
      m_group = "institution,topaccount";
      break;
    default:
      throw MYMONEYEXCEPTION("QueryTable::QueryTable(): unhandled row type");
  }

  QString sort;
  switch (m_config.rowType()) {
    case MyMoneyReport::Row::Month:
    case MyMoneyReport::Row::Week:
      sort = m_group + "sort," + m_columns + ",id,rank";
      break;

    default:
      sort = m_group + ',' + m_columns + ",id,rank";
      break;
  }

  switch (m_config.rowType()) {
    case MyMoneyReport::Row::AccountByTopAccount:
    case MyMoneyReport::Row::EquityType:
    case MyMoneyReport::Row::AccountType:
    case MyMoneyReport::Row::Institution:
      m_columns = "account";
      break;

    default:
      m_columns = "postdate";
  }

  unsigned qc = m_config.queryColumns();

  if (qc & MyMoneyReport::QueryColumns::Number)
    m_columns += ",number";
  if (qc & MyMoneyReport::QueryColumns::Payee)
    m_columns += ",payee";
  if (qc & MyMoneyReport::QueryColumns::Tag)
    m_columns += ",tag";
  if (qc & MyMoneyReport::QueryColumns::Category)
    m_columns += ",category";
  if (qc & MyMoneyReport::QueryColumns::Account)
    m_columns += ",account";
  if (qc & MyMoneyReport::QueryColumns::Reconciled)
    m_columns += ",reconcileflag";
  if (qc & MyMoneyReport::QueryColumns::Memo)
    m_columns += ",memo";
  if (qc & MyMoneyReport::QueryColumns::Action)
    m_columns += ",action";
  if (qc & MyMoneyReport::QueryColumns::Shares)
    m_columns += ",shares";
  if (qc & MyMoneyReport::QueryColumns::Price)
    m_columns += ",price";
  if (qc & MyMoneyReport::QueryColumns::Performance)
    m_columns += ",startingbal,buys,sells,reinvestincome,cashincome,return,returninvestment";
  if (qc & MyMoneyReport::QueryColumns::Loan) {
    m_columns += ",payment,interest,fees";
    m_postcolumns = "balance";
  }
  if (qc & MyMoneyReport::QueryColumns::Balance)
    m_postcolumns = "balance";

  TableRow::setSortCriteria(sort);

  qSort(m_rows);
}

/**
 * helper function used by querytabletest
 * @param date date to create the string from
 * @return string with formatted date
 */
QString QueryTable::toDateString(const QDate &date)
{
    return date.toString(Qt::DefaultLocaleLongDate);
}

void QueryTable::constructTransactionTable()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //make sure we have all subaccounts of investment accounts
  includeInvestmentSubAccounts();

  MyMoneyReport report(m_config);
  report.setReportAllSplits(false);
  report.setConsiderCategory(true);

  bool use_transfers = false;
  bool use_summary = false;
  bool hide_details = false;
  bool tag_special_case = false;

  switch (m_config.rowType()) {
    case MyMoneyReport::Row::Category:
    case MyMoneyReport::Row::TopCategory:
      use_summary = false;
      use_transfers = false;
      hide_details = false;
      break;
    case MyMoneyReport::Row::Payee:
      use_summary = false;
      use_transfers = false;
      hide_details = (m_config.detailLevel() == MyMoneyReport::DetailLevel::None);
      break;
    case MyMoneyReport::Row::Tag:
      use_summary = false;
      use_transfers = false;
      hide_details = (m_config.detailLevel() == MyMoneyReport::DetailLevel::None);
      tag_special_case = true;
      report.setConsiderCategorySplits(true);
      break;
    default:
      use_summary = true;
      use_transfers = true;
      hide_details = (m_config.detailLevel() == MyMoneyReport::DetailLevel::None);
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
    qA["monthsort"] = qS["monthsort"] = i18n("Month of %1", QDate(pd.year(), pd.month(), 1).toString(Qt::ISODate));
    qA["weeksort"] = qS["weeksort"] = i18n("Week of %1", pd.addDays(1 - pd.dayOfWeek()).toString(Qt::ISODate));
    qA["month"] = qS["month"] = i18n("Month of %1", toDateString(QDate(pd.year(), pd.month(), 1)));
    qA["week"] = qS["week"] = i18n("Week of %1", toDateString(pd.addDays(1 - pd.dayOfWeek())));

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
    if (m_config.queryColumns() & MyMoneyReport::QueryColumns::Loan) {
      ReportAccount splitAcc = (*it_split).accountId();
      loan_special_case = splitAcc.isLoan();
    }

    bool include_me = true;
    bool transaction_text = false; //indicates whether a text should be considered as a match for the transaction or for a split only
    QString a_fullname = "";
    QString a_memo = "";
    int pass = 1;
    QString myBeginCurrency = (file->account((*myBegin).accountId())).currencyId(); //currency of the main split
    qA["tag"] = tag_special_case ? i18n("[No Tag]") : QLatin1String("");

    do {
      MyMoneyMoney xr;
      ReportAccount splitAcc = (* it_split).accountId();

      if (include_me) {
        // handle tags
        const QStringList tagIdList = (*it_split).tagIdList();
        foreach (const QString &tagId, tagIdList) {
          if (!tagIdListCache.contains(tagId))
            tagIdListCache << tagId;
        }
        if (tagIdListCache.size() > 0) {
          qSort(tagIdListCache);
          qA["tag"] = "";
          QString delimiter = "";
          foreach (const QString &tagId, tagIdListCache) {
            qA["tag"] += delimiter + file->tag(tagId).name().simplified();
            delimiter = ", ";
          }
        }
      }

      //get fraction for account
      int fraction = splitAcc.currency().smallestAccountFraction();

      //use base currency fraction if not initialized
      if (fraction == -1)
        fraction = file->baseCurrency().smallestAccountFraction();

      QString institution = splitAcc.institutionId();
      QString payee = (*it_split).payeeId();

      //convert to base currency
      if (m_config.isConvertCurrency()) {
        xr = (splitAcc.deepCurrencyPrice((*it_transaction).postDate()) * splitAcc.baseCurrencyPrice((*it_transaction).postDate())).reduce();
      } else {
        xr = (splitAcc.deepCurrencyPrice((*it_transaction).postDate())).reduce();
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

      if (it_split == myBegin) {

        include_me = m_config.includes(splitAcc);
        a_fullname = splitAcc.fullName();
        a_memo = (*it_split).memo();

        transaction_text = m_config.match(&(*it_split));

        qA["price"] = xr.toString();
        qA["account"] = splitAcc.name();
        qA["accountid"] = splitAcc.id();
        qA["topaccount"] = splitAcc.topParentName();

        qA["institution"] = institution.isEmpty()
                            ? i18n("No Institution")
                            : file->institution(institution).name();

        qA["payee"] = payee.isEmpty()
                      ? i18n("[Empty Payee]")
                      : file->payee(payee).name().simplified();

        qA["reconciledate"] = (*it_split).reconcileDate().toString(Qt::ISODate);
        qA["reconcileflag"] = KMyMoneyUtils::reconcileStateToString((*it_split).reconcileFlag(), true);
        qA["number"] = (*it_split).number();

        qA["memo"] = a_memo;

        qA["value"] = (((*it_split).shares()) * xr).convert(fraction).toString();

        qS["reconciledate"] = qA["reconciledate"];
        qS["reconcileflag"] = qA["reconcileflag"];
        qS["number"] = qA["number"];

        qS["topcategory"] = splitAcc.topParentName();
        qS["categorytype"] = i18n("Transfer");

        // only include the configured accounts
        if (include_me) {

          if (loan_special_case) {

            // put the principal amount in the "value" column and convert to lowest fraction
            qA["value"] = ((-(*it_split).shares()) * xr).convert(fraction).toString();

            qA["rank"] = '0';
            qA["split"] = "";

          } else {
            if ((splits.count() > 2) && use_summary) {

              // add the "summarized" split transaction
              // this is the sub-total of the split detail
              // convert to lowest fraction
              qA["value"] = ((*it_split).shares() * xr).convert(fraction).toString();
              qA["rank"] = '0';
              qA["category"] = i18n("[Split Transaction]");
              qA["topcategory"] = i18nc("Split transaction", "Split");
              qA["categorytype"] = i18nc("Split transaction", "Split");

              m_rows += qA;
            }
          }

          // track accts that will need opening and closing balances
          //FIXME in some cases it will show the opening and closing
          //balances but no transactions if the splits are all filtered out -- asoliverez
          accts.insert(splitAcc.id(), splitAcc);
        }

      } else {

        if (include_me) {

          if (loan_special_case) {
            MyMoneyMoney value = ((-(* it_split).shares()) * xr).convert(fraction);

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
              qA["split"] = ((-(*it_split).shares()) * xr).convert(fraction).toString();
              qA["rank"] = '1';
            } else {
              //this applies when the transaction has only 2 splits, or each split is going to be
              //shown separately, eg. transactions by category

              qA["split"] = "";

              // multiply by currency and convert to lowest fraction
              // but only for income and expense
              // transfers are dealt with somewhere else below
              if (splitAcc.isIncomeExpense()) {
                // if the currency of the split is different from the currency of the main split, then convert to the currency of the main split
                MyMoneyMoney ieXr(xr);
                if (!m_config.isConvertCurrency() && splitAcc.currency().id() != myBeginCurrency) {
                  ieXr = (xr * splitAcc.foreignCurrencyPrice(myBeginCurrency, (*it_transaction).postDate())).reduce();
                }
                qA["value"] = ((-(*it_split).shares()) * ieXr).convert(fraction).toString();
              }
              qA["rank"] = '0';
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
                m_rows += qA;
              }
            }
          }
        }

        if (m_config.includes(splitAcc) && use_transfers) {
          if (! splitAcc.isIncomeExpense()) {
            //multiply by currency and convert to lowest fraction
            qS["value"] = ((*it_split).shares() * xr).convert(fraction).toString();

            qS["rank"] = '0';

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
    case MyMoneyReport::Row::Account:
    case MyMoneyReport::Row::TopAccount:
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
    qA["rank"] = "-2";

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
    qA["id"] = 'Z';
    m_rows += qA;
  }
}

void QueryTable::constructPerformanceRow(const ReportAccount& account, TableRow& result) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity security = file->security(account.currencyId());

  result["equitytype"] = KMyMoneyUtils::securityTypeToString(security.securityType());

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

  //add start balance to calculate return on investment
  MyMoneyMoney returnInvestment = startingBal;
  MyMoneyMoney paidDividend;
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

    //get price for the day of the transaction if we have to calculate base currency
    //we are using the value of the split which is in deep currency
    if (m_config.isConvertCurrency()) {
      price = account.baseCurrencyPrice((*it_transaction).postDate()); //we only need base currency because the value is in deep currency
    } else {
      price = MyMoneyMoney::ONE;
    }

    MyMoneyMoney value = s.value() * price;

    const QString& action = s.action();
    if (action == MyMoneySplit::ActionBuyShares) {
      if (s.value().isPositive()) {
        buys += CashFlowListItem((*it_transaction).postDate(), -value);
      } else {
        sells += CashFlowListItem((*it_transaction).postDate(), -value);
      }
      returnInvestment += value;
      //convert to lowest fraction
      returnInvestment = returnInvestment.convert(fraction);
    } else if (action == MyMoneySplit::ActionReinvestDividend) {
      reinvestincome += CashFlowListItem((*it_transaction).postDate(), value);
    } else if (action == MyMoneySplit::ActionDividend || action == MyMoneySplit::ActionYield) {
      // find the split with the category, which has the actual amount of the dividend
      QList<MyMoneySplit> splits = (*it_transaction).splits();
      QList<MyMoneySplit>::const_iterator it_split = splits.constBegin();
      while (it_split != splits.constEnd()) {
        ReportAccount acc = (*it_split).accountId();
        if (acc.isIncomeExpense()) {
          cashincome += CashFlowListItem((*it_transaction).postDate(), -(*it_split).value() * price);
          paidDividend += ((-(*it_split).value()) * price).convert(fraction);
        }
        ++it_split;
      }
    } else if (action == MyMoneySplit::ActionAddShares) {
      // Add shares is not a buy operation, do nothing
    } else {
      //if the split does not match any action above, add it as buy or sell depending on sign

      //if value is zero, get the price for that date
      if (s.value().isZero()) {
        if (m_config.isConvertCurrency()) {
          price = account.deepCurrencyPrice((*it_transaction).postDate()) * account.baseCurrencyPrice((*it_transaction).postDate());
        } else {
          price = account.deepCurrencyPrice((*it_transaction).postDate());
        }
        value = s.shares() * price;
        if (s.shares().isPositive()) {
          buys += CashFlowListItem((*it_transaction).postDate(), -value);
        } else {
          sells += CashFlowListItem((*it_transaction).postDate(), -value);
        }
        returnInvestment += value;
      } else {
        value = s.value() * price;
        if (s.value().isPositive()) {
          buys += CashFlowListItem((*it_transaction).postDate(), -value);
        } else {
          sells += CashFlowListItem((*it_transaction).postDate(), -value);
        }
        returnInvestment += value;
      }
    }
    ++it_transaction;
  }

  // Note that reinvested dividends are not included , because these do not
  // represent a cash flow event.
  CashFlowList all;
  all += buys;
  all += sells;
  all += cashincome;
  all += CashFlowListItem(startingDate, -startingBal);
  all += CashFlowListItem(endingDate, endingBal);

  //check if no activity on that term
  if (!returnInvestment.isZero() && !endingBal.isZero()) {
    returnInvestment = ((endingBal + paidDividend) - returnInvestment) / returnInvestment;
    returnInvestment = returnInvestment.convert(10000);
    result["returninvestment"] = returnInvestment.toString();
  } else {
    result["returninvestment"] = QString();
  }

  try {
    MyMoneyMoney annualReturn(all.XIRR(), 10000);
    result["return"] = annualReturn.toString();
  } catch (MyMoneyException &e) {
    result["return"] = QString();
    kDebug(2) << e.what();
  }

  result["buys"] = (-(buys.total())).toString();
  result["sells"] = (-(sells.total())).toString();
  result["cashincome"] = (cashincome.total()).toString();
  result["reinvestincome"] = (reinvestincome.total()).toString();
  result["startingbal"] = (startingBal).toString();
  result["endingbal"] = (endingBal).toString();
}

void QueryTable::constructAccountTable()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //make sure we have all subaccounts of investment accounts
  includeInvestmentSubAccounts();

  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);
  QList<MyMoneyAccount>::const_iterator it_account = accounts.constBegin();
  while (it_account != accounts.constEnd()) {
    ReportAccount account = *it_account;

    //get fraction for account
    int fraction = account.currency().smallestAccountFraction();

    //use base currency fraction if not initialized
    if (fraction == -1)
      fraction = MyMoneyFile::instance()->baseCurrency().smallestAccountFraction();

    // Note, "Investment" accounts are never included in account rows because
    // they don't contain anything by themselves.  In reports, they are only
    // useful as a "topaccount" aggregator of stock accounts
    if (account.isAssetLiability() && m_config.includes(account) && account.accountType() != MyMoneyAccount::Investment) {
      TableRow qaccountrow;

      // help for sort and render functions
      qaccountrow["rank"] = '0';

      //
      // Handle currency conversion
      //

      MyMoneyMoney displayprice(1, 1);
      if (m_config.isConvertCurrency()) {
        // display currency is base currency, so set the price
        if (account.isForeignCurrency())
          displayprice = account.baseCurrencyPrice(m_config.toDate()).reduce();
      } else {
        // display currency is the account's deep currency.  display this fact in the report
        qaccountrow["currency"] = account.currency().id();
      }

      qaccountrow["account"] = account.name();
      qaccountrow["accountid"] = account.id();
      qaccountrow["topaccount"] = account.topParentName();

      MyMoneyMoney shares = file->balance(account.id(), m_config.toDate());
      qaccountrow["shares"] = shares.toString();

      MyMoneyMoney netprice = account.deepCurrencyPrice(m_config.toDate()).reduce() * displayprice;
      qaccountrow["price"] = (netprice.reduce()).convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())).toString();
      qaccountrow["value"] = (netprice.reduce() * shares.reduce()).convert(fraction).toString();

      QString iid = (*it_account).institutionId();

      // If an account does not have an institution, get it from the top-parent.
      if (iid.isEmpty() && ! account.isTopLevel()) {
        ReportAccount topaccount = account.topParent();
        iid = topaccount.institutionId();
      }

      if (iid.isEmpty())
        qaccountrow["institution"] = i18nc("No institution", "None");
      else
        qaccountrow["institution"] = file->institution(iid).name();

      qaccountrow["type"] = KMyMoneyUtils::accountTypeToString((*it_account).accountType());

      // TODO: Only do this if the report we're making really needs performance.  Otherwise
      // it's an expensive calculation done for no reason
      if (account.isInvest()) {
        constructPerformanceRow(account, qaccountrow);
      } else
        qaccountrow["equitytype"].clear();

      // don't add the account if it is closed. In fact, the business logic
      // should prevent that an account can be closed with a balance not equal
      // to zero, but we never know.
      if (!(shares.isZero() && account.isClosed()))
        m_rows += qaccountrow;
    }

    ++it_account;
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
    if (m_config.queryColumns() & MyMoneyReport::QueryColumns::Loan) {
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
        qA["rank"] = '0';

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
    case MyMoneyReport::Row::Account:
    case MyMoneyReport::Row::TopAccount:
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
    qA["rank"] = "-2";

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
    qA["id"] = 'Z';
    m_rows += qA;
  }
}



}
