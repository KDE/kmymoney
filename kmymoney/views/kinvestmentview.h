/***************************************************************************
                          kinvestmentview.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

  void setDefaultFocus() override;
  void refresh() override;
  void updateActions(const MyMoneyObject &obj) override;

public Q_SLOTS:
  /**
    * This slot is used to preselect investment account from ledger view
    */
  void slotSelectAccount(const MyMoneyObject &obj);

  void slotShowInvestmentMenu(const MyMoneyAccount& acc);

Q_SIGNALS:
  void accountSelected(const MyMoneyObject&);
  void objectSelected(const MyMoneyObject&);
  void contextMenuRequested(const MyMoneyObject& obj);

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
