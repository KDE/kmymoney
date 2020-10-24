/*
 * Copyright 2002-2004  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2003-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004-2005  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@kde.org>
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


#ifndef KINVESTMENTVIEW_H
#define KINVESTMENTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class MyMoneyAccount;
class MyMoneyObject;
class SelectedObjects;

/**
  * @author Kevin Tambascio
  * @author Łukasz Wojniłowicz
  */
class KInvestmentViewPrivate;
class KInvestmentView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KInvestmentView(QWidget *parent = nullptr);
  ~KInvestmentView() override;

  void executeCustomAction(eView::Action action) override;
  void setDefaultFocus();
  void refresh();
  void updateActions(const MyMoneyObject &obj) Q_DECL_DEPRECATED;

public Q_SLOTS:
  /**
    * This slot is used to preselect investment account from ledger view
    */
  void slotSelectAccount(const MyMoneyObject &obj);

  void slotShowInvestmentMenu(const MyMoneyAccount& acc);
  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;

  void updateActions(const SelectedObjects& selections) override;

protected:
  void showEvent(QShowEvent* event) override;

private:
  Q_DECLARE_PRIVATE(KInvestmentView)

private Q_SLOTS:
  /**
    * This slot is used to reload (filters + equities account) specific tab
    */
  void slotLoadTab(int index);

  void slotEquitySelected(const QModelIndex &current, const QModelIndex &previous);
  void slotSecuritySelected(const QModelIndex &current, const QModelIndex &previous);

  void slotNewInvestment();
  void slotEditInvestment();
  void slotDeleteInvestment();
  void slotUpdatePriceOnline();
  void slotUpdatePriceManually();

  void slotEditSecurity();
  void slotDeleteSecurity();

  /**
    * This slot is used to programatically preselect account in investment view
    */
  void slotSelectAccount(const QString &id);

  /**
    * This slot is used to load investment account into tree view
    */
  void slotLoadAccount(const QString &id);

  void slotInvestmentMenuRequested(const QPoint&);
};

#endif
