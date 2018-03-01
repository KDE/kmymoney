
/***************************************************************************
                             kmymoneyaccountselector.h
                             -------------------
    begin                : Thu Sep 18 2003
    copyright            : (C) 2003 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyselector.h"

class MyMoneyAccount;

template <typename T> class QList;

namespace eMyMoney { namespace Account { enum class Type; } }

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
class KMyMoneyAccountSelectorPrivate;
class KMM_WIDGETS_EXPORT KMyMoneyAccountSelector : public KMyMoneySelector
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountSelector)

public:
  friend class AccountSet;

  explicit KMyMoneyAccountSelector(QWidget* parent = nullptr, Qt::WindowFlags flags = 0, const bool createButtons = true);
  ~KMyMoneyAccountSelector() override;

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
  QStringList accountList(const QList<eMyMoney::Account::Type>& list) const;
  QStringList accountList() const;

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
  virtual bool match(const QRegExp& exp, QTreeWidgetItem* item) const override;

  /**
    * This method returns, if any of the items in the selector contains
    * the text @a txt.
    *
    * @param txt const reference to string to be looked for
    * @retval true exact match found
    * @retval false no match found
    */
  virtual bool contains(const QString& txt) const override;

  /**
    * This method removes all the buttons of the widget
    */
  void removeButtons();

public Q_SLOTS:
  /**
    * This slot selects all items that are currently in
    * the account list of the widget.
    */
  void slotSelectAllAccounts();

  /**
    * This slot deselects all items that are currently in
    * the account list of the widget.
    */
  void slotDeselectAllAccounts();

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

protected Q_SLOTS:
  /**
    * This slot selects all income categories
    */
  void slotSelectIncomeCategories();

  /**
    * This slot selects all expense categories
    */
  void slotSelectExpenseCategories();

private:
  Q_DECLARE_PRIVATE(KMyMoneyAccountSelector)
};

class AccountSetPrivate;
class KMM_WIDGETS_EXPORT AccountSet
{
  Q_DISABLE_COPY(AccountSet)

public:
  AccountSet();
  AccountSet(AccountSet && other);
  friend void swap(AccountSet& first, AccountSet& second);
  ~AccountSet();

  void addAccountType(eMyMoney::Account::Type type);
  void addAccountGroup(eMyMoney::Account::Type type);
  void removeAccountType(eMyMoney::Account::Type type);

  void clear();

  int load(KMyMoneyAccountSelector* selector);
  int load(KMyMoneyAccountSelector* selector, const QString& baseName, const QList<QString>& accountIdList, const bool clear = false);

  int count() const;

  void setHideClosedAccounts(bool _bool);
  bool isHidingClosedAccounts() const;

protected:
  int loadSubAccounts(KMyMoneyAccountSelector* selector, QTreeWidgetItem* parent, const QString& key, const QStringList& list);
  bool includeAccount(const MyMoneyAccount& acc);

private:
  AccountSetPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(AccountSet)
};

#endif
