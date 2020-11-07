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

#ifndef INSTITUTIONSMODEL_H
#define INSTITUTIONSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "accountsmodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyinstitution.h"

class AccountsModel;
class QColor;
class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT InstitutionsModel : public MyMoneyModel<MyMoneyInstitution>
{
  Q_OBJECT

public:
  explicit InstitutionsModel(AccountsModel* accountsModel, QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~InstitutionsModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) final override;

  void load(const QMap<QString, MyMoneyInstitution>& list);
  void addAccount(const QString& institutionId, const QString& accountId);
  void removeAccount(const QString& institutionId, const QString& accountId);

  void setColorScheme(AccountsModel::ColorScheme scheme, const QColor& color);

public Q_SLOTS:
  /**
   * Add the accounts pointed to by @a indexes to the group of
   * accounts not assigned to any institution. The indexes should
   * point into the AccountsModel. The addition is performed on
   * the id returned by the @c IdRole role. The dirty flag
   * is not modified.
   */
  void slotLoadAccountsWithoutInstitutions(const QModelIndexList& indexes);

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // INSTITUTIONSMODEL_H

