/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
