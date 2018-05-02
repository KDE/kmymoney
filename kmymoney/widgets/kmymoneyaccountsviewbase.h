/***************************************************************************
                          kmymoneyaccountsviewbase.h
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                               2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYACCOUNTSVIEWBASE_H
#define KMYMONEYACCOUNTSVIEWBASE_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include "kmymoneyviewbase.h"

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyAccountTreeView;
class AccountsViewProxyModel;

/**
  * This class is an abstract base class that all specific views
  * should be based on.
  */
class KMyMoneyAccountsViewBasePrivate;
class KMM_WIDGETS_EXPORT KMyMoneyAccountsViewBase : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KMyMoneyAccountsViewBase(QWidget* parent = nullptr);
  virtual ~KMyMoneyAccountsViewBase();

  AccountsViewProxyModel  *getProxyModel();
  KMyMoneyAccountTreeView *getTreeView();

protected:
  KMyMoneyAccountsViewBase(KMyMoneyAccountsViewBasePrivate &dd, QWidget *parent);

private:
  Q_DECLARE_PRIVATE(KMyMoneyAccountsViewBase)
};

#endif
