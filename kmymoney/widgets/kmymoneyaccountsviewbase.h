/*
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
