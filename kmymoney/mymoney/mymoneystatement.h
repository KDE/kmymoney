/***************************************************************************
                          mymoneystatement.h
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

#ifndef MYMONEYSTATEMENT_H
#define MYMONEYSTATEMENT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QDate>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

class QDomElement;
class QDomDocument;

/**
Represents the electronic analog of the paper bank statement just like we used to get in the regular mail.  This class is designed to be easy to extend and easy to create with minimal dependencies.  So the header file should include as few project files as possible (preferably NONE).

@author ace jones
*/
class MyMoneyStatement
{
public:
  MyMoneyStatement() : m_closingBalance(MyMoneyMoney::autoCalc), m_eType(etNone), m_skipCategoryMatching(false) {}

  enum EType { etNone = 0, etCheckings, etSavings, etInvestment, etCreditCard, etEnd };

  class Split
  {
  public:
    Split() : m_reconcile(eMyMoney::Split::State::NotReconciled) {}
    QString      m_strCategoryName;
    QString      m_strMemo;
    QString      m_accountId;
    eMyMoney::Split::State m_reconcile;
    MyMoneyMoney m_amount;

  };

  class Transaction
  {
  public:
    Transaction() : m_reconcile(eMyMoney::Split::State::NotReconciled), m_eAction(eaNone) {}
    QDate m_datePosted;
    QString m_strPayee;
    QString m_strMemo;
    QString m_strNumber;
    QString m_strBankID;
    MyMoneyMoney m_amount;
    eMyMoney::Split::State m_reconcile;

    // the following members are only used for investment accounts (m_eType==etInvestment)
    // eaNone means the action, shares, and security can be ignored.
    enum EAction { eaNone = 0, eaBuy, eaSell, eaReinvestDividend, eaCashDividend, eaShrsin, eaShrsout, eaStksplit, eaFees, eaInterest, eaEnd };
    EAction m_eAction;
    MyMoneyMoney m_shares;
    MyMoneyMoney m_fees;
    MyMoneyMoney m_price;
    QString m_strInterestCategory;
    QString m_strBrokerageAccount;
    QString m_strSymbol;
    QString m_strSecurity;
    QList<Split> m_listSplits;
  };

  struct Price {
    QDate m_date;
    QString m_sourceName;
    QString m_strSecurity;
    QString m_strCurrency;
    MyMoneyMoney m_amount;
  };

  struct Security {
    QString m_strName;
    QString m_strSymbol;
    QString m_strId;
  };

  QString m_strAccountName;
  QString m_strAccountNumber;
  QString m_strRoutingNumber;

  /**
   * The statement provider's information for the statement reader how to find the
   * account. The provider usually leaves some value with a key unique to the provider in the KVP of the
   * MyMoneyAccount object when setting up the connection or at a later point in time.
   * Using the KMyMoneyPlugin::KMMStatementInterface::account() method it can retrieve the
   * MyMoneyAccount object for this key. The account id of that account should be returned
   * here. If no id is available, leave it empty.
   */
  QString m_accountId;

  QString m_strCurrency;
  QDate m_dateBegin;
  QDate m_dateEnd;
  MyMoneyMoney m_closingBalance;
  EType m_eType;

  QList<Transaction> m_listTransactions;
  QList<Price> m_listPrices;
  QList<Security> m_listSecurities;

  bool m_skipCategoryMatching;

  void write(QDomElement&, QDomDocument*) const;
  bool read(const QDomElement&);

  KMM_MYMONEY_EXPORT static bool isStatementFile(const QString&);
  KMM_MYMONEY_EXPORT static bool readXMLFile(MyMoneyStatement&, const QString&);
  KMM_MYMONEY_EXPORT static void writeXMLFile(const MyMoneyStatement&, const QString&);
};

/**
  * Make it possible to hold @ref MyMoneyStatement objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyStatement)

#endif
