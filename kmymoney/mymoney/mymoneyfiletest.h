/***************************************************************************
                          mymoneyfiletest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYFILETEST_H
#define MYMONEYFILETEST_H

#include <QtCore/QObject>
#include "autotest.h"

#define private public
#define protected public
#include <QList>
#include "mymoneyfile.h"
#include "storage/mymoneyseqaccessmgr.h"
#undef private
#undef protected

class MyMoneyFileTest : public QObject
{
  Q_OBJECT
protected:
  MyMoneyFile *m;
  MyMoneySeqAccessMgr* storage;
  MyMoneyAccount  m_inv;

private slots:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testAddOneInstitution();
  void testAddTwoInstitutions();
  void testRemoveInstitution();
  void testInstitutionRetrieval();
  void testInstitutionListRetrieval();
  void testInstitutionModify();
  void testSetFunctions();
  void testAddAccounts();
  void testModifyAccount();
  void testModifyStdAccount();
  void testReparentAccount();
  void testRemoveAccount();
  void testRemoveAccountTree();
  void testAccountListRetrieval();
  void testAddTransaction();
  void testIsStandardAccount();
  void testHasActiveSplits();
  void testModifyTransactionSimple();
  void testModifyTransactionNewPostDate();
  void testModifyTransactionNewAccount();
  void testRemoveTransaction();
  void testBalanceTotal();
  void testSetAccountName();
  void testAddPayee();
  void testModifyPayee();
  void testRemovePayee();
  void testAddTransactionStd();
  void testAttachStorage();
  void testAccount2Category();
  void testCategory2Account();
  void testAttachedStorage();
  void testHasAccount();
  void testAddEquityAccount();
  void testReparentEquity();
  void testReparentEquity(QList<MyMoneyAccount::accountTypeE>& list, MyMoneyAccount& parent);
  void testBaseCurrency();
  void testOpeningBalanceNoBase();
  void testOpeningBalance();

private:
  void testRemoveStdAccount(const MyMoneyAccount& acc);
};

#endif
