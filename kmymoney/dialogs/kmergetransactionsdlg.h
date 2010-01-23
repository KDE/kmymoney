//Added by qt3to4:
#include <QEvent>
#include <QResizeEvent>
/***************************************************************************
                          kmergetransactionsdlg.h
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

#ifndef KMERGETRANSACTIONSDLG_H
#define KMERGETRANSACTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

class QResizeEvent;

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <register.h>
#include <mymoneyaccount.h>

#include "kselecttransactionsdlg.h"


class KMergeTransactionsDlg: public KSelectTransactionsDlg
{
  Q_OBJECT
public:
  explicit KMergeTransactionsDlg(const MyMoneyAccount& account, QWidget* parent = 0);

  bool eventFilter(QObject* , QEvent*) {
    return false;
  }

public slots:
  void slotHelp();
};

#endif // KMERGETRANSACTIONSDLG_H
// vim:cin:si:ai:et:ts=2:sw=2:
