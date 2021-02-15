/*
 * SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
