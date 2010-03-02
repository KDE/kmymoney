/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#ifndef ACCOUNTSMODEL_H
#define ACCOUNTSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"

/**
  * A model for the accounts.
  * This model loads all the accounts from the @ref MyMoneyFile.
  * It also computes various data like account balances needed
  * in different views. This object should be kept sychronized
  * with the data in the @ref MyMoneyFile (this is accomplished
  * by the @ref Models object).
  *
  * @see MyMoneyAccount
  * @see MyMoneyFile
  *
  * @author Cristian Onet 2010
  *
  */
class AccountsModel : public QStandardItemModel
{
  Q_OBJECT

public:

  /**
    * User roles used by this model.
    */
  enum AccountsItemDataRole {
    AccountIdRole = Qt::UserRole,                     /**< The account id is stored in this role in column 0 as a string.*/
    AccountFavoriteRole = Qt::UserRole + 1,           /**< The 'account is favorite' property is stored in this role in column 0 as a bool.*/
    AccountRole = Qt::UserRole + 2,                   /**< The MyMoneyAccount is stored in this role in column 0.*/
    AccountBalanceRole = Qt::UserRole + 3,            /**< The account balance is stored in this role in column 0 as a MyMoneyMoney object.*/
    AccountValueRole = Qt::UserRole + 4,              /**< The account value (the balance converted to base currency) is stored in this role in column 0 as a MyMoneyMoney object.*/
    AccountTotalValueRole = Qt::UserRole + 5,         /**< The account total value (the value of the account and of child accounts) is stored in this role in column 0 as a MyMoneyMoney object.*/
    AccountBalanceDispalyRole = Qt::UserRole + 6,     /**< The account balance is stored in this role in column TotalBalance as a formatted string for the user.*/
    AccountValueDisplayRole = Qt::UserRole + 7,       /**< The account value (the balance converted to base currency) is stored in this role in column TotalValue as a formated string for the user.*/
    AccountTotalValueDisplayRole = Qt::UserRole + 8,  /**< The account total value is stored in this role in column TotalValue as a formatted string for the user.*/
    DisplayOrderRole = Qt::UserRole + 9,              /**< This role is used by the filtering proxies to order the accounts for displaying.*/
    CleanupRole = Qt::UserRole + 10                   /**< This role is used internally by the model to clean up removed accounts. */
  };

  /**
    * The columns of this model.
    */
  enum Columns {
    FirstColumnMarker = 0,
    Account = 0,
    Type,
    Tax,
    VAT,
    TotalBalance,
    TotalValue,
    LastColumnMarker
  };

  /**
    * The account id used by this model for the 'Favorites' top level item. This can be used to identify that item on the @ref AccountIdRole.
    */
  static const QString favoritesAccountId;

  ~AccountsModel();

  void load();

public slots:

  void slotReconcileAccount(const MyMoneyAccount &account, const QDate &reconciliationDate, const MyMoneyMoney &endingBalance);

signals:
  /**
    * Emit this signal when the net worth based on the value of the loaded accounts is changed.
    */
  void netWorthChanged(const MyMoneyMoney &);

private:
  AccountsModel(QObject *parent = 0);

  /**
    * The copy-constructor is private so that only the @ref Models object can create such an object.
    */
  AccountsModel(const AccountsModel&);
  AccountsModel& operator=(AccountsModel&);

  /**
    * Allow only the @ref Models object to create such an object.
    */
  friend class Models;

private:
  class Private;
  Private* const d;
};

/**
  * A proxy model to provide various sorting and filtering operations for @ref AccountsModel.
  *
  * Here is an example of how to use this class in combination with the @ref AccountsModel.
  * (in the example @a widget is a pointer to a model/view widget):
  *
  * @code
  *   AccountsFilterProxyModel *filterModel = new AccountsFilterProxyModel(widget);
  *   filterModel->addAccountGroup(MyMoneyAccount::Asset);
  *   filterModel->addAccountGroup(MyMoneyAccount::Liability);
  *   filterModel->setSourceModel(Models::instance()->accountsModel());
  *   filterModel->sort(0);
  *
  *   widget->setModel(filterModel);
  * @endcode
  *
  * @endcode
  *
  * @see AccountsModel
  *
  * @author Cristian Onet 2010
  *
  */
class AccountsFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  AccountsFilterProxyModel(QObject *parent = 0);

  void addAccountType(MyMoneyAccount::accountTypeE type);
  void addAccountGroup(MyMoneyAccount::accountTypeE type);
  void removeAccountType(MyMoneyAccount::accountTypeE type);

  void clear(void);

  void setHideClosedAccounts(bool hideClosedAccounts);
  bool hideClosedAccounts(void) const;

protected:
  virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
  virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  virtual bool acceptSourceItem(const QModelIndex &source) const;

  bool filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const;

private:
  class Private;
  Private* const d;
};

#endif // ACCOUNTSMODEL_H
