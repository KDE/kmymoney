/*
 * SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
