/***************************************************************************
                          mymoneystatementreader.cpp
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneystatementreader.h"
#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Headers

#include <QFile>
#include <QStringList>
#include <QTimer>
#include <QLabel>
#include <QList>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>

// ----------------------------------------------------------------------------
// Project Headers

#include <mymoneyfile.h>
#include <mymoneystatement.h>
#include <kmymoneyglobalsettings.h>
#include <transactioneditor.h>
#include <kmymoneyedit.h>
#include "kaccountselectdlg.h"
#include "transactionmatcher.h"
#include "kenterscheduledlg.h"
#include "kmymoney.h"
#include <kmymoneyaccountcombo.h>

class MyMoneyStatementReader::Private
{
  public:
    Private() :
      transactionsCount(0),
      transactionsAdded(0),
      transactionsMatched(0),
      transactionsDuplicate(0),
      scannedCategories(false)
    {}

    const QString& feeId(const MyMoneyAccount& invAcc);
    const QString& interestId(const MyMoneyAccount& invAcc);
    QString interestId(const QString& name);
    QString feeId(const QString& name);
    void assignUniqueBankID(MyMoneySplit& s, const MyMoneyStatement::Transaction& t_in);

    MyMoneyAccount                 lastAccount;
    QList<MyMoneyTransaction> transactions;
    QList<MyMoneyPayee>       payees;
    int                            transactionsCount;
    int                            transactionsAdded;
    int                            transactionsMatched;
    int                            transactionsDuplicate;
    QMap<QString, bool>            uniqIds;
    QMap<QString, MyMoneySecurity> securitiesBySymbol;
    QMap<QString, MyMoneySecurity> securitiesByName;
    bool                           m_skipCategoryMatching;
  private:
    void scanCategories(QString& id, const MyMoneyAccount& invAcc, const MyMoneyAccount& parentAccount, const QString& defaultName);
    QString nameToId(const QString&name, MyMoneyAccount& parent);
  private:
    QString                        m_feeId;
    QString                        m_interestId;
    bool                           scannedCategories;
};


const QString& MyMoneyStatementReader::Private::feeId(const MyMoneyAccount& invAcc)
{
  scanCategories(m_feeId, invAcc, MyMoneyFile::instance()->expense(), i18n("_Fees"));
  return m_feeId;
}

const QString& MyMoneyStatementReader::Private::interestId(const MyMoneyAccount& invAcc)
{
  scanCategories(m_interestId, invAcc, MyMoneyFile::instance()->income(), i18n("_Dividend"));
  return m_interestId;
}

QString MyMoneyStatementReader::Private::nameToId(const QString&name, MyMoneyAccount& parent)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc = file->accountByName(name);
    // if it does not exist, we have to create it
  if(acc.id().isEmpty()) {
    acc.setName( name );
    acc.setAccountType( parent.accountType() );
    acc.setCurrencyId(parent.currencyId());
    file->addAccount(acc, parent);
  }
  return acc.id();
}

QString MyMoneyStatementReader::Private::interestId(const QString& name)
{
  MyMoneyAccount parent = MyMoneyFile::instance()->income();
  return nameToId(name, parent);
}

QString MyMoneyStatementReader::Private::feeId(const QString& name)
{
  MyMoneyAccount parent = MyMoneyFile::instance()->expense();
  return nameToId(name, parent);
}


void MyMoneyStatementReader::Private::scanCategories(QString& id, const MyMoneyAccount& invAcc, const MyMoneyAccount& parentAccount, const QString& defaultName)
{
  if(!scannedCategories) {
    KMyMoneyUtils::previouslyUsedCategories(invAcc.id(), m_feeId, m_interestId);
    scannedCategories = true;
  }

  if(id.isEmpty()) {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount acc = file->accountByName(defaultName);
    // if it does not exist, we have to create it
    if(acc.id().isEmpty()) {
      MyMoneyAccount parent = parentAccount;
      acc.setName( defaultName );
      acc.setAccountType( parent.accountType() );
      acc.setCurrencyId(parent.currencyId());
      file->addAccount(acc, parent);
    }
    id = acc.id();
  }
}

void MyMoneyStatementReader::Private::assignUniqueBankID(MyMoneySplit& s, const MyMoneyStatement::Transaction& t_in)
{
  if( ! t_in.m_strBankID.isEmpty() ) {
    // make sure that id's are unique from this point on by appending a -#
    // postfix if needed
    QString base(t_in.m_strBankID);
    QString hash(base);
    int idx = 1;
    for(;;) {
      QMap<QString, bool>::const_iterator it;
      it = uniqIds.constFind(hash);
      if(it == uniqIds.constEnd()) {
        uniqIds[hash] = true;
        break;
      }
      hash = QString("%1-%2").arg(base).arg(idx);
      ++idx;
    }

    s.setBankID(hash);
  }
}


MyMoneyStatementReader::MyMoneyStatementReader() :
  d(new Private),
  m_userAbort(false),
  m_autoCreatePayee(false),
  m_ft(0),
  m_progressCallback(0)
{
  m_askPayeeCategory = KMyMoneyGlobalSettings::askForPayeeCategory();
}

MyMoneyStatementReader::~MyMoneyStatementReader()
{
  delete d;
}

void MyMoneyStatementReader::setAutoCreatePayee(bool create)
{
  m_autoCreatePayee = create;
}

void MyMoneyStatementReader::setAskPayeeCategory(bool ask)
{
  m_askPayeeCategory = ask;
}

bool MyMoneyStatementReader::import(const MyMoneyStatement& s, QStringList& messages)
{
  //
  // For testing, save the statement to an XML file
  // (uncomment this line)
  //
  //MyMoneyStatement::writeXMLFile(s,"Imported.Xml");

  //
  // Select the account
  //

  m_account = MyMoneyAccount();

  m_ft = new MyMoneyFileTransaction();
  d->m_skipCategoryMatching = s.m_skipCategoryMatching;

  // if the statement source left some information about
  // the account, we use it to get the current data of it
  if(!s.m_accountId.isEmpty()) {
    try {
      m_account = MyMoneyFile::instance()->account(s.m_accountId);
    } catch(MyMoneyException* e) {
      qDebug("Received reference '%s' to unknown account in statement", qPrintable(s.m_accountId));
      delete e;
    }
  }

  if(m_account.id().isEmpty())
  {
    m_account.setName(s.m_strAccountName);
    m_account.setNumber(s.m_strAccountNumber);

    switch ( s.m_eType )
    {
      case MyMoneyStatement::etCheckings:
        m_account.setAccountType(MyMoneyAccount::Checkings);
        break;
      case MyMoneyStatement::etSavings:
        m_account.setAccountType(MyMoneyAccount::Savings);
        break;
      case MyMoneyStatement::etInvestment:
        //testing support for investment statements!
        //m_userAbort = true;
        //KMessageBox::error(kmymoney2, i18n("This is an investment statement.  These are not supported currently."), i18n("Critical Error"));
        m_account.setAccountType(MyMoneyAccount::Investment);
        break;
      case MyMoneyStatement::etCreditCard:
        m_account.setAccountType(MyMoneyAccount::CreditCard);
        break;
      default:
        m_account.setAccountType(MyMoneyAccount::Checkings);
        break;
    }


    // we ask the user only if we have some transactions to process
    if ( !m_userAbort && s.m_listTransactions.count() > 0)
      m_userAbort = ! selectOrCreateAccount(Select, m_account);
  }

  // see if we need to update some values stored with the account
  if(m_account.value("lastStatementBalance") != s.m_closingBalance.toString()
  || m_account.value("lastImportedTransactionDate") != s.m_dateEnd.toString(Qt::ISODate)) {
    if(s.m_closingBalance != MyMoneyMoney::autoCalc) {
      m_account.setValue("lastStatementBalance", s.m_closingBalance.toString());
      if ( s.m_dateEnd.isValid() ) {
        m_account.setValue("lastImportedTransactionDate", s.m_dateEnd.toString(Qt::ISODate));
      }
    }

    try {
      MyMoneyFile::instance()->modifyAccount(m_account);
    } catch(MyMoneyException* e) {
      qDebug("Updating account in MyMoneyStatementReader::startImport failed");
      delete e;
    }
  }


  if(!m_account.name().isEmpty())
    messages += i18n("Importing statement for account %1",m_account.name());
  else if(s.m_listTransactions.count() == 0)
    messages += i18n("Importing statement without transactions");

  qDebug("Importing statement for '%s'", qPrintable(m_account.name()));

  //
  // Process the securities
  //
  signalProgress(0, s.m_listSecurities.count(), "Importing Statement ...");
  int progress = 0;
  QList<MyMoneyStatement::Security>::const_iterator it_s = s.m_listSecurities.begin();
  while ( it_s != s.m_listSecurities.end() )
  {
    processSecurityEntry(*it_s);
    signalProgress(++progress, 0);
    ++it_s;
  }
  signalProgress(-1, -1);

  //
  // Process the transactions
  //

  if ( !m_userAbort )
  {
    try {
      qDebug("Processing transactions (%s)", qPrintable(m_account.name()));
      signalProgress(0, s.m_listTransactions.count(), "Importing Statement ...");
      int progress = 0;
      QList<MyMoneyStatement::Transaction>::const_iterator it_t = s.m_listTransactions.begin();
      while ( it_t != s.m_listTransactions.end() )
      {
        processTransactionEntry(*it_t);
        signalProgress(++progress, 0);
        ++it_t;
      }
      qDebug("Processing transactions done (%s)", qPrintable(m_account.name()));

    } catch(MyMoneyException* e) {
      if(e->what() == "USERABORT")
        m_userAbort = true;
      else
        qDebug("Caught exception from processTransactionEntry() not caused by USERABORT: %s", qPrintable(e->what()));
      delete e;
    }
    signalProgress(-1, -1);
  }

  //
  // process price entries
  //
  if ( !m_userAbort )
  {
    try {
      signalProgress(0, s.m_listPrices.count(), "Importing Statement ...");
      QList<MyMoneySecurity> slist = MyMoneyFile::instance()->securityList();
      QList<MyMoneySecurity>::const_iterator it_s;
      for(it_s = slist.constBegin(); it_s != slist.constEnd(); ++it_s) {
        d->securitiesBySymbol[(*it_s).tradingSymbol()] = *it_s;
        d->securitiesByName[(*it_s).name()] = *it_s;
      }

      int progress = 0;
      QList<MyMoneyStatement::Price>::const_iterator it_p = s.m_listPrices.begin();
      while(it_p != s.m_listPrices.end()) {
        processPriceEntry(*it_p);
        signalProgress(++progress, 0);
        ++it_p;
      }
    } catch(MyMoneyException* e) {
      if(e->what() == "USERABORT")
        m_userAbort = true;
      else
        qDebug("Caught exception from processPriceEntry() not caused by USERABORT: %s", qPrintable(e->what()));
      delete e;
    }
    signalProgress(-1, -1);
  }

  bool  rc = false;

  // delete all payees created in vain
  int payeeCount = d->payees.count();
  QList<MyMoneyPayee>::const_iterator it_p;
  for(it_p = d->payees.constBegin(); it_p != d->payees.constEnd(); ++it_p) {
    try {
      MyMoneyFile::instance()->removePayee(*it_p);
      --payeeCount;
    } catch(MyMoneyException* e) {
      // if we can't delete it, it must be in use which is ok for us
      delete e;
    }
  }

  if(s.m_closingBalance.isAutoCalc()) {
    messages += i18n("  Statement balance is not contained in statement.");
  } else {
    messages += i18n("  Statement balance on %1 is reported to be %2",s.m_dateEnd.toString(Qt::ISODate),s.m_closingBalance.formatMoney("",2));
  }
  messages += i18n("  Transactions");
  messages += i18n("    %1 processed",d->transactionsCount);
  messages += i18nc("x transactions have been added", "    %1 added",d->transactionsAdded);
  messages += i18n("    %1 matched",d->transactionsMatched);
  messages += i18n("    %1 duplicates",d->transactionsDuplicate);
  messages += i18n("  Payees");
  messages += i18nc("x transactions have been created", "    %1 created",payeeCount);
  messages += QString();

  // remove the Don't ask again entries
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group(QString::fromLatin1("Notification Messages"));
  QStringList::ConstIterator it;

  for(it = m_dontAskAgain.constBegin(); it != m_dontAskAgain.constEnd(); ++it) {
    grp.deleteEntry(*it);
  }
  config->sync();
  m_dontAskAgain.clear();

  rc = !m_userAbort;

  // finish the transaction
  if(rc)
    m_ft->commit();
  delete m_ft;
  m_ft = 0;

  qDebug("Importing statement for '%s' done", qPrintable(m_account.name()));

  return rc;
}

void MyMoneyStatementReader::processPriceEntry(const MyMoneyStatement::Price& p_in)
{
  if(d->securitiesBySymbol.contains(p_in.m_strSecurity)) {

    MyMoneyPrice price(d->securitiesBySymbol[p_in.m_strSecurity].id(),
                       MyMoneyFile::instance()->baseCurrency().id(),
                       p_in.m_date,
                       p_in.m_amount, "QIF");
    MyMoneyFile::instance()->addPrice(price);

  } else if(d->securitiesByName.contains(p_in.m_strSecurity)) {

    MyMoneyPrice price(d->securitiesByName[p_in.m_strSecurity].id(),
                       MyMoneyFile::instance()->baseCurrency().id(),
                                             p_in.m_date,
                                             p_in.m_amount, "QIF");
    MyMoneyFile::instance()->addPrice(price);
  }

}

void MyMoneyStatementReader::processSecurityEntry(const MyMoneyStatement::Security& sec_in)
{
  // For a security entry, we will just make sure the security exists in the
  // file. It will not get added to the investment account until it's called
  // for in a transaction.
  MyMoneyFile* file = MyMoneyFile::instance();

  // check if we already have the security
  // In a statement, we do not know what type of security this is, so we will
  // not use type as a matching factor.
  MyMoneySecurity security;
  QList<MyMoneySecurity> list = file->securityList();
  QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
  while ( it != list.constEnd() && security.id().isEmpty() )
  {
    if(sec_in.m_strSymbol.isEmpty()) {
      if((*it).name() == sec_in.m_strName)
        security = *it;
    } else if((*it).tradingSymbol() == sec_in.m_strSymbol)
      security = *it;
    ++it;
  }

  // if the security was not found, we have to create it while not forgetting
  // to setup the type
  if(security.id().isEmpty())
  {
    security.setName(sec_in.m_strName);
    security.setTradingSymbol(sec_in.m_strSymbol);
    security.setSmallestAccountFraction(1000);
    security.setTradingCurrency(file->baseCurrency().id());
    security.setValue("kmm-security-id", sec_in.m_strId);
    security.setValue("kmm-online-source", "Yahoo");
    security.setSecurityType(MyMoneySecurity::SECURITY_STOCK);
    MyMoneyFileTransaction ft;
    try {
      file->addSecurity(security);
      ft.commit();
      kDebug(0) << "Created " << security.name() << " with id " << security.id();
    } catch(MyMoneyException *e) {
      KMessageBox::error(0, i18n("Error creating security record: %1",e->what()), i18n("Error"));
    }
  } else {
    kDebug(0) << "Found " << security.name() << " with id " << security.id();
  }
}

void MyMoneyStatementReader::processTransactionEntry(const MyMoneyStatement::Transaction& t_in)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyTransaction t;

#if 0
  QString dbgMsg;
  dbgMsg = QString("Process %1, '%3', %2").arg(t_in.m_datePosted.toString(Qt::ISODate)).arg(t_in.m_amount.formatMoney("", 2)).arg(t_in.m_strBankID);
  qDebug("%s", dbgMsg.data());
#endif

  // mark it imported for the view
  t.setImported();

  // TODO (Ace) We can get the commodity from the statement!!
  // Although then we would need UI to verify
  t.setCommodity(m_account.currencyId());

  t.setPostDate(t_in.m_datePosted);
  t.setMemo(t_in.m_strMemo);

#if 0
  // (acejones) removing this code.  keeping it around for reference.
  //
  // this is the OLD way of handling bank ID's, which unfortunately was wrong.
  // bank ID's actually need to go on the split which corresponds with the
  // account we're importing into.
  //
  // thus anywhere "this account" is put into a split is also where we need
  // to put the bank ID in.
  //
  if ( ! t_in.m_strBankID.isEmpty() )
    t.setBankID(t_in.m_strBankID);
#endif

  MyMoneySplit s1;

  s1.setMemo(t_in.m_strMemo);
  s1.setValue(t_in.m_amount - t_in.m_fees);
  s1.setShares(s1.value());
  s1.setNumber(t_in.m_strNumber);

  // set these values if a transfer split is needed at the very end.
  MyMoneyMoney transfervalue;

  // If the user has chosen to import into an investment account, determine the correct account to use
  MyMoneyAccount thisaccount = m_account;
  QString brokerageactid;

  if ( thisaccount.accountType() == MyMoneyAccount::Investment )
  {
    // determine the brokerage account
    brokerageactid = m_account.value("kmm-brokerage-account").toUtf8();
    if (brokerageactid.isEmpty() )
    {
      brokerageactid = file->accountByName(m_account.brokerageName()).id();
    }

    // find the security transacted, UNLESS this transaction didn't
    // involve any security.
    if ( (t_in.m_eAction != MyMoneyStatement::Transaction::eaNone)
      && (t_in.m_eAction != MyMoneyStatement::Transaction::eaInterest)
      && (t_in.m_eAction != MyMoneyStatement::Transaction::eaFees))
    {
      // the correct account is the stock account which matches two criteria:
      // (1) it is a sub-account of the selected investment account, and
      // (2a) the symbol of the underlying security matches the security of the
      // transaction, or
      // (2b) the name of the security matches the name of the security of the transaction.

      // search through each subordinate account
      bool found = false;
      QStringList accounts = thisaccount.accountList();
      QStringList::const_iterator it_account = accounts.constBegin();
      while( !found && it_account != accounts.constEnd() )
      {
        QString currencyid = file->account(*it_account).currencyId();
        MyMoneySecurity security = file->security( currencyid );
        if((t_in.m_strSymbol.toLower() == security.tradingSymbol().toLower())
        || (t_in.m_strSecurity.toLower() == security.name().toLower()))
        {
          thisaccount = file->account(*it_account);
          found = true;

          // Don't update price if there is no price information contained in the transaction
          if(t_in.m_eAction != MyMoneyStatement::Transaction::eaCashDividend
          && t_in.m_eAction != MyMoneyStatement::Transaction::eaShrsin
          && t_in.m_eAction != MyMoneyStatement::Transaction::eaShrsout)
          {
            // update the price, while we're here.  in the future, this should be
            // an option
            QString basecurrencyid = file->baseCurrency().id();
            MyMoneyPrice price = file->price( currencyid, basecurrencyid, t_in.m_datePosted, true );
            if ( !price.isValid()  && ((!t_in.m_amount.isZero() && !t_in.m_shares.isZero()) || !t_in.m_price.isZero()))
            {
              MyMoneyPrice newprice;
              if(!t_in.m_price.isZero()) {
                newprice = MyMoneyPrice( currencyid, basecurrencyid, t_in.m_datePosted,
                                         t_in.m_price.abs(), i18n("Statement Importer") );
              } else {
                newprice = MyMoneyPrice( currencyid, basecurrencyid, t_in.m_datePosted,
                  (t_in.m_amount / t_in.m_shares).abs(), i18n("Statement Importer") );
              }
              file->addPrice(newprice);
            }
          }
        }

        ++it_account;
      }

      // If there was no stock account under the m_acccount investment account,
      // add one using the security.
      if (!found)
      {
        // The security should always be available, because the statement file
        // should separately list all the securities referred to in the file,
        // and when we found a security, we added it to the file.

        if ( t_in.m_strSecurity.isEmpty() )
        {
          KMessageBox::information(0, i18n("This imported statement contains investment transactions with no security.  These transactions will be ignored."),i18n("Security not found"),QString("BlankSecurity"));
          return;
        }
        else
        {
          MyMoneySecurity security;
          QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
          QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
          while ( it != list.constEnd() && security.id().isEmpty() )
          {
            if(t_in.m_strSecurity.toLower() == (*it).tradingSymbol().toLower()
            || t_in.m_strSecurity.toLower() == (*it).name().toLower()) {
              security = *it;
            }
            ++it;
          }
          if(!security.id().isEmpty())
          {
            thisaccount = MyMoneyAccount();
            thisaccount.setName(security.name());
            thisaccount.setAccountType(MyMoneyAccount::Stock);
            thisaccount.setCurrencyId(security.id());

            file->addAccount(thisaccount, m_account);
            kDebug(0) << Q_FUNC_INFO << ": created account " << thisaccount.id() << " for security " << t_in.m_strSecurity << " under account " << m_account.id();
          }
          // this security does not exist in the file.
          else
          {
            // This should be rare.  A statement should have a security entry for any
            // of the securities referred to in the transactions.  The only way to get
            // here is if that's NOT the case.
            KMessageBox::information(0, i18n("This investment account does not contain the \"%1\" security.  Transactions involving this security will be ignored.",t_in.m_strSecurity),i18n("Security not found"),QString("MissingSecurity%1").arg(t_in.m_strSecurity.trimmed()));
            return;
          }
        }
      }
    }

    s1.setAccountId(thisaccount.id());
    d->assignUniqueBankID(s1, t_in);

    if (t_in.m_eAction==MyMoneyStatement::Transaction::eaReinvestDividend)
    {
      s1.setAction(MyMoneySplit::ActionReinvestDividend);
      s1.setShares(t_in.m_shares);

      if(!t_in.m_price.isZero()) {
        s1.setPrice(t_in.m_price);
      } else {
        s1.setPrice(((t_in.m_amount - t_in.m_fees) / t_in.m_shares).convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())));
      }


      MyMoneySplit s2;
      s2.setMemo(t_in.m_strMemo);
      if(t_in.m_strInterestCategory.isEmpty())
        s2.setAccountId(d->interestId(thisaccount));
      else
        s2.setAccountId(d->interestId(t_in.m_strInterestCategory));

      s2.setShares(-t_in.m_amount - t_in.m_fees);
      s2.setValue(s2.shares());
      t.addSplit(s2);
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaCashDividend)
    {
      // Cash dividends require setting 2 splits to get all of the information
      // in.  Split #1 will be the income split, and we'll set it to the first
      // income account.  This is a hack, but it's needed in order to get the
      // amount into the transaction.

      // There are some sign issues.  The OFX plugin universally reverses the sign
      // for investment transactions.
      //
      // The way we interpret the sign on 'amount' is the s1 split, which is always
      // the thing that's NOT the cash account.  For dividends, it's the income
      // category, for buy/sell it's the stock account.
      //
      // For cash account transactions, the s1 split IS the cash account split,
      // which explains why they have to be reversed for investment transactions
      //
      // Ergo, the 'amount' is negative at this point and needs to stay negative.
      // The 'fees' is positive.
      //
      // This should probably change.  It would be more consistent to ALWAYS
      // interpret the 'amount' as the cash account part.

      if(t_in.m_strInterestCategory.isEmpty())
        s1.setAccountId(d->interestId(thisaccount));
      else
        s1.setAccountId(d->interestId(t_in.m_strInterestCategory));
      s1.setShares(t_in.m_amount);
      s1.setValue(t_in.m_amount);

      // Split 2 will be the zero-amount investment split that serves to
      // mark this transaction as a cash dividend and note which stock account
      // it belongs to.
      MyMoneySplit s2;
      s2.setMemo(t_in.m_strMemo);
      s2.setAction(MyMoneySplit::ActionDividend);
      s2.setAccountId(thisaccount.id());
      t.addSplit(s2);

      transfervalue = -t_in.m_amount - t_in.m_fees;
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaInterest)
    {
      if(t_in.m_strInterestCategory.isEmpty())
        s1.setAccountId(d->interestId(thisaccount));
      else
        s1.setAccountId(d->interestId(t_in.m_strInterestCategory));
      s1.setShares(t_in.m_amount);
      s1.setValue(t_in.m_amount);

      transfervalue = -t_in.m_amount;

    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaFees)
    {
      if(t_in.m_strInterestCategory.isEmpty())
        s1.setAccountId(d->feeId(thisaccount));
      else
        s1.setAccountId(d->feeId(t_in.m_strInterestCategory));
      s1.setShares(t_in.m_amount);
      s1.setValue(t_in.m_amount);

      transfervalue = -t_in.m_amount;

    }
    else if ((t_in.m_eAction==MyMoneyStatement::Transaction::eaBuy ) ||
             (t_in.m_eAction==MyMoneyStatement::Transaction::eaSell))
    {
      if(!t_in.m_price.isZero()) {
        s1.setPrice(t_in.m_price.abs());
      } else {
        MyMoneyMoney total;
        total = t_in.m_amount - t_in.m_fees;
        if(!t_in.m_shares.isZero())
          s1.setPrice((total / t_in.m_shares).abs().convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision())));
      }

      s1.setAction(MyMoneySplit::ActionBuyShares);

      // Make sure to setup the sign correctly
      if(t_in.m_eAction==MyMoneyStatement::Transaction::eaBuy ) {
        s1.setShares(t_in.m_shares.abs());
        s1.setValue(s1.value().abs());
        transfervalue = -(t_in.m_amount.abs());
      } else {
        s1.setShares(-(t_in.m_shares.abs()));
        s1.setValue(-(s1.value().abs()));
        transfervalue = t_in.m_amount.abs();
      }

    }
    else if ((t_in.m_eAction==MyMoneyStatement::Transaction::eaShrsin) ||
              (t_in.m_eAction==MyMoneyStatement::Transaction::eaShrsout))
    {
      s1.setValue(MyMoneyMoney());
      s1.setShares(t_in.m_shares);
      s1.setAction(MyMoneySplit::ActionAddShares);
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaNone)
    {
      // User is attempting to import a non-investment transaction into this
      // investment account.  This is not supportable the way KMyMoney is
      // written.  However, if a user has an associated brokerage account,
      // we can stuff the transaction there.

      QString brokerageactid = m_account.value("kmm-brokerage-account").toUtf8();
      if (brokerageactid.isEmpty() )
      {
        brokerageactid = file->accountByName(m_account.brokerageName()).id();
      }
      if ( ! brokerageactid.isEmpty() )
      {
        s1.setAccountId(brokerageactid);
        d->assignUniqueBankID(s1, t_in);

        // Needed to satisfy the bankid check below.
        thisaccount = file->account(brokerageactid);
      }
      else
      {
        // Warning!! Your transaction is being thrown away.
      }
    }
    if ( !t_in.m_fees.isZero() )
    {
      MyMoneySplit s;
      s.setMemo(i18n("(Fees) ") + t_in.m_strMemo);
      s.setValue(t_in.m_fees);
      s.setShares(t_in.m_fees);
      s.setAccountId(d->feeId(thisaccount));
      t.addSplit(s);
    }
  }
  else
  {
    // For non-investment accounts, just use the selected account
    // Note that it is perfectly reasonable to import an investment statement into a non-investment account
    // if you really want.  The investment-specific information, such as number of shares and action will
    // be discarded in that case.
    s1.setAccountId(m_account.id());
    d->assignUniqueBankID(s1, t_in);
  }


  QString payeename = t_in.m_strPayee;
  if(!payeename.isEmpty())
  {
    QString payeeid;
    try {
      QList<MyMoneyPayee> pList = file->payeeList();
      QList<MyMoneyPayee>::const_iterator it_p;
      QMap<int, QString> matchMap;
      for(it_p = pList.constBegin(); it_p != pList.constEnd(); ++it_p) {
        bool ignoreCase;
        QStringList keys;
        QStringList::const_iterator it_s;
        switch((*it_p).matchData(ignoreCase, keys)) {
          case MyMoneyPayee::matchDisabled:
            break;

          case MyMoneyPayee::matchName:
            keys << QString("%1").arg(QRegExp::escape((*it_p).name()));
            // tricky fall through here

          case MyMoneyPayee::matchKey:
            for(it_s = keys.constBegin(); it_s != keys.constEnd(); ++it_s) {
              QRegExp exp(*it_s, ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive);
              if(exp.indexIn(payeename) != -1) {
                matchMap[exp.matchedLength()] = (*it_p).id();
              }
            }
            break;
        }
      }

      // at this point we can have several scenarios:
      // a) multiple matches
      // b) a single match
      // c) no match at all
      //
      // for c) we just do nothing, for b) we take the one we found
      // in case of a) we take the one with the largest matchedLength()
      // which happens to be the last one in the map
      if(matchMap.count() > 1) {
        QMap<int, QString>::const_iterator it_m = matchMap.constEnd();
        --it_m;
        payeeid = *it_m;
      } else if(matchMap.count() == 1)
        payeeid = *(matchMap.constBegin());

      // if we did not find a matching payee, we throw an exception and try to create it
      if(payeeid.isEmpty())
        throw new MYMONEYEXCEPTION("payee not matched");

      s1.setPayeeId(payeeid);
    }
    catch (MyMoneyException *e)
    {
      MyMoneyPayee payee;
      int rc = KMessageBox::Yes;

      if(m_autoCreatePayee == false) {
        // Ask the user if that is what he intended to do?
        QString msg = i18n("Do you want to add \"%1\" as payee/receiver?\n\n",payeename);
        msg += i18n("Selecting \"Yes\" will create the payee, \"No\" will skip "
                    "creation of a payee record and remove the payee information "
                    "from this transaction. Selecting \"Cancel\" aborts the import "
                    "operation.\n\nIf you select \"No\" here and mark the \"Don't ask "
                    "again\" checkbox, the payee information for all following transactions "
                    "referencing \"%1\" will be removed.",payeename);

        QString askKey = QString("Statement-Import-Payee-")+payeename;
        if(!m_dontAskAgain.contains(askKey)) {
          m_dontAskAgain += askKey;
        }
        rc = KMessageBox::questionYesNoCancel(0, msg, i18n("New payee/receiver"),
                  KStandardGuiItem::yes(), KStandardGuiItem::no(), KStandardGuiItem::cancel(),askKey);
      }
      delete e;

      if(rc == KMessageBox::Yes) {
        // for now, we just add the payee to the pool and turn
        // on simple name matching, so that future transactions
        // with the same name don't get here again.
        //
        // In the future, we could open a dialog and ask for
        // all the other attributes of the payee, but since this
        // is called in the context of an automatic procedure it
        // might distract the user.
        payee.setName(payeename);
        payee.setMatchData(MyMoneyPayee::matchName, true, QStringList());
        if (m_askPayeeCategory) {
          // We use a QPointer because the dialog may get deleted
          // during exec() if the parent of the dialog gets deleted.
          // In that case the guarded ptr will reset to 0.
          QPointer<KDialog> dialog = new KDialog(0);
          dialog->setCaption(i18n("Default Category for Payee"));
          dialog->setButtons(KDialog::Yes | KDialog::No | KDialog::Cancel);
          dialog->setModal(true);
          dialog->setButtonGuiItem(KDialog::Yes, KGuiItem(i18n("Save Category")));
          dialog->setButtonGuiItem(KDialog::No, KGuiItem(i18n("No Category")));
          dialog->setButtonGuiItem(KDialog::Cancel, KGuiItem(i18n("Abort")));

          QVBoxLayout *topcontents = new QVBoxLayout (dialog);
          topcontents->setSpacing(KDialog::spacingHint()*2);
          topcontents->setMargin(KDialog::marginHint());

          //add in caption? and account combo here
          QLabel *label1 = new QLabel(i18n("Please select a default category for payee '%1':",payee.name()));
          topcontents->addWidget(label1);

          QPointer<KMyMoneyAccountCombo> accountCombo = new KMyMoneyAccountCombo(dialog);
          dialog->setLayout(topcontents);

          int result = dialog->exec();

          QString accountId;
          if (accountCombo && !accountCombo->selectedAccounts().isEmpty()) {
            accountId = accountCombo->selectedAccounts().front();
          }
          if (dialog) {
            delete dialog;
          }
          //if they hit yes instead of no, then grab setting of account combo
          if (result == KDialog::Yes) {
            payee.setDefaultAccountId(accountId);
          }
          else if (result != KDialog::No) {
            //add cancel button? and throw exception like below
            throw new MYMONEYEXCEPTION("USERABORT");
          }
        }

        try {
          file->addPayee(payee);
          qDebug("Payee '%s' created", qPrintable(payee.name()));
          d->payees << payee;
          payeeid = payee.id();
          s1.setPayeeId(payeeid);

        } catch(MyMoneyException *e) {
          KMessageBox::detailedSorry(0, i18n("Unable to add payee/receiver"),
            (e->what() + ' ' + i18n("thrown in") + ' ' + e->file()+ ":%1").arg(e->line()));
          delete e;

        }

      } else if(rc == KMessageBox::No) {
        s1.setPayeeId(QString());

      } else {
        throw new MYMONEYEXCEPTION("USERABORT");

      }
    }

    if(thisaccount.accountType() != MyMoneyAccount::Stock ) {
      //
      // Fill in other side of the transaction (category/etc) based on payee
      //
      // Note, this logic is lifted from KLedgerView::slotPayeeChanged(),
      // however this case is more complicated, because we have an amount and
      // a memo.  We just don't have the other side of the transaction.
      //
      // We'll search for the most recent transaction in this account with
      // this payee.  If this reference transaction is a simple 2-split
      // transaction, it's simple.  If it's a complex split, and the amounts
      // are different, we have a problem.  Somehow we have to balance the
      // transaction.  For now, we'll leave it unbalanced, and let the user
      // handle it.
      //
      const MyMoneyPayee& payeeObj = MyMoneyFile::instance()->payee(payeeid);
      if (t_in.m_listSplits.isEmpty() && payeeObj.defaultAccountEnabled()) {
        MyMoneySplit s;
        s.setReconcileFlag(MyMoneySplit::Cleared);
        s.clearId();
        s.setBankID(QString());
        s.setShares(-s1.shares());
        s.setValue(-s1.value());
        s.setAccountId(payeeObj.defaultAccountId());
        t.addSplit(s);
      }
      else if (t_in.m_listSplits.isEmpty() && !d->m_skipCategoryMatching) {
        MyMoneyTransactionFilter filter(thisaccount.id());
        filter.addPayee(payeeid);
        QList<MyMoneyTransaction> list = file->transactionList(filter);
        if(!list.empty())
        {
          // Default to using the most recent transaction as the reference
          MyMoneyTransaction t_old = list.last();

          // if there is more than one matching transaction, try to be a little
          // smart about which one we take.  for now, we'll see if there's one
          // with the same VALUE as our imported transaction, and if so take that one.
          if ( list.count() > 1 )
          {
            QList<MyMoneyTransaction>::ConstIterator it_trans = list.constEnd();
            if(it_trans != list.constBegin())
              --it_trans;
            while ( it_trans != list.constBegin() )
            {
              MyMoneySplit s = (*it_trans).splitByAccount(thisaccount.id());
              if ( s.value() == s1.value() )
              {
                t_old = *it_trans;
                break;
              }
              --it_trans;
            }
            // check constBegin, just in case
            if (it_trans == list.constBegin()) {
              MyMoneySplit s = (*it_trans).splitByAccount(thisaccount.id());
              if ( s.value() == s1.value() )
              {
                t_old = *it_trans;
              }
            }
          }

          QList<MyMoneySplit>::ConstIterator it_split;
          for(it_split = t_old.splits().constBegin(); it_split != t_old.splits().constEnd(); ++it_split)
          {
            // We don't need the split that covers this account,
            // we just need the other ones.
            if ( (*it_split).accountId() != thisaccount.id() )
            {
              MyMoneySplit s(*it_split);
              s.setReconcileFlag(MyMoneySplit::NotReconciled);
              s.clearId();
              s.setBankID(QString());

              if ( t_old.splits().count() == 2 )
              {
                s.setShares(-s1.shares());
                s.setValue(-s1.value());
                s.setMemo(s1.memo());
              }
              t.addSplit(s);
            }
          }
        }
      }
    }
  }

  s1.setReconcileFlag(t_in.m_reconcile);
  t.addSplit(s1);

  // Add the 'account' split if it's needed
  if ( ! transfervalue.isZero() )
  {
    // in case the transaction has a reference to the brokerage account, we use it
    if(!t_in.m_strBrokerageAccount.isEmpty()) {
      brokerageactid = file->accountByName(t_in.m_strBrokerageAccount).id();
    }

    if ( !brokerageactid.isEmpty() )
    {
      // FIXME This may not deal with foreign currencies properly
      MyMoneySplit s;
      s.setMemo(t_in.m_strMemo);
      s.setValue(transfervalue);
      s.setShares(transfervalue);
      s.setAccountId(brokerageactid);
      s.setReconcileFlag(t_in.m_reconcile);
      t.addSplit(s);
    }
  }

  if ((t_in.m_eAction != MyMoneyStatement::Transaction::eaReinvestDividend) &&   (t_in.m_eAction!=MyMoneyStatement::Transaction::eaCashDividend)
     )
  {
  //******************************************
  //                   process splits
  //******************************************

    QList<MyMoneyStatement::Split>::const_iterator it_s;
    for(it_s = t_in.m_listSplits.begin(); it_s != t_in.m_listSplits.end(); ++it_s) {
      MyMoneySplit s2;
      s2.setAccountId((*it_s).m_accountId);
      MyMoneyAccount acc = file->account(s2.accountId());
      if(acc.isAssetLiability()) {
        s2.setPayeeId(s1.payeeId());
      }
      s2.setMemo((*it_s).m_strMemo);
      s2.setShares((*it_s).m_amount);
      s2.setValue((*it_s).m_amount);
      s2.setReconcileFlag((*it_s).m_reconcile);
      t.addSplit(s2);
    }

#if 0
    QString accountId;
    int   count;
    int cnt = 0;
    count = t_in.m_listSplits.count();

    for(cnt = 0; cnt < count; ++cnt )
    {
      MyMoneySplit s2 = s1;
      s2.setMemo(t_in.m_listSplits[cnt].m_strMemo);
      s2.clearId();
      s2.setValue(t_in.m_listSplits[cnt].m_amount);
      s2.setShares(t_in.m_listSplits[cnt].m_amount);
      s2.setAccountId(QString(t_in.m_listSplits[cnt].m_accountId));
#if 0
      accountId = file->nameToAccount(t_in.m_listSplits[cnt].m_strCategoryName);
      if (accountId.isEmpty())
        accountId = checkCategory(t_in.m_listSplits[cnt].m_strCategoryName, t_in.m_listSplits[0].m_amount, t_in.m_listSplits[cnt].m_amount);

      s2.setAccountId(accountId);
#endif
      t.addSplit(s2);
    }
#endif
  }

  // Add the transaction
  try {

    // check for matches already stored in the engine
    MyMoneySplit matchedSplit;
    TransactionMatcher::autoMatchResultE result;
    TransactionMatcher matcher(thisaccount);
    matcher.setMatchWindow(KMyMoneyGlobalSettings::matchInterval());
    const MyMoneyObject *o = matcher.findMatch(t, s1, matchedSplit, result);
    d->transactionsCount++;

    // if we did not already find this one, we need to process it
    if(result != TransactionMatcher::matchedDuplicate) {
      d->transactionsAdded++;
      file->addTransaction(t);

      if(o) {
        if(typeid(*o) == typeid(MyMoneyTransaction)) {
          // it matched a simple transaction. that's the easy case
          MyMoneyTransaction tm(*(dynamic_cast<const MyMoneyTransaction*>(o)));
          switch(result) {
            case TransactionMatcher::notMatched:
            case TransactionMatcher::matchedDuplicate:
              // no need to do anything here
              break;
            case TransactionMatcher::matched:
            case TransactionMatcher::matchedExact:
              qDebug("Detected as match to transaction '%s'", qPrintable(tm.id()));
              matcher.match(tm, matchedSplit, t, s1, true);
              d->transactionsMatched++;
              break;
          }

        } else if(typeid(*o) == typeid(MyMoneySchedule)) {
          // a match has been found in a pending schedule. We'll ask the user if she wants
          // to enter the schedule and match it agains the new transaction. Otherwise, we
          // just leave the transaction as imported.
          MyMoneySchedule schedule(*(dynamic_cast<const MyMoneySchedule*>(o)));
          if(KMessageBox::questionYesNo(0, QString("<qt>%1</qt>").arg(i18n("KMyMoney has found a scheduled transaction named <b>%1</b> which matches an imported transaction. Do you want KMyMoney to enter this schedule now so that the transaction can be matched? ",schedule.name())), i18n("Schedule found")) == KMessageBox::Yes) {
            KEnterScheduleDlg dlg(0, schedule);
            TransactionEditor* editor = dlg.startEdit();
            if(editor) {
              MyMoneyTransaction torig;
              // in case the amounts of the scheduled transaction and the
              // imported transaction differ, we need to update the amount
              // using the transaction editor.
              if(matchedSplit.shares() != s1.shares() && !schedule.isFixed()) {
                // for now this only works with regular transactions and not
                // for investment transactions. As of this, we don't have
                // scheduled investment transactions anyway.
                StdTransactionEditor* se = dynamic_cast<StdTransactionEditor*>(editor);
                if(se) {
                  // the following call will update the amount field in the
                  // editor and also adjust a possible VAT assignment. Make
                  // sure to use only the absolute value of the amount, because
                  // the editor keeps the sign in a different position (deposit,
                  // withdrawal tab)
                  kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(se->haveWidget("amount"));
                  if(amount) {
                    amount->setValue(s1.shares().abs());
                    se->slotUpdateAmount(s1.shares().abs().toString());

                    // we also need to update the matchedSplit variable to
                    // have the modified share/value.
                    matchedSplit.setShares(s1.shares());
                    matchedSplit.setValue(s1.value());
                  }
                }
              }

              editor->createTransaction(torig, dlg.transaction(), dlg.transaction().splits().isEmpty() ? MyMoneySplit() : dlg.transaction().splits().front(), true);
              QString newId;
              if(editor->enterTransactions(newId, false, true)) {
                if(!newId.isEmpty()) {
                  torig = MyMoneyFile::instance()->transaction(newId);
                  schedule.setLastPayment(torig.postDate());
                }
                schedule.setNextDueDate(schedule.nextPayment(schedule.nextDueDate()));
                MyMoneyFile::instance()->modifySchedule(schedule);
              }

              // now match the two transactions
              matcher.match(torig, matchedSplit, t, s1);
              d->transactionsMatched++;
            }
            delete editor;
          }
        }
      }
    } else {
      d->transactionsDuplicate++;
      qDebug("Detected as duplicate");
    }
    delete o;
  } catch (MyMoneyException *e) {
    QString message(i18n("Problem adding or matching imported transaction with id '%1': %2",t_in.m_strBankID,e->what()));
    qDebug("%s", qPrintable(message));
    delete e;

    int result = KMessageBox::warningContinueCancel(0, message);
    if ( result == KMessageBox::Cancel )
      throw new MYMONEYEXCEPTION("USERABORT");
  }
}

bool MyMoneyStatementReader::selectOrCreateAccount(const SelectCreateMode /*mode*/, MyMoneyAccount& account)
{
  bool result = false;

  MyMoneyFile* file = MyMoneyFile::instance();

  QString accountId;

  // Try to find an existing account in the engine which matches this one.
  // There are two ways to be a "matching account".  The account number can
  // match the statement account OR the "StatementKey" property can match.
  // Either way, we'll update the "StatementKey" property for next time.

  QString accountNumber = account.number();
  if ( ! accountNumber.isEmpty() )
  {
    // Get a list of all accounts
    QList<MyMoneyAccount> accounts;
    file->accountList(accounts);

    // Iterate through them
    QList<MyMoneyAccount>::const_iterator it_account = accounts.constBegin();
    while ( it_account != accounts.constEnd() )
    {
      if (
         ( (*it_account).value("StatementKey") == accountNumber ) ||
          ( (*it_account).number() == accountNumber )
        )
        {
          MyMoneyAccount newAccount((*it_account).id(), account);
          account = newAccount;
          accountId = (*it_account).id();
          break;
        }

      ++it_account;
    }
  }

  QString msg = i18n("<b>You have downloaded a statement for the following account:</b><br/><br/>");
  msg += i18n(" - Account Name: %1",account.name()) + "<br/>";
  msg += i18n(" - Account Type: %1",KMyMoneyUtils::accountTypeToString(account.accountType())) + "<br/>";
  msg += i18n(" - Account Number: %1",account.number()) + "<br/>";
  msg += "<br/>";

  QString header;

  if(!account.name().isEmpty())
  {
    if(!accountId.isEmpty())
      msg += i18n("Do you want to import transactions to this account?");
    else
      msg += i18n("KMyMoney cannot determine which of your accounts to use.  You can "
                  "create a new account by pressing the <b>Create</b> button "
                  "or select another one manually from the selection box below.");
  }
  else
  {
    msg += i18n("No account information has been found in the selected statement file. "
               "Please select an account using the selection box in the dialog or "
               "create a new account by pressing the <b>Create</b> button.");
  }

  KMyMoneyUtils::categoryTypeE type = static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::asset|KMyMoneyUtils::liability);
  QPointer<KAccountSelectDlg> accountSelect = new KAccountSelectDlg(type, "StatementImport", kmymoney2);
  accountSelect->setHeader(i18n("Import transactions"));
  accountSelect->setDescription(msg);
  accountSelect->setAccount(account, accountId);
  accountSelect->setMode(false);
  accountSelect->showAbortButton(true);
  accountSelect->m_qifEntry->hide();
  QString accname;
  bool done = false;
  while ( !done )
  {
    if ( accountSelect->exec() == QDialog::Accepted && !accountSelect->selectedAccount().isEmpty() )
    {
      result = true;
      done = true;
      accountId = accountSelect->selectedAccount();
      account = file->account(accountId);
      if ( ! accountNumber.isEmpty() && account.value("StatementKey") != accountNumber )
      {
        account.setValue("StatementKey", accountNumber);
        MyMoneyFileTransaction ft;
        try {
          MyMoneyFile::instance()->modifyAccount(account);
          ft.commit();
          accname = account.name();
        } catch(MyMoneyException* e) {
          qDebug("Updating account in MyMoneyStatementReader::selectOrCreateAccount failed");
          delete e;
        }
      }
    }
    else
    {
      if(accountSelect->aborted())
        //throw new MYMONEYEXCEPTION("USERABORT");
        done = true;
      else
        KMessageBox::error(0, QString("<qt>%1</qt>").arg(i18n("You must select an account, create a new one, or press the <b>Abort</b> button.")));
    }
  }
  delete accountSelect;

  return result;
}

void MyMoneyStatementReader::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStatementReader::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}


#include "mymoneystatementreader.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
