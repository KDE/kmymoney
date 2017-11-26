/***************************************************************************
                             kaccountssview.h
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef KACCOUNTSVIEW_H
#define KACCOUNTSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountsviewbase.h"

/**
  * This class implements the accounts hierarchical and iconic 'view'.
  */
class MyMoneyMoney;
class KAccountsViewPrivate;
class KAccountsView : public KMyMoneyAccountsViewBase
{
  Q_OBJECT

public:
  explicit KAccountsView(QWidget *parent = nullptr);
  ~KAccountsView();

  void setDefaultFocus() override;
  void refresh() override;

public Q_SLOTS:
  void slotNetWorthChanged(const MyMoneyMoney &);

protected:
  KAccountsView(KAccountsViewPrivate &dd, QWidget *parent);
  void showEvent(QShowEvent * event) override;

protected Q_SLOTS:
  void slotUnusedIncomeExpenseAccountHidden();

private:
  Q_DECLARE_PRIVATE(KAccountsView)
};

#endif
