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

enum AccountsItemDataRole {
  AccountIdRole = Qt::UserRole,
  AccountTypeRole = Qt::UserRole + 1,
  AccountClosedRole = Qt::UserRole + 2,
  AccountFavoriteRole = Qt::UserRole + 3,
  DisplayOrderRole = Qt::UserRole + 4
};

/**
  * A model for the accounts.
  * This model loads all the accounts from the @see MyMoneyFile.
  *
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
    AccountsModel(QObject *parent = 0);

    void load();

  private:
    class Private;
    Private* const d;
};

/**
  * A proxy mode to provide various sorting and filtering operations for the above model.
  *
  * Here's an example of how to use this clas in combination with the accounts model
  * (in the example widget is a pointer to a model/view widget):
  *
  *   AccountsFilterProxyModel *filterModel = new AccountsFilterProxyModel(widget);
  *   filterModel->addAccountGroup(MyMoneyAccount::Asset);
  *   filterModel->addAccountGroup(MyMoneyAccount::Liability);
  *   filterModel->setSourceModel(new AccountsModel(widget));
  *   filterModel->sort(0);
  *
  *   widget->setModel(filterModel);
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
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  virtual bool acceptSourceItem(const QModelIndex &source) const;

private:
  class Private;
  Private* const d;
};

#endif // ACCOUNTSMODEL_H
