
/***************************************************************************
                             kmymoneyaccountselector.h
                             -------------------
    begin                : Thu Sep 18 2003
    copyright            : (C) 2003 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYACCOUNTSELECTOR_H
#define KMYMONEYACCOUNTSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

class QPushButton;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyselector.h>
#include "mymoneyenums.h"

class MyMoneyFile;
class MyMoneyAccount;

/**
  * This class implements an account/category selector. It is based
  * on a tree view. Using this widget, one can select one or multiple
  * accounts depending on the mode of operation and the set of accounts
  * selected to be displayed. (see setSelectionMode()
  * and loadList() about the specifics of configuration).
  *
  * - Single selection mode\n
  *   In this mode the widget allows to select a single entry out of
  *   the set of displayed accounts.
  *
  * - Multi selection mode\n
  *   In this mode, the widget allows to select one or more entries
  *   out of the set of displayed accounts. Selection is performed
  *   by marking the account in the view.
  */
class kMyMoneyAccountSelector : public KMyMoneySelector
{
  Q_OBJECT
public:
  friend class AccountSet;

  explicit kMyMoneyAccountSelector(QWidget *parent = 0, Qt::WindowFlags flags = 0, const bool createButtons = true);
  virtual ~kMyMoneyAccountSelector();

  /**
    * This method returns a list of account ids of those accounts
    * currently loaded into the widget. It is possible to select
    * a list of specific account types only. In this case, pass
    * a list of account types as parameter @p list.
    *
    * @param list QList of account types to be returned. If this
    *             list is empty (the default), then the ids of all accounts
    *             will be returned.
    * @return QStringList of account ids
    */
  QStringList accountList(const QList<eMyMoney::Account>& list = QList<eMyMoney::Account>()) const;

  void setSelectionMode(QTreeWidget::SelectionMode mode);

  /**
    * This method checks if a given @a item matches the given regular expression @a exp.
    *
    * @param exp const reference to a regular expression object
    * @param item pointer to QListViewItem
    *
    * @retval true item matches
    * @retval false item does not match
    */
  virtual bool match(const QRegExp& exp, QTreeWidgetItem* item) const;

  /**
    * This method returns, if any of the items in the selector contains
    * the text @a txt.
    *
    * @param txt const reference to string to be looked for
    * @retval true exact match found
    * @retval false no match found
    */
  virtual bool contains(const QString& txt) const;

  /**
    * This method removes all the buttons of the widget
    */
  void removeButtons();

public slots:
  /**
    * This slot selects all items that are currently in
    * the account list of the widget.
    */
  void slotSelectAllAccounts() {
    selectAllItems(true);
  };

  /**
    * This slot deselects all items that are currently in
    * the account list of the widget.
    */
  void slotDeselectAllAccounts() {
    selectAllItems(false);
  };

protected:
  /**
    * This method loads the list of subaccounts as found in the
    * @p list and attaches them to the parent widget passed as @p parent.
    *
    * @param parent pointer to parent widget
    * @param list QStringList containing the ids of all subaccounts to load
    * @return This method returns the number of accounts loaded into the list
    */
  int loadSubAccounts(QTreeWidgetItem* parent, const QStringList& list);

  /**
    * This is a helper method for selectAllIncomeCategories()
    * and selectAllExpenseCategories().
    */
  void selectCategories(const bool income, const bool expense);

protected slots:
  /**
    * This slot selects all income categories
    */
  void slotSelectIncomeCategories() {
    selectCategories(true, false);
  };

  /**
    * This slot selects all expense categories
    */
  void slotSelectExpenseCategories() {
    selectCategories(false, true);
  };

protected:
  QPushButton*              m_allAccountsButton;
  QPushButton*              m_noAccountButton;
  QPushButton*              m_incomeCategoriesButton;
  QPushButton*              m_expenseCategoriesButton;
  QList<int>                m_typeList;
  QStringList               m_accountList;
};


class AccountSet
{
public:
  AccountSet();

  void addAccountType(eMyMoney::Account type);
  void addAccountGroup(eMyMoney::Account type);
  void removeAccountType(eMyMoney::Account type);

  void clear();

  int load(kMyMoneyAccountSelector* selector);
  int load(kMyMoneyAccountSelector* selector, const QString& baseName, const QList<QString>& accountIdList, const bool clear = false);

  int count() const {
    return m_count;
  }

  void setHideClosedAccounts(bool _bool) {
    m_hideClosedAccounts = _bool;
  }
  bool isHidingClosedAccounts() const {
    return m_hideClosedAccounts;
  }

protected:
  int loadSubAccounts(kMyMoneyAccountSelector* selector, QTreeWidgetItem* parent, const QString& key, const QStringList& list);
  bool includeAccount(const MyMoneyAccount& acc);

private:
  int                                      m_count;
  MyMoneyFile*                             m_file;
  QList<eMyMoney::Account>      m_typeList;
  QTreeWidgetItem*                         m_favorites;
  bool                                     m_hideClosedAccounts;
};
#endif
