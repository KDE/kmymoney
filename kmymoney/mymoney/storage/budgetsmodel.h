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

#ifndef BUDGETSMODEL_H
#define BUDGETSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneybudget.h"

/**
  */
class KMM_MYMONEY_EXPORT BudgetsModel : public MyMoneyModel<MyMoneyBudget>
{
  Q_OBJECT

public:
  enum Columns {
      Name,
      Year
  };

  explicit BudgetsModel(QObject* parent = 0);
  ~BudgetsModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // BUDGETSMODEL_H

