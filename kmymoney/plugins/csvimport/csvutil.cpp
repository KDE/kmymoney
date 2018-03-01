/***************************************************************************
                                      csvutil.cpp
                                     -------------
    begin                    :      Sat Jan 01 2010
    copyright                : (C) 2010 by Allan Anderson
    email                    :    agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "csvutil.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <mymoneyfile.h>
#include "kmymoneyutils.h"
#include "investtransactioneditor.h"
#include "transactioneditor.h"
#include "mymoneyaccount.h"

Parse::Parse() :
    m_csvDialog(0),
    m_decimalSymbolIndex(0),
    m_fieldDelimiterIndex(0),
    m_lastLine(0),
    m_textDelimiterIndex(0),
    m_thousandsSeparatorIndex(0),
    m_symbolFound(false)
{
  m_fieldDelimiterCharList << "," << ";" << ":" << "\t";
  m_fieldDelimiterCharacter = m_fieldDelimiterCharList[m_fieldDelimiterIndex];
  m_textDelimiterCharList << "\"" << "'";
  m_textDelimiterCharacter = m_textDelimiterCharList[m_textDelimiterIndex];
  m_decimalSymbolList << "." << ",";
  m_thousandsSeparatorList << "," << ".";
  m_invalidConversion = false;
}

Parse::~Parse()
{
}

QStringList Parse::parseLine(const QString& data)
{
  QStringList listIn;
  QStringList listOut;
  QString txt;
  QString txt1;
  m_fieldDelimiterCharacter = m_fieldDelimiterCharList[m_fieldDelimiterIndex];
  m_inBuffer = data;
  listIn = m_inBuffer.split(m_fieldDelimiterCharacter);  // firstly, split on m_fieldDelimiterCharacter

  QStringList::const_iterator constIterator;

  for (constIterator = listIn.constBegin(); constIterator < listIn.constEnd(); ++constIterator) {
    txt = (*constIterator);

    // detect where a "quoted" string has been erroneously split, because of a comma,
    // or in a value, a 'thousand separator' being mistaken for a field delimiter.
    //Also, where a 'field seperator' is within quotes and the quotes don't include the whole of the field.

    while ((txt.startsWith(m_textDelimiterCharacter)) && (!txt.mid(1, -1).contains(m_textDelimiterCharacter)))  {
      if (++constIterator < listIn.constEnd())  {
        txt1 = (*constIterator);//                       second part of the split string
        txt += m_fieldDelimiterCharacter + txt1;//       rejoin the string
      } else break;
    }
    listOut += txt.remove(m_textDelimiterCharacter);
  }
  return listOut;
}

QStringList Parse::parseFile(const QString& buf, int strt, int end)
{
  QStringList outBuffer;
  outBuffer.clear();
  int lineCount = 0;
  QString tmpBuffer;
  tmpBuffer.clear();
  bool inQuotes = false;
  int charCount = buf.count();
  QString::const_iterator constIterator;

  for (constIterator = buf.constBegin(); constIterator != buf.constEnd();
       ++constIterator) {
    QString chr = (*constIterator);
    charCount -= 1;
    if (chr == m_textDelimiterCharacter) {
      tmpBuffer += chr;
      if (inQuotes == true) {      //               if already in quoted field..
        inQuotes = false;//                    ..end it
      } else {//                               if not..
        inQuotes = true;//                     ..start it
      }
      continue;

      //  find carriage return and line feed chars

    } else if ((chr == "\r") || (chr == "\n")) {
      if (inQuotes == true) {      //               embedded '\n'  or '\r' in quoted field
        chr = '~';//                           replace it with ~ for now
        tmpBuffer += chr;
        if (charCount > 0)      //                      more chars yet
          continue;//                          more chars yet
      }
      //                                       true EOL (not in quotes)
      if (tmpBuffer.isEmpty()) {
        continue;
      }
      lineCount ++;
      if (lineCount < strt) {      //   startLine      not yet reached first wanted line
        tmpBuffer.clear();
        continue;
      }
      outBuffer << tmpBuffer;
      tmpBuffer.clear();
      //                                       look for start of wanted data
      //  if first pass or if not at last line, proceed
      if ((!end == 0) && (lineCount >= end)) {      //  m_endLine is set from UI after first pass
        m_lastLine = lineCount;
        break;
      }
    }

    //  end of 'EOL detected' loop

    else {//                                   must be data char
      tmpBuffer += chr;

      if (charCount > 0) {    //                      more chars yet
        continue;
      } else {//                                      else eoFile = true;
        //  last char in file is data char
        //  meaning no return on last line
        //  so bump line count
        lineCount ++;
      }
    }
    if (!tmpBuffer.isEmpty()) {
      outBuffer << tmpBuffer;
    }
  }
  m_lastLine = lineCount;
  return outBuffer;
}

QString Parse::fieldDelimiterCharacter(int index)
{
  if (index == -1) {
    return 0;
  }
  return m_fieldDelimiterCharList[index];
}

void Parse::setFieldDelimiterCharacter(int index)
{
  m_fieldDelimiterCharacter = m_fieldDelimiterCharList[index];
}

void Parse::setFieldDelimiterIndex(int index)
{
  m_fieldDelimiterIndex = index;
}

QString Parse::textDelimiterCharacter(int index)
{
  return m_textDelimiterCharList[index];
}

void Parse::setTextDelimiterCharacter(int index)
{
  m_textDelimiterCharacter = m_textDelimiterCharList[index];
}

void Parse::setTextDelimiterIndex(int index)
{
  m_textDelimiterIndex = index;
}

void Parse::decimalSymbolSelected(int val)
{
  if (val < 0) {
    return;
  }

  m_decimalSymbolIndex = val;
  m_decimalSymbol = m_decimalSymbolList[val];
  thousandsSeparatorChanged(val);
}

QString Parse::decimalSymbol(int index)
{
  return m_decimalSymbolList[index];
}

void Parse::setDecimalSymbol(int index)
{
  m_decimalSymbol = m_decimalSymbolList[index];
}

int Parse::decimalSymbolIndex()
{
  return m_decimalSymbolIndex;
}

void Parse::setDecimalSymbolIndex(int index)
{
  m_decimalSymbolIndex = index;
}

void Parse::thousandsSeparatorChanged(int val)
{
  m_thousandsSeparatorIndex = val;
  m_thousandsSeparator = m_thousandsSeparatorList[val];
  if (m_thousandsSeparator == KGlobal::locale()->thousandsSeparator()) {
    return;
  }
}

QString Parse::thousandsSeparator()
{
  return m_thousandsSeparator;
}

void Parse::setThousandsSeparator(int index)
{
  m_thousandsSeparator = m_thousandsSeparatorList[index];
}

void Parse::setThousandsSeparatorIndex(int index)
{
  m_thousandsSeparatorIndex = index;
}

int Parse::lastLine()
{
  return m_lastLine;
}

bool Parse::symbolFound()
{
  return m_symbolFound;
}

void Parse::setSymbolFound(bool found)
{
  m_symbolFound = found;
}

QString Parse::possiblyReplaceSymbol(const QString&  str)
{
  m_symbolFound = false;
  m_invalidConversion = false;

  QString txt = str.trimmed();
  if (txt.isEmpty()) {
    return txt;  //                            exit as no decimal symbol
  }
  if (txt.contains('(')) {     //              "(" or "Af" = debit
    txt = txt.remove(QRegExp("[()]"));
    txt = '-' + txt;
  }
  int decimalIndex = txt.indexOf(m_decimalSymbol, 0);
  int length = txt.length();
  int thouIndex = txt.lastIndexOf(m_thousandsSeparator, -1);

  //  Check if this col/cell contains decimal symbol

  if (decimalIndex == -1) {     //             selected decimal not found
    m_symbolFound = false;
    if ((thouIndex == -1) || (thouIndex == length - 4))  {      //no separator || correct format
      txt.remove(m_thousandsSeparator);
      QString tmp = txt + KGlobal::locale()->decimalSymbol() + "00";
      return tmp;
    } else
      m_invalidConversion = true;
    return txt;
  }

  txt.remove(m_thousandsSeparator);  //    remove unwanted old thousands separator
  //  Found decimal

  m_symbolFound = true;  //                            found genuine decimal

  if (thouIndex >= 0) {      //                        there was a separator
    if (decimalIndex < thouIndex) {      //            invalid conversion
      m_invalidConversion = true;
    }
    if (decimalIndex == length - 1) {   //             ...decimal point with no decimal part (strange?)
      txt += m_decimalSymbol + "00";
    }
  }//  thouIndex = -1                            no thousands separator

  //  m_symbolFound = true                      found genuine decimal
  txt.replace(m_decimalSymbol, KGlobal::locale()->decimalSymbol());  // so swap it
  return txt;
}

bool Parse::invalidConversion()
{
  return m_invalidConversion;
}

//--------------------------------------------------------------------------------------------------------------------------------

CsvUtil::CsvUtil() :
    m_investTransactionEditor(0),
    m_scannedCategories(false)
{
}

CsvUtil::~CsvUtil()
{
}

const QString& CsvUtil::feeId(const MyMoneyAccount& invAcc)
{
  scanCategories(m_feeId, invAcc, MyMoneyFile::instance()->expense(), i18n("_Fees"));
  return m_feeId;
}

const QString& CsvUtil::interestId(const MyMoneyAccount& invAcc)
{
  scanCategories(m_interestId, invAcc, MyMoneyFile::instance()->income(), i18n("_Dividend"));
  return m_interestId;
}

QString CsvUtil::nameToId(const QString& name, MyMoneyAccount& parent)
{
  //  Adapted from KMyMoneyApp::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
  //  Needed to find/create category:sub-categories
  MyMoneyFile* file = MyMoneyFile::instance();

  QString id = file->categoryToAccount(name, MyMoneyAccount::UnknownAccountType);
  // if it does not exist, we have to create it
  if (id.isEmpty()) {
    MyMoneyAccount newAccount;
    MyMoneyAccount parentAccount = parent;
    newAccount.setName(name) ;
    int pos;
    // check for ':' in the name and use it as separator for a hierarchy
    while ((pos = newAccount.name().indexOf(MyMoneyFile::AccountSeparator)) != -1) {
      QString part = newAccount.name().left(pos);
      QString remainder = newAccount.name().mid(pos + 1);
      const MyMoneyAccount& existingAccount = file->subAccountByName(parentAccount, part);
      if (existingAccount.id().isEmpty()) {
        newAccount.setName(part);
        newAccount.setAccountType(parentAccount.accountType());
        file->addAccount(newAccount, parentAccount);
        parentAccount = newAccount;
      } else {
        parentAccount = existingAccount;
      }
      newAccount.setParentAccountId(QString());  // make sure, there's no parent
      newAccount.clearId();                       // and no id set for adding
      newAccount.removeAccountIds();              // and no sub-account ids
      newAccount.setName(remainder);
    }//end while
    newAccount.setAccountType(parentAccount.accountType());

    // make sure we have a currency. If none is assigned, we assume base currency
    if (newAccount.currencyId().isEmpty())
      newAccount.setCurrencyId(file->baseCurrency().id());

    file->addAccount(newAccount, parentAccount);
    id = newAccount.id();
  }
  return id;
}

QString CsvUtil::expenseId(const QString& name)
{
  MyMoneyAccount parent = MyMoneyFile::instance()->expense();
  return nameToId(name, parent);
}

QString CsvUtil::interestId(const QString& name)
{
  MyMoneyAccount parent = MyMoneyFile::instance()->income();
  return nameToId(name, parent);
}

QString CsvUtil::feeId(const QString& name)
{
  MyMoneyAccount parent = MyMoneyFile::instance()->expense();
  return nameToId(name, parent);
}


void CsvUtil::scanCategories(QString& id, const MyMoneyAccount& invAcc, const MyMoneyAccount& parentAccount, const QString& defaultName)
{
  if (!m_scannedCategories) {
    previouslyUsedCategories(invAcc.id(), m_feeId, m_interestId);
    m_scannedCategories = true;
  }

  if (id.isEmpty()) {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount acc = file->accountByName(defaultName);
    // if it does not exist, we have to create it
    if (acc.id().isEmpty()) {
      MyMoneyAccount parent = parentAccount;
      acc.setName(defaultName);
      acc.setAccountType(parent.accountType());
      acc.setCurrencyId(parent.currencyId());
      file->addAccount(acc, parent);
    }
    id = acc.id();
  }
}

void CsvUtil::previouslyUsedCategories(const QString& investmentAccount, QString& feesId, QString& interestId)
{
  feesId.clear();
  interestId.clear();
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    MyMoneyAccount acc = file->account(investmentAccount);
    MyMoneyTransactionFilter filter(investmentAccount);
    filter.setReportAllSplits(false);
    // since we assume an investment account here, we need to collect the stock accounts as well
    filter.addAccount(acc.accountList());
    QList< QPair<MyMoneyTransaction, MyMoneySplit> > list;
    file->transactionList(list, filter);
    QList< QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it_t;
    for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      const MyMoneyTransaction& t = (*it_t).first;
      const MyMoneySplit&s = (*it_t).second;
      MyMoneySplit assetAccountSplit;
      QList<MyMoneySplit> feeSplits;
      QList<MyMoneySplit> interestSplits;
      MyMoneySecurity security;
      MyMoneySecurity currency;
      MyMoneySplit::investTransactionTypeE transactionType;
      dissectTransaction(t, s, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
      if (feeSplits.count() == 1) {
        feesId = feeSplits.first().accountId();
      }
      if (interestSplits.count() == 1) {
        interestId = interestSplits.first().accountId();
      }
    }
  } catch (const MyMoneyException &) {
  }
}


void CsvUtil::dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, MyMoneySplit::investTransactionTypeE& transactionType)
{
  // collect the splits. split references the stock account and should already
  // be set up. assetAccountSplit references the corresponding asset account (maybe
  // empty), feeSplits is the list of all expenses and interestSplits
  // the list of all incomes
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    MyMoneyAccount acc = file->account((*it_s).accountId());
    if ((*it_s).id() == split.id()) {
      security = file->security(acc.currencyId());
    } else if (acc.accountGroup() == MyMoneyAccount::Expense) {
      feeSplits.append(*it_s);
    } else if (acc.accountGroup() == MyMoneyAccount::Income) {
      interestSplits.append(*it_s);
    } else {
      assetAccountSplit = *it_s;
    }
  }

  // determine transaction type
  if (split.action() == MyMoneySplit::ActionAddShares) {
    transactionType = (!split.shares().isNegative()) ? MyMoneySplit::AddShares : MyMoneySplit::RemoveShares;
  } else if (split.action() == MyMoneySplit::ActionBuyShares) {
    transactionType = (!split.value().isNegative()) ? MyMoneySplit::BuyShares : MyMoneySplit::SellShares;
  } else if (split.action() == MyMoneySplit::ActionDividend) {
    transactionType = MyMoneySplit::Dividend;
  } else if (split.action() == MyMoneySplit::ActionReinvestDividend) {
    transactionType = MyMoneySplit::ReinvestDividend;
  } else if (split.action() == MyMoneySplit::ActionYield) {
    transactionType = MyMoneySplit::Yield;
  } else if (split.action() == MyMoneySplit::ActionSplitShares) {
    transactionType = MyMoneySplit::SplitShares;
  } else if (split.action() == MyMoneySplit::ActionInterestIncome) {
    transactionType = MyMoneySplit::InterestIncome;
  } else
    transactionType = MyMoneySplit::BuyShares;

  currency.setTradingSymbol("???");
  try {
    currency = file->security(transaction.commodity());
  } catch (const MyMoneyException &) {
  }
}


const QString CsvUtil::checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2)
{
  //  Borrowed from MyMoneyQifReader::checkCategory()
  QString accountId;
  MyMoneyFile *file = MyMoneyFile::instance();
  MyMoneyAccount account;
  bool found = true;

  if (!name.isEmpty()) {
    // The category might be constructed with an arbitraty depth (number of
    // colon delimited fields). We try to find a parent account within this
    // hierarchy by searching the following sequence:
    //
    //    aaaa:bbbb:cccc:ddddd
    //
    // 1. search aaaa:bbbb:cccc:dddd, create nothing
    // 2. search aaaa:bbbb:cccc     , create dddd
    // 3. search aaaa:bbbb          , create cccc:dddd
    // 4. search aaaa               , create bbbb:cccc:dddd
    // 5. don't search              , create aaaa:bbbb:cccc:dddd

    account.setName(name);
    QString accName;      // part to be created (right side in above list)
    QString parent(name);    // a possible parent part (left side in above list)
    do {
      accountId = file->categoryToAccount(parent);
      if (accountId.isEmpty()) {
        found = false;
        // prepare next step
        if (!accName.isEmpty())
          accName.prepend(':');
        accName.prepend(parent.section(':', -1));
        account.setName(accName);
        parent = parent.section(':', 0, -2);
      } else if (!accName.isEmpty()) {
        account.setParentAccountId(accountId);
      }
    } while (!parent.isEmpty() && accountId.isEmpty());

    // if we did not find the category, we create it
    if (!found) {
      MyMoneyAccount parent;
      if (account.parentAccountId().isEmpty()) {
        if (!value.isNegative() && value2.isNegative())
          parent = file->income();
        else
          parent = file->expense();
      } else {
        parent = file->account(account.parentAccountId());
      }
      account.setAccountType((!value.isNegative() && value2.isNegative()) ? MyMoneyAccount::Income : MyMoneyAccount::Expense);
      MyMoneyAccount brokerage;
      // clear out the parent id, because createAccount() does not like that
      account.setParentAccountId(QString());
      createAccount(account, parent, brokerage, MyMoneyMoney());
      accountId = account.id();
    }
  }

  return accountId;
}


void CsvUtil::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // make sure we have a currency. If none is assigned, we assume base currency
  if (newAccount.currencyId().isEmpty())
    newAccount.setCurrencyId(file->baseCurrency().id());

  MyMoneyFileTransaction ft;
  try {
    int pos;
    // check for ':' in the name and use it as separator for a hierarchy
    while ((pos = newAccount.name().indexOf(MyMoneyFile::AccountSeparator)) != -1) {
      QString part = newAccount.name().left(pos);
      QString remainder = newAccount.name().mid(pos + 1);
      const MyMoneyAccount& existingAccount = file->subAccountByName(parentAccount, part);
      if (existingAccount.id().isEmpty()) {
        newAccount.setName(part);

        file->addAccount(newAccount, parentAccount);
        parentAccount = newAccount;
      } else {
        parentAccount = existingAccount;
      }
      newAccount.setParentAccountId(QString());  // make sure, there's no parent
      newAccount.clearId();                       // and no id set for adding
      newAccount.removeAccountIds();              // and no sub-account ids
      newAccount.setName(remainder);
    }

    const MyMoneySecurity& sec = file->security(newAccount.currencyId());
    // Check the opening balance
    if (openingBal.isPositive() && newAccount.accountGroup() == MyMoneyAccount::Liability) {
      QString message = i18n("This account is a liability and if the "
                             "opening balance represents money owed, then it should be negative.  "
                             "Negate the amount?\n\n"
                             "Please click Yes to change the opening balance to %1,\n"
                             "Please click No to leave the amount as %2,\n"
                             "Please click Cancel to abort the account creation."
                             , MyMoneyUtils::formatMoney(-openingBal, newAccount, sec)
                             , MyMoneyUtils::formatMoney(openingBal, newAccount, sec));

      int ans = KMessageBox::questionYesNoCancel(0, message);
      if (ans == KMessageBox::Yes) {
        openingBal = -openingBal;

      } else if (ans == KMessageBox::Cancel)
        return;
    }

    file->addAccount(newAccount, parentAccount);

    if (newAccount.accountType() == MyMoneyAccount::Investment
        && !brokerageAccount.name().isEmpty()) {
      file->addAccount(brokerageAccount, parentAccount);

      // set a link from the investment account to the brokerage account
      file->modifyAccount(newAccount);
      file->createOpeningBalanceTransaction(brokerageAccount, openingBal);

    } else
      file->createOpeningBalanceTransaction(newAccount, openingBal);

    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::information(0, i18n("Unable to add account: %1", e.what()));
  }
}

