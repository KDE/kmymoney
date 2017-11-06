/***************************************************************************
                          kmergetransactionsdlg.h
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

#ifndef KMERGETRANSACTIONSDLG_H
#define KMERGETRANSACTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kselecttransactionsdlg.h"

class MyMoneyAccount;
class KMergeTransactionsDlg: public KSelectTransactionsDlg
{
  Q_OBJECT
public:
  explicit KMergeTransactionsDlg(const MyMoneyAccount& account, QWidget* parent = nullptr);

  bool eventFilter(QObject* , QEvent*) override;

public slots:
  void slotHelp() override;
};

#endif // KMERGETRANSACTIONSDLG_H
