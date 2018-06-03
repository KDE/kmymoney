/*
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2005-2017  Thomas Baumgart <tbaumgart@kde.org>
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
  * their file  without comprimising their financial data.  If a user
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

  MyMoneyMoney m_factor;
};

#endif
