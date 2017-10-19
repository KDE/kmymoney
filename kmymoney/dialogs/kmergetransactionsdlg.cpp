/***************************************************************************
                          kmergetransactionsdlg.cpp
                             -------------------
    begin                : Sun Aug 20 2006
    copyright            : (C) 2006 by Ace Jones
    email                : <acejones@users.sf.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmergetransactionsdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"

KMergeTransactionsDlg::KMergeTransactionsDlg(const MyMoneyAccount& account, QWidget* parent) :
    KSelectTransactionsDlg(account, parent)
{

  // setup descriptive texts
  setWindowTitle(i18n("Merge Transactions"));
  m_description->setText(i18n("Are you sure you wish to merge these transactions?"));

  // no selection possible
  m_register->setSelectionMode(QTableWidget::NoSelection);

  // override default and enable ok button right away
  buttonOk->setEnabled(true);
}

void KMergeTransactionsDlg::slotHelp()
{
  KHelpClient::invokeHelp("details.ledgers.match");
}
