/*
 * SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2005-2017 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYMONEYSTORAGEANON_H
#define MYMONEYSTORAGEANON_H


// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragexml.h"
#include "mymoneymoney.h"

class MyMoneyKeyValueContainer;

/**
  * @author Kevin Tambascio (ktambascio@users.sourceforge.net)
  */

#define VERSION_0_60_XML  0x10000010    // Version 0.5 file version info
#define VERSION_0_61_XML  0x10000011    // use 8 bytes for MyMoneyMoney objects

/**
  * This class provides storage of an anonymized version of the current
  * file.  Any object with an ID (account, transaction, etc) is renamed
  * with that ID.  Any other string value the user typed in is replaced with
  * x's equal in length to the original string.  Any numeric value is
  * replaced with an arbitrary number which matches the sign of the original.
  *
  * The purpose of this class is to give users a way to send a developer
  * their file  without compromising their financial data.  If a user
  * encounters an error, they should try saving the anonymous version of the
  * file and see if the error is still there.  If so, they should notify the
  * list of the problem, and then when requested, send the anonymous file
  * privately to the developer who takes the problem.  I still don't think
  * it's wise to post the file to the public list...maybe I'm just paranoid.
  *
  * @author Ace Jones <ace.j@hotpop.com>
  */

class MyMoneyStorageANON : public MyMoneyStorageXML
{
public:
  MyMoneyStorageANON();
  virtual ~MyMoneyStorageANON();

protected:
  void writeUserInformation(QDomElement& userInfo) final override;

  void writeInstitution(QDomElement& institutions, const MyMoneyInstitution& i) final override;

  void writePayee(QDomElement& payees, const MyMoneyPayee& p) final override;

  void writeTag(QDomElement& tags, const MyMoneyTag& ta) final override;

  void writeAccount(QDomElement& accounts, const MyMoneyAccount& p) final override;

  void writeTransaction(QDomElement& transactions, const MyMoneyTransaction& tx) final override;

  void writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx) final override;

  void writeBudget(QDomElement& budgets, const MyMoneyBudget& b) final override;

  void writeReport(QDomElement& reports, const MyMoneyReport& r) final override;

  void readFile(QIODevice* s, MyMoneyStorageMgr* storage) final override;

  void writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security) final override;

  /** Cannot remove prive data from plugins, yet. It is simply doing nothing. */
  void writeOnlineJob(QDomElement& onlineJobs, const onlineJob& job) final override;

  QDomElement findChildElement(const QString& name, const QDomElement& root);

  void writeAccounts(QDomElement& accounts) final override;

private:
  /**
    * The list of key-value pairs to not modify
    */
  static QStringList zKvpNoModify;

  /**
    * The list of key-value pairs which are numbers to be hidden
    */
  static QStringList zKvpXNumber;

  QString hideString(const QString&) const;
  MyMoneyMoney hideNumber(const MyMoneyMoney&) const;
  void fakeTransaction(MyMoneyTransaction& tn);
  void fakeBudget(MyMoneyBudget& bn);
  void fakeKeyValuePair(MyMoneyKeyValueContainer& _kvp);

  MyMoneyMoney          m_factor;
  QList<MyMoneyAccount> m_accountList;
};

#endif
