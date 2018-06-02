/*
 * Copyright 2007-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KSELECTTRANSACTIONSDLG_P_H
#define KSELECTTRANSACTIONSDLG_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kselecttransactionsdlg.h"

#include "mymoneyaccount.h"

class KSelectTransactionsDlgPrivate
{
  Q_DISABLE_COPY(KSelectTransactionsDlgPrivate)

public:
  KSelectTransactionsDlgPrivate() :
    ui(new Ui::KSelectTransactionsDlg)
  {
  }

  ~KSelectTransactionsDlgPrivate()
  {
    delete ui;
  }

  Ui::KSelectTransactionsDlg  *ui;
 /**
   * The account in which the transactions are displayed
   */
  MyMoneyAccount m_account;
};

#endif // KMERGETRANSACTIONSDLG_H
