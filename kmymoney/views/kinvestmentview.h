/*
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
  Q_DECL_DEPRECATED void updateActions(const MyMoneyObject &obj);

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
