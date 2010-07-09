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

// ----------------------------------------------------------------------------
// KDE Includes

#include <KListWidget>

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
  void slotLoadAccounts(void);

  /**
    * Override the base class behaviour to include all updates that
    * happened in the meantime and restore the layout.
    */
  void showEvent(QShowEvent * event);

  /**
    * update the account objects if their icon position has changed since
    * the last time.
    *
    * @param action must be KMyMoneyView::preSave, otherwise this slot is a NOP.
    */
  //void slotUpdateIconPos(unsigned int action);

  void slotReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);

protected:
  typedef enum {
    ListView = 0,
    IconView,
    // insert new values above this line
    MaxViewTabs
  } AccountsViewTab;

  enum accountViewRole {
    reconcileRole = Qt::UserRole + 1
  };

  /**
    * This method loads the accounts for the respective tab.
    *
    * @param tab which tab should be loaded
    */
  void loadAccounts(AccountsViewTab tab);
  void loadListView(void);
  void loadIconGroups(void);

  /**
    * This method loads all the subaccounts recursively of a given root account
    *
    */
  void loadAccountIconsIntoList(const MyMoneyAccount& parentAccount, KListWidget* listWidget);

  /**
    * This method returns a pointer to the currently selected
    * account icon or 0 if no icon is selected.
    */
  QListWidgetItem* selectedIcon(void) const;

protected slots:
  void slotNetWorthChanged(const MyMoneyMoney &);
  void slotTabCurrentChanged(QWidget*);
  void slotSelectIcon(QListWidgetItem* item);
  void slotOpenContextMenu(MyMoneyAccount account);
  void slotAssetsSelectIcon(void);
  void slotAssetsOpenContextMenu(const QPoint& point);
  void slotLiabilitiesSelectIcon(void);
  void slotLiabilitiesOpenContextMenu(const QPoint& point);
  void slotEquitiesSelectIcon(void);
  void slotEquitiesOpenContextMenu(const QPoint& point);
  void slotOpenObject(QListWidgetItem* item);
  void slotExpandCollapse(void);
  void slotUnusedIncomeExpenseAccountHidden(void);
  void slotReconcileAccount(KListWidget* list, const MyMoneyAccount& acc);

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

private:
  MyMoneyAccount                      m_reconciliationAccount;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload[MaxViewTabs];
  bool                                m_haveUnusedCategories;

  AccountsViewFilterProxyModel        *m_filterProxyModel;
};

#endif
