/*
 * Copyright 2016-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef COSTCENTERMODEL_H
#define COSTCENTERMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneycostcenter.h"

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT CostCenterModel : public MyMoneyModel<MyMoneyCostCenter>
{
  Q_OBJECT

public:
  class Column {
    enum {
      Name
    } Columns;
  };

  explicit CostCenterModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~CostCenterModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // COSTCENTERMODEL_H

