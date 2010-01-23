/***************************************************************************
                          kcategoriesview.h
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2005 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
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

#ifndef KCATEGORIESVIEW_H
#define KCATEGORIESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <kmymoneyaccounttree.h>
#include <mymoneyutils.h>
//Added by qt3to4:
#include <QPixmap>
class K3ListViewSearchLineWidget;

#include "ui_kcategoriesviewdecl.h"

/**
  * @brief  This class contains the implementation of the categories view.
  * @author Michael Edwardes, Thomas Baumgart
  *
  * While named "categories view", this view actually displays all accounts
  * that are children of the global "Income" and "Expense" accounts. Even though
  * categories are internally just accounts as well, the distinction between
  * categories and accounts in the user interface is done for better
  * usability and clarity.
  *
  * The main functionality in the categories view is actually implemented
  * in the KMyMoneyAccountTree. Signals from user actions are connect to
  * other signals/slots in KCategoriesView and relayed to KMyMoneyView.
  * A typical example is the selectObject() signal that eventually results
  * in enabling/disabling the user actions for the categories view.
  *
  * For the categories view three user actions are important (all created in
  * kmymoney.cpp): category_new, category_edit and category_delete. They are
  * accessible from either the main menu or the context menu.
  */

class KCategoriesViewDecl : public QDialog, public Ui::KCategoriesViewDecl
{
public:
  KCategoriesViewDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

class KCategoriesView : public KCategoriesViewDecl
{
  Q_OBJECT
public:
  KCategoriesView(QWidget *parent = 0);
  virtual ~KCategoriesView();


public slots:
  void slotLoadAccounts(void);

  /**
    * Override the base class behaviour to include all updates that
    * happened in the meantime and restore the layout.
    */
  void showEvent(QShowEvent * event);

protected:
  void loadAccounts(void);
  bool loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QStringList& accountList);

protected slots:
  void slotUpdateProfit(void);
  void slotExpandCollapse(void);


private:
  /**
    * This method returns an icon according to the account type
    * passed in the argument @p type.
    *
    * @param type account type as defined in MyMoneyAccount::accountTypeE
    */
  const QPixmap accountImage(const MyMoneyAccount::accountTypeE type) const;

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTree::selectObject()
    */
  void selectObject(const MyMoneyObject&);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTree::openContextMenu(const MyMoneyObject&)
    */
  void openContextMenu(const MyMoneyObject& obj);

  /**
    * This signal will be emitted when the left mouse button is double
    * clicked (actually the KDE executed setting is used) on an account.
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
  QMap<QString, MyMoneySecurity>      m_securityMap;
  QMap<QString, unsigned long>        m_transactionCountMap;

  KMyMoneyAccountTreeItem*            m_incomeItem;
  KMyMoneyAccountTreeItem*            m_expenseItem;

  /**
   * Search widget for the list
   */
  K3ListViewSearchLineWidget*  m_searchWidget;

  /// set if a view needs to be reloaded during showEvent()
  bool                                m_needReload;
};

#endif
