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

#include "ui_kinvestmentview.h"

class KMyMoneyApp;
class KMyMoneyView;
class MyMoneyObject;
class MyMoneySecurity;

/**
  * @author Kevin Tambascio
  * @author Łukasz Wojniłowicz
  */
class KInvestmentView : public QWidget, private Ui::KInvestmentView
{
  Q_OBJECT

public:
  explicit KInvestmentView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview);
  ~KInvestmentView();

  void setDefaultFocus();

public Q_SLOTS:
  /**
    * This slot is used to reload all data from the MyMoneyFile engine.
    * All existing data in the view will be invalidated.
    * Call this e.g. if a new file has been loaded.
    */
  void slotLoadView();

  /**
    * This slot is used to preselect investment account from ledger view
    */
  void slotSelectAccount(const MyMoneyObject &obj);

  void showEvent(QShowEvent* event);

private Q_SLOTS:
  /**
    * This slot is used to reload (filters + equities account) specific tab
    */
  void slotLoadTab(int index);

  void slotEquitySelected(const QModelIndex &current, const QModelIndex &previous);
  void slotEquityRightClicked(const QPoint& point);
  void slotEquityDoubleClicked();
  void slotSecuritySelected(const QModelIndex &current, const QModelIndex &previous);

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

Q_SIGNALS:
  void accountSelected(const MyMoneyObject&);

  void equityRightClicked();

  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

private:
  class Private;
  Private* const d;

  KMyMoneyApp                         *m_kmymoney;
  KMyMoneyView                        *m_kmymoneyview;

  /** Initializes page and sets its load status to initialized
   */
  void init();

  /**
    * This slot is used to programatically preselect default account in investment view
    */
  void selectDefaultInvestmentAccount();
  enum Tab { Equities = 0, Securities };

  /**
    * This slots are used to reload tabs
    */
  void loadInvestmentTab();
  void loadSecuritiesTab();

  /**
    * This slots returns security currently selected in tree view
    */
  MyMoneySecurity currentSecurity();

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;
};

#endif
