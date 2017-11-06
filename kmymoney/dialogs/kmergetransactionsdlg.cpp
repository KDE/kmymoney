/***************************************************************************
                          kmergetransactionsdlg.cpp
                             -------------------
    begin                : Sun Aug 20 2006
    copyright            : (C) 2006 by Ace Jones
    email                : <acejones@users.sf.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
#include "kselecttransactionsdlg_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kselecttransactionsdlg.h"

#include "register.h"

KMergeTransactionsDlg::KMergeTransactionsDlg(const MyMoneyAccount& account, QWidget* parent) :
    KSelectTransactionsDlg(account, parent)
{
  // setup descriptive texts
  setWindowTitle(i18n("Merge Transactions"));
  d_ptr->ui->m_description->setText(i18n("Are you sure you wish to merge these transactions?"));

  // no selection possible
  d_ptr->ui->m_register->setSelectionMode(QTableWidget::NoSelection);

  // override default and enable ok button right away
  d_ptr->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

bool KMergeTransactionsDlg::eventFilter(QObject* , QEvent*)
{
  return false;
}

void KMergeTransactionsDlg::slotHelp()
{
  KHelpClient::invokeHelp("details.ledgers.match");
}
