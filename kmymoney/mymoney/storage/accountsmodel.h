/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef ACCOUNTSMODEL_H
#define ACCOUNTSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyaccount.h"

/**
  */
class KMM_MYMONEY_EXPORT AccountsModel : public MyMoneyModel<MyMoneyAccount>
{
  Q_OBJECT
  Q_DISABLE_COPY(AccountsModel)

public:
  enum Column {
    AccountName = 0,
    Type,
    Tax,
    Vat,
    CostCenter,
    TotalBalance,
    PostedValue,
    TotalValue,
    Number,
    SortCode,
    // insert new columns above this line
    MaxColumns
  };


  explicit AccountsModel(QObject* parent = 0);
  virtual ~AccountsModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  void load(const QMap<QString, MyMoneyAccount>& list);

  QList<MyMoneyAccount> itemList() const;
  QModelIndex indexById(const QString& id) const;
  QModelIndexList indexListByName(const QString& name) const;


  /**
   * Returns the full account path for the given index @a idx.
   */
  QString indexToHierarchicalName(const QModelIndex& idx, bool includeStandardAccounts) const;

  /**
   * Convert the given @a accountId into the full hierarchical name
   * Include the standard base account if @a includeStandardAccounts is @c true
   *
   * Returns the full account path or empty if @a accountId is not known
   *
   * @sa indexToHierarchicalName()
   */
  QString accountIdToHierarchicalName(const QString& accountId, bool includeStandardAccounts) const;

  /**
   * Convert a given account @a name and @a type into the corresponding id. Valid types are
   * eMyMoney::Account::Type::(Income|Expense|Unknown). In case of Unknown, both Income and
   * Expense subtrees will be searched. The first matching entry will be used. If no account
   * is found, an empty string will be returned.
   */
  QString accountNameToId(const QString& name, eMyMoney::Account::Type type) const;

protected:
  void clearModelItems() override;
  void addFavorite(const QString& id);
  void removeFavorite(const QString& id);
  int processItems(Worker *worker) override;

  // reparent()
public Q_SLOTS:


private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // ACCOUNTSMODEL_H

