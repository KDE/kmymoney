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

#ifndef KACCOUNTSSVIEW_H
#define KACCOUNTSSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3iconview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <kmymoneyaccounttree.h>
#include <mymoneyutils.h>
class K3ListViewSearchLineWidget;

#include "ui_kaccountsviewdecl.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents an item in the account icon view. It is used
  * by the KAccountsView to select between the accounts using icons.
  */
class KMyMoneyAccountIconItem : public K3IconViewItem
{
public:
  /**
    * Constructor to be used to construct an account icon object.
    *
    * @param parent pointer to the K3IconView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the K3IconView entry is constructed
    */
  KMyMoneyAccountIconItem(Q3IconView *parent, const MyMoneyAccount& account);
  ~KMyMoneyAccountIconItem();

  /**
    * This method is loads new information into the item and updates the fields
    *
    * @param account the account data for the object to be updated
    *
    * @note if account.id() is not equal to the current account id
    *       then this method returns immediately
    */
  void updateAccount(const MyMoneyAccount& account);

  const MyMoneyObject& itemObject(void) const { return m_account; };

  void setReconciliation(bool);

protected:

private:
  MyMoneyAccount        m_account;
  bool                  m_reconcileFlag;
};




/**
  * This class implements the accounts hierarchical and iconic 'view'.
  */

class KAccountsViewDecl : public QWidget, public Ui::KAccountsViewDecl
{
public:
  KAccountsViewDecl( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};
class KAccountsView : public KAccountsViewDecl
{
  Q_OBJECT
private:

public:
  KAccountsView(QWidget *parent=0);
  virtual ~KAccountsView();

public slots:
  void slotLoadAccounts(void);

  /**
    * Override the base class behaviour to include all updates that
    * happened in the meantime.
    */
  void show(void);

  /**
   * Override the base class behaviour to restore the layout. Do not
   * do this in show() because show() itself may change the layout
   * in undesired ways.
   */
  void polish(void);

  /**
    * update the account objects if their icon position has changed since
    * the last time.
    *
    * @param action must be KMyMoneyView::preSave, otherwise this slot is a NOP.
    */
  void slotUpdateIconPos(unsigned int action);

  void slotReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);

protected:
  typedef enum {
    ListView = 0,
    IconView,
    // insert new values above this line
    MaxViewTabs
  } AccountsViewTab;

  /**
    * This method loads the accounts for the respective tab.
    *
    * @param tab which tab should be loaded
    */
  void loadAccounts(AccountsViewTab tab);
  void loadListView(void);
  void loadIconView(void);

  bool loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QStringList& accountList);

  /**
    * This method returns a pointer to the currently selected
    * account icon or 0 if no icon is selected.
    */
  KMyMoneyAccountIconItem* selectedIcon(void) const;

  QPoint point(const QString& str) const;
  QString point(const QPoint& val) const;

protected slots:
  void slotUpdateNetWorth(void);
  void slotTabChanged(QWidget*);
  void slotSelectIcon(Q3IconViewItem* item);
  void slotOpenContext(Q3IconViewItem* item);
  void slotOpenObject(Q3IconViewItem* item);

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTree::selectObject()
    *
    * @param obj const reference to object
    */
  void selectObject(const MyMoneyObject& obj);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTree::openContextMenu(const MyMoneyObject&)
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
    * This signal is emitted, when the user selected to reparent the
    * account @p acc to be a subordinate account of @p parent.
    *
    * @param acc const reference to account to be reparented
    * @param parent const reference to new parent account
    */
  void reparent(const MyMoneyAccount& acc, const MyMoneyAccount& parent);

private:
  MyMoneyAccount                      m_reconciliationAccount;
  QMap<QString, MyMoneySecurity>      m_securityMap;
  QMap<QString, unsigned long>        m_transactionCountMap;

  KMyMoneyAccountTreeItem*            m_assetItem;
  KMyMoneyAccountTreeItem*            m_liabilityItem;

  /**
   * Search widget for the list
   */
  K3ListViewSearchLineWidget*  m_searchWidget;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload[MaxViewTabs];
  bool                                m_haveUnusedCategories;
};

#endif
