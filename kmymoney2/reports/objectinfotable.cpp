/***************************************************************************
                          objectinfotable.cpp
                         -------------------
    begin                : Sat 28 jun 2008
    copyright            : (C) 2004-2005 by Ace Jones
                               2008 by Alvaro Soliverez
    email                :  acejones@users.sourceforge.net
                            asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes
#include <qlist.h>
#include <qfile.h>
#include <q3textstream.h>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "reportaccount.h"
#include "reportdebug.h"
#include "objectinfotable.h"

namespace reports {

// ****************************************************************************
//
// ObjectInfoTable implementation
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

ObjectInfoTable::ObjectInfoTable(const MyMoneyReport& _report): ListTable(_report)
{
  // seperated into its own method to allow debugging (setting breakpoints
  // directly in ctors somehow does not work for me (ipwizard))
  // TODO: remove the init() method and move the code back to the ctor
  init();
}

void ObjectInfoTable::init ( void )
{
  switch ( m_config.rowType() )
  {
    case MyMoneyReport::eSchedule:
      constructScheduleTable();
      m_columns = "nextduedate,name";
      break;
    case MyMoneyReport::eAccountInfo:
      constructAccountTable();
      m_columns = "institution,type,name";
      break;
    case MyMoneyReport::eAccountLoanInfo:
      constructAccountLoanTable();
      m_columns = "institution,type,name";
      break;
    default:
      break;
  }

  // Sort the data to match the report definition
  m_subtotal="value";

  switch ( m_config.rowType() )
  {
    case MyMoneyReport::eSchedule:
      m_group = "type";
      m_subtotal="value";
      break;
    case MyMoneyReport::eAccountInfo:
    case MyMoneyReport::eAccountLoanInfo:
      m_group = "topcategory,institution";
      m_subtotal="currentbalance";
      break;
    default:
      throw new MYMONEYEXCEPTION ( "ObjectInfoTable::ObjectInfoTable(): unhandled row type" );
  }

  QString sort = m_group + "," + m_columns + ",id,rank";

  switch ( m_config.rowType() ) {
    case MyMoneyReport::eSchedule:
      if ( m_config.detailLevel() == MyMoneyReport::eDetailAll ) {
          m_columns="name,payee,paymenttype,occurence,nextduedate,category";
      } else {
          m_columns="name,payee,paymenttype,occurence,nextduedate";
      }
      break;
    case MyMoneyReport::eAccountInfo:
      m_columns="type,name,number,description,openingdate,currencyname,balancewarning,maxbalancelimit,creditwarning,maxcreditlimit,tax,favorite";
      break;
    case MyMoneyReport::eAccountLoanInfo:
      m_columns="type,name,number,description,openingdate,currencyname,payee,loanamount,interestrate,nextinterestchange,periodicpayment,finalpayment,favorite";
      break;
    default:
      m_columns = "";
  }

  TableRow::setSortCriteria ( sort );
  #warning #Port to KDE4
  //qSort ( m_rows );
}

void ObjectInfoTable::constructScheduleTable ( void )
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneySchedule> schedules;

  schedules = file->scheduleList ( "", MyMoneySchedule::TYPE_ANY, MyMoneySchedule::OCCUR_ANY, MyMoneySchedule::STYPE_ANY, m_config.fromDate(), m_config.toDate() );

  QList<MyMoneySchedule>::const_iterator it_schedule = schedules.begin();
  while ( it_schedule != schedules.end() )
  {
    MyMoneySchedule schedule = *it_schedule;

    ReportAccount account = schedule.account();

    if ( m_config.includes ( account ) )  {
      //get fraction for account
      int fraction = account.fraction();

      //use base currency fraction if not initialized
      if ( fraction == -1 )
        fraction = MyMoneyFile::instance()->baseCurrency().smallestAccountFraction();

      TableRow scheduleRow;

      //convert to base currency if needed
      MyMoneyMoney xr = MyMoneyMoney(1,1);
      if (m_config.isConvertCurrency() && account.isForeignCurrency()) {
        xr = account.baseCurrencyPrice(QDate::currentDate()).reduce();
      }

      // help for sort and render functions
      scheduleRow["rank"] = "0";

      //schedule data
      scheduleRow["id"] = schedule.id();
      scheduleRow["name"] = schedule.name();
      scheduleRow["nextduedate"] = schedule.nextDueDate().toString ( Qt::ISODate );
      scheduleRow["type"] = KMyMoneyUtils::scheduleTypeToString ( schedule.type() );
      scheduleRow["occurence"] = i18n( schedule.occurenceToString().toLatin1() );
      scheduleRow["paymenttype"] = KMyMoneyUtils::paymentMethodToString ( schedule.paymentType() );

      //scheduleRow["category"] = account.name();

      //to get the payee we must look into the splits of the transaction
      MyMoneyTransaction transaction = schedule.transaction();
      MyMoneySplit split = transaction.splitByAccount ( account.id(), true );
      scheduleRow["value"] = (split.value() * xr).toString();
      MyMoneyPayee payee = file->payee ( split.payeeId() );
      scheduleRow["payee"] = payee.name();
      m_rows += scheduleRow;

      //the text matches the main split
      bool transaction_text = m_config.match(&split);

      if ( m_config.detailLevel() == MyMoneyReport::eDetailAll )
      {
        //get the information for all splits
        QList<MyMoneySplit> splits = transaction.splits();
        QList<MyMoneySplit>::const_iterator split_it = splits.begin();
        for ( ;split_it != splits.end(); split_it++ )
        {
          TableRow splitRow;
          ReportAccount splitAcc = ( *split_it ).accountId();

          splitRow["rank"] = "1";
          splitRow["id"] = schedule.id();
          splitRow["name"] = schedule.name();
          splitRow["type"] = KMyMoneyUtils::scheduleTypeToString ( schedule.type() );
          splitRow["nextduedate"] = schedule.nextDueDate().toString ( Qt::ISODate );

          if ( ( *split_it ).value() == MyMoneyMoney::autoCalc ) {
            splitRow["split"] = MyMoneyMoney::autoCalc.toString();
          } else if ( ! splitAcc.isIncomeExpense() ) {
            splitRow["split"] = ( *split_it ).value().toString();
          } else {
            splitRow["split"] = ( - ( *split_it ).value() ).toString();
          }

          //if it is an assett account, mark it as a transfer
          if ( ! splitAcc.isIncomeExpense() ) {
            splitRow["category"] = ( ( * split_it ).value().isNegative() )
                                   ? i18n ( "Transfer from %1" ).arg ( splitAcc.fullName() )
                                   : i18n ( "Transfer to %1" ).arg ( splitAcc.fullName() );
          } else {
            splitRow ["category"] = splitAcc.fullName();
          }

          //add the split only if it matches the text or it matches the main split
          if(m_config.match( &(*split_it) )
             || transaction_text )
            m_rows += splitRow;
        }
      }
    }
    ++it_schedule;
  }
}

void ObjectInfoTable::constructAccountTable ( void )
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);
  QList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
  while ( it_account != accounts.end() )
  {
    TableRow accountRow;
    ReportAccount account = *it_account;

    if(m_config.includes(account)
       && account.accountType() != MyMoneyAccount::Stock
       && !account.isClosed())
    {
      MyMoneyMoney value;
      accountRow["rank"] = "0";
      accountRow["topcategory"] = KMyMoneyUtils::accountTypeToString(account.accountGroup());
      accountRow["institution"] = (file->institution(account.institutionId())).name();
      accountRow["type"] = KMyMoneyUtils::accountTypeToString(account.accountType());
      accountRow["name"] = account.name();
      accountRow["number"] = account.number();
      accountRow["description"] = account.description();
      accountRow["openingdate"] = account.openingDate().toString( Qt::ISODate );
      //accountRow["currency"] = (file->currency(account.currencyId())).tradingSymbol();
      accountRow["currencyname"] = (file->currency(account.currencyId())).name();
      accountRow["balancewarning"] = account.value("minBalanceEarly");
      accountRow["maxbalancelimit"] = account.value("minBalanceAbsolute");
      accountRow["creditwarning"] = account.value("maxCreditEarly");
      accountRow["maxcreditlimit"] = account.value("maxCreditAbsolute");
      accountRow["tax"] = account.value("Tax");
      accountRow["favorite"] = account.value("PreferredAccount");

      //investment accounts show the balances of all its subaccounts
      if(account.accountType() == MyMoneyAccount::Investment) {
        value = investmentBalance(account);
      } else {
        value = file->balance(account.id());
      }

      //convert to base currency if needed
      if (m_config.isConvertCurrency() && account.isForeignCurrency()) {
        MyMoneyMoney xr = account.baseCurrencyPrice(QDate::currentDate()).reduce();
        value = value * xr;
      }
      accountRow["currentbalance"] = value.toString();

      m_rows += accountRow;
    }
    ++it_account;
  }
}

void ObjectInfoTable::constructAccountLoanTable ( void )
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);
  QList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
  while ( it_account != accounts.end() )
  {
    TableRow accountRow;
    ReportAccount account = *it_account;
    MyMoneyAccountLoan loan = *it_account;

    if(m_config.includes(account) &&
       ( account.accountType() == MyMoneyAccount::Loan
       || account.accountType() == MyMoneyAccount::AssetLoan )
       && !account.isClosed())
    {
      //convert to base currency if needed
      MyMoneyMoney xr = MyMoneyMoney(1,1);
      if (m_config.isConvertCurrency() && account.isForeignCurrency()) {
         xr = account.baseCurrencyPrice(QDate::currentDate()).reduce();
      }

      accountRow["rank"] = "0";
      accountRow["topcategory"] = KMyMoneyUtils::accountTypeToString(account.accountGroup());
      accountRow["institution"] = (file->institution(account.institutionId())).name();
      accountRow["type"] = KMyMoneyUtils::accountTypeToString(account.accountType());
      accountRow["name"] = account.name();
      accountRow["number"] = account.number();
      accountRow["description"] = account.description();
      accountRow["openingdate"] = account.openingDate().toString( Qt::ISODate );
      //accountRow["currency"] = (file->currency(account.currencyId())).tradingSymbol();
      accountRow["currencyname"] = (file->currency(account.currencyId())).name();
      accountRow["payee"] = file->payee(loan.payee()).name();
      accountRow["loanamount"] = (loan.loanAmount() * xr).toString();
      accountRow["interestrate"] = (loan.interestRate(QDate::currentDate())/MyMoneyMoney(100,1)*xr).toString();
      accountRow["nextinterestchange"] = loan.nextInterestChange().toString( Qt::ISODate );
      accountRow["periodicpayment"] = (loan.periodicPayment() * xr).toString();
      accountRow["finalpayment"] = (loan.finalPayment() * xr).toString();
      accountRow["favorite"] = account.value("PreferredAccount");

      MyMoneyMoney value = file->balance(account.id());
      value = value * xr;
      accountRow["currentbalance"] = value.toString();
      m_rows += accountRow;
    }
    ++it_account;
  }
}

MyMoneyMoney ObjectInfoTable::investmentBalance(const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyMoney value = file->balance(acc.id());
  QStringList accList = acc.accountList();

  QStringList::const_iterator it_a = accList.begin();
  for(; it_a != acc.accountList().end(); ++it_a) {
    MyMoneyAccount stock = file->account(*it_a);
    try {
      MyMoneyMoney val;
      MyMoneyMoney balance = file->balance(stock.id());
      MyMoneySecurity security = file->security(stock.currencyId());
      MyMoneyPrice price = file->price(stock.currencyId(), security.tradingCurrency());
      val = balance * price.rate(security.tradingCurrency());
      // adjust value of security to the currency of the account
      MyMoneySecurity accountCurrency = file->currency(acc.currencyId());
      val = val * file->price(security.tradingCurrency(), accountCurrency.id()).rate(accountCurrency.id());
      val = val.convert(acc.fraction());
      value += val;
    } catch(MyMoneyException* e) {
      qWarning("%s", qPrintable(QString("cannot convert stock balance of %1 to base currency: %2").arg(stock.name(), e->what())));
      delete e;
    }
  }
  return value;
}

}
// vim:cin:si:ai:et:ts=2:sw=2:
