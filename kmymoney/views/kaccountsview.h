/***************************************************************************
                             kaccountssview.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include <QListWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyutils.h>
#include "accountsmodel.h"

#include "ui_kaccountsviewdecl.h"


/**
  * This class implements the accounts hierarchical and iconic 'view'.
  */
class KAccountsView : public QWidget, public Ui::KAccountsViewDecl
{
  Q_OBJECT

public:
  KAccountsView(QWidget *parent = 0);
  virtual ~KAccountsView();

public slots:
  void slotLoadAccounts();

protected:
  enum accountViewRole {
    reconcileRole = Qt::UserRole + 1
  };

  void loadIconGroups();

protected slots:
  void slotNetWorthChanged(const MyMoneyMoney &);
  void slotOpenContextMenu(MyMoneyAccount account);
  void slotOpenObject(QListWidgetItem* item);
  void slotExpandCollapse();
  void slotUnusedIncomeExpenseAccountHidden();

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTreeView::selectObject()
    *
    * @param obj const reference to object
    */
  void selectObject(const MyMoneyObject& obj);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTreeView::openContextMenu(const MyMoneyObject&)
    *
    * @param obj const reference to object
    */
  void openContextMenu(const MyMoneyObject& obj);

  /**
    * This signal will be emitted when the left mouse button is double
    * clicked (actually the KDE executed setting is used) on an object.
    *
    * @param obj const reference to object
    */
  void openObject(const MyMoneyObject& obj);

  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

private:
  bool                                m_haveUnusedCategories;

  AccountsViewFilterProxyModel        *m_filterProxyModel;
};

#endif
