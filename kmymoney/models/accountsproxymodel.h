/*
 * Copyright 2010-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACCOUNTSPROXYMODEL_H
#define ACCOUNTSPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KItemModels/KRecursiveFilterProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

/**
  * A proxy model to provide various sorting and filtering operations for @ref AccountsModel.
  *
  * Here is an example of how to use this class in combination with the @ref AccountsModel.
  * (in the example @a widget is a pointer to a model/view widget):
  *
  * @code
  *   AccountsFilterProxyModel *filterModel = new AccountsFilterProxyModel(widget);
  *   filterModel->addAccountGroup(eMyMoney::Account::Type::Asset);
  *   filterModel->addAccountGroup(eMyMoney::Account::Type::Liability);
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

namespace eMyMoney { namespace Account {enum class Type;} }
namespace eAccountsModel { enum class Column; }

class AccountsProxyModelPrivate;
class KMM_MODELS_EXPORT AccountsProxyModel : public KRecursiveFilterProxyModel
{
  Q_OBJECT
  Q_DISABLE_COPY(AccountsProxyModel)

public:
  explicit AccountsProxyModel(QObject *parent = nullptr);
  virtual ~AccountsProxyModel();

  void addAccountType(eMyMoney::Account::Type type);
  void addAccountGroup(const QVector<eMyMoney::Account::Type> &groups);
  void removeAccountType(eMyMoney::Account::Type type);

  void clear();

  void setHideClosedAccounts(bool hideClosedAccounts);
  bool hideClosedAccounts() const;

  void setHideEquityAccounts(bool hideEquityAccounts);
  bool hideEquityAccounts() const;

  void setHideUnusedIncomeExpenseAccounts(bool hideUnusedIncomeExpenseAccounts);
  bool hideUnusedIncomeExpenseAccounts() const;

  int visibleItems(bool includeBaseAccounts = false) const;

  void setSourceColumns(QList<eAccountsModel::Column> *columns);

protected:
  const QScopedPointer<AccountsProxyModelPrivate> d_ptr;
  AccountsProxyModel(AccountsProxyModelPrivate &dd, QObject *parent);

  bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
  bool acceptSourceItem(const QModelIndex &source) const;

  bool filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const;

  int visibleItems(const QModelIndex& index) const;

Q_SIGNALS:
  void unusedIncomeExpenseAccountHidden();

private:
  Q_DECLARE_PRIVATE(AccountsProxyModel)
};

#endif
