/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
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

#include <KItemModels/KRecursiveFilterProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneyfile.h"

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
    DisplayOrderRole = Qt::UserRole + 9,              /**< This role is used by the filtering proxies to order the accounts for displaying.*/
    FullNameRole = Qt::UserRole + 10,                 /**< This role is used to provide the full pathname of the account */
  };

  /**
    * The columns of this model.
    */
  enum Columns {
    FirstColumnMarker = 0,
    Account = 0,  // CAUTION! Assumption is being made that Account column number is always 0 and you shouldn't change this
    Type,
    Tax,
    VAT,
    CostCenter,
    TotalBalance,
    PostedValue,
    TotalValue,
    AccountNumber,
    AccountSortCode,
    LastColumnMarker
  };

  /**
    * The account id used by this model for the 'Favorites' top level item. This can be used to identify that item on the @ref AccountIdRole.
    */
  static const QString favoritesAccountId;

  ~AccountsModel();

  /**
    * This method must be used to perform the initial load of the model.
    */
  void load();

  /**
    * Compute the value of the given account using the provided balance.
    * The value is defined as the balance of the account converted to the base currency.
    *
    * @param account The account for which the value is being computed.
    * @param balance The balance which should be used.
    *
    * @todo Make this a static or a global function since the object's state has nothing to do with this computation
    */
  MyMoneyMoney accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance);

  /**
   * This method returns the QModelIndex of the account specified by its @a id. If the
   * account was not found, an invalid QModelIndex is returned.
   */
  QModelIndex accountById(const QString& id) const;

  QList<AccountsModel::Columns> *getColumns();

  void setColumnVisibility(const Columns column, const bool show);
  static QString getHeaderName(const Columns column);

public slots:

  void slotReconcileAccount(const MyMoneyAccount &account, const QDate &reconciliationDate, const MyMoneyMoney &endingBalance);
  void slotObjectAdded(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj);
  void slotObjectModified(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj);
  void slotObjectRemoved(MyMoneyFile::notificationObjectT objType, const QString& id);
  void slotBalanceOrValueChanged(const MyMoneyAccount &account);

signals:
  /**
    * Emit this signal when the net worth based on the value of the loaded accounts is changed.
    */
  void netWorthChanged(const MyMoneyMoney &);

  /**
    * Emit this signal when the profit based on the value of the loaded accounts is changed.
    */
  void profitChanged(const MyMoneyMoney &);

private:
  AccountsModel(QObject *parent = 0);

  void init();
  void checkNetWorth();
  void checkProfit();

  /**
    * The copy-constructor is private so that only the @ref Models object can create such an object.
    */
  AccountsModel(const AccountsModel&);
  AccountsModel& operator=(AccountsModel&);

  /**
    * Allow only the @ref Models object to create such an object.
    */
  friend class Models;

protected:
  class Private;
  Private* const d;

  /**
    * This constructor can be used from derived classes in order to use a derived Private class.
    */
  AccountsModel(Private* const priv, QObject *parent = 0);
};

/**
  * A model for the accounts grouped by institutions. It extends the functionality already present
  * in @ref AccountsModel to enable the grouping of the accounts by institutions.
  *
  * @author Cristian Onet 2011
  *
  */
class InstitutionsModel : public AccountsModel
{
  Q_OBJECT

public:
  /**
    * This method must be used to perform the initial load of the model.
    */
  void load();

public slots:
  void slotObjectAdded(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj);
  void slotObjectModified(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj);
  void slotObjectRemoved(MyMoneyFile::notificationObjectT objType, const QString& id);

private:
  InstitutionsModel(QObject *parent = 0);

  /**
    * The copy-constructor is private so that only the @ref Models object can create such an object.
    */
  InstitutionsModel(const InstitutionsModel&);
  InstitutionsModel& operator=(InstitutionsModel&);

  /**
    * Allow only the @ref Models object to create such an object.
    */
  friend class Models;

  /**
    * The implementation object is derived from the @ref AccountsModel objects implementation object.
    */
  class InstitutionsPrivate;
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
  * @see AccountsModel
  *
  * @author Cristian Onet 2010
  *
  */
class AccountsFilterProxyModel : public KRecursiveFilterProxyModel
{
  Q_OBJECT

public:
  AccountsFilterProxyModel(QObject *parent = 0);
  ~AccountsFilterProxyModel();

  void addAccountType(MyMoneyAccount::accountTypeE type);
  void addAccountGroup(const QVector<MyMoneyAccount::_accountTypeE> &groups);
  void removeAccountType(MyMoneyAccount::accountTypeE type);

  void clear();

  void setHideClosedAccounts(bool hideClosedAccounts);
  bool hideClosedAccounts() const;

  void setHideEquityAccounts(bool hideEquityAccounts);
  bool hideEquityAccounts() const;

  void setHideUnusedIncomeExpenseAccounts(bool hideUnusedIncomeExpenseAccounts);
  bool hideUnusedIncomeExpenseAccounts() const;

  int visibleItems(bool includeBaseAccounts = false) const;

  void init(AccountsModel *model, QList<AccountsModel::Columns> *visColumns);
  void init(AccountsModel *model);

protected:
  virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
  virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
  virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  virtual bool acceptSourceItem(const QModelIndex &source) const;

  bool filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const;

  int visibleItems(const QModelIndex& index) const;

  QList<AccountsModel::Columns> *m_mdlColumns;
  QList<AccountsModel::Columns> *m_visColumns;
signals:
  void unusedIncomeExpenseAccountHidden() const;

private:
  class Private;
  Private* const d;
};

#endif // ACCOUNTSMODEL_H
