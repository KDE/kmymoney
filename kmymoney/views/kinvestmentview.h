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

#include <mymoneysecurity.h>
#include <mymoneyaccount.h>
#include "ui_kinvestmentviewdecl.h"
#include "kinvestmentlistitem.h"


/**
  * @author Kevin Tambascio
  */


class KInvestmentViewDecl : public QWidget, public Ui::KInvestmentViewDecl
{
public:
  KInvestmentViewDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class KInvestmentView : public KInvestmentViewDecl
{
  Q_OBJECT

public:
  KInvestmentView(QWidget *parent = 0);
  ~KInvestmentView();

  /**
    * Start reconciliation for the account in the current view
    */
  void reconcileAccount(void);

public slots:
  /**
    * This slot is used to reload all data from the MyMoneyFile engine.
    * All existing data in the view will be discarded.
    * Call this e.g. if a new file has been loaded.
    */
  void slotLoadView(void);

  /**
    * This slot is used to select the correct ledger view type for
    * the account specified by @p id. If @p transactionId is not
    * empty, then the respective transaction will be selected.
    *
    * @param accountId Internal id used for the account to show
    * @param transactionId Internal id used for the transaction to select
    * @param reconciliation if true, the account will be selected in
    *                       reconciliation mode. If false, it will
    *                       be selected in regular ledger mode.
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  bool slotSelectAccount(const QString& accountId, const QString& transactionId = QString(), const bool reconciliation = false);

  /**
    * This method is provided for convenience and acts as the method above.
    */
  bool slotSelectAccount(const MyMoneyObject& acc);

  void showEvent(QShowEvent* event);

protected:
  /**
    * This method reloads the account selection combo box of the
    * view with all asset and liability accounts from the engine.
    * If the account id of the current account held in @p m_accountId is
    * empty or if the referenced account does not exist in the engine,
    * the first account found in the list will be made the current account.
    */
  void loadAccounts(void);

  /**
    * clear the view
    */
  void clear(void);

  void loadView(void);

protected slots:
  /**
    * This slot receives the signal from the listview @c lv control that the context menu
    * was requested for @c item at @c point.
    */
  void slotListContextMenu(K3ListView* lv, Q3ListViewItem* item, const QPoint& point);

  void slotSelectionChanged(Q3ListViewItem *item);


signals:
  /**
    * This signal is emitted, if an account has been selected
    * which cannot handled by this view.
    */
  void accountSelected(const QString& accountId, const QString& transactionId);

  void accountSelected(const MyMoneyObject&);

  void investmentRightMouseClick(void);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif
