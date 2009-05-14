/***************************************************************************
                          kmymoneyfileinfodlg.cpp  -  description
                             -------------------
    begin                : Sun Oct 9 2005
    copyright            : (C) 2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <qpushbutton.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyfileinfodlg.h"
#include <imymoneystorage.h>
#include <mymoneyfile.h>
#include <kmymoneyutils.h>

KMyMoneyFileInfoDlg::KMyMoneyFileInfoDlg(QWidget *parent)
 : KMyMoneyFileInfoDlgDecl(parent)
{
  // Hide the unused buttons.
  buttonCancel->hide();
  buttonHelp->hide();

  // Now fill the fields with data
  IMyMoneyStorage* storage = MyMoneyFile::instance()->storage();

  m_creationDate->setText(storage->creationDate().toString(Qt::ISODate));
  m_lastModificationDate->setText(storage->lastModificationDate().toString(Qt::ISODate));
  m_baseCurrency->setText(storage->value("kmm-baseCurrency"));

  m_payeeCount->setText(QString("%1").arg(storage->payeeList().count()));
  m_institutionCount->setText(QString("%1").arg(storage->institutionList().count()));

  Q3ValueList<MyMoneyAccount> a_list;
  storage->accountList(a_list);
  m_accountCount->setText(QString("%1").arg(a_list.count()));

  QMap<MyMoneyAccount::accountTypeE, int> accountMap;
  QMap<MyMoneyAccount::accountTypeE, int> accountMapClosed;
  Q3ValueList<MyMoneyAccount>::const_iterator it_a;
  for(it_a = a_list.begin(); it_a != a_list.end(); ++it_a) {
    accountMap[(*it_a).accountType()] = accountMap[(*it_a).accountType()] + 1;
    accountMapClosed[(*it_a).accountType()] = accountMapClosed[(*it_a).accountType()] + 0;
    if((*it_a).isClosed())
      accountMapClosed[(*it_a).accountType()] = accountMapClosed[(*it_a).accountType()] + 1;
  }

  QMap<MyMoneyAccount::accountTypeE, int>::const_iterator it_m;
  for(it_m = accountMap.begin(); it_m != accountMap.end(); ++it_m) {
    new K3ListViewItem(m_accountView, KMyMoneyUtils::accountTypeToString(it_m.key()), QString("%1").arg(*it_m), QString("%1").arg(accountMapClosed[it_m.key()]));
  }


  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  m_transactionCount->setText(QString("%1").arg(storage->transactionList(filter).count()));
  filter.setReportAllSplits(true);
  m_splitCount->setText(QString("%1").arg(storage->transactionList(filter).count()));
  m_scheduleCount->setText(QString("%1").arg(storage->scheduleList().count()));
  MyMoneyPriceList list = storage->priceList();
  MyMoneyPriceList::const_iterator it_p;
  int pCount = 0;
  for(it_p = list.begin(); it_p != list.end(); ++it_p)
    pCount += (*it_p).count();
  m_priceCount->setText(QString("%1").arg(pCount));
}

KMyMoneyFileInfoDlg::~KMyMoneyFileInfoDlg()
{
}

#include "kmymoneyfileinfodlg.moc"
