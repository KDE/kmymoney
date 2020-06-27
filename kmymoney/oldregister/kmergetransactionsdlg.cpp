/*
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
