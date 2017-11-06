/***************************************************************************
                          kselecttransactionsdlg_p.h
                             -------------------
    begin                : Wed May 16 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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
