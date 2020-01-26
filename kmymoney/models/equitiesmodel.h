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


#ifndef EQUITIESMODEL_H
#define EQUITIESMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KExtraColumnsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

class EquitiesModelPrivate;
class KMM_MODELS_EXPORT EquitiesModel : public KExtraColumnsProxyModel
{
  Q_OBJECT
public:
  enum Column {
    Symbol = 0,
    Quantity,
    Price,
    Value,
  };

  EquitiesModel(QObject *parent = nullptr);
  ~EquitiesModel();

  QVariant extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role = Qt::DisplayRole) const override;

public Q_SLOTS:

private:
  void init();
  void load();

private:
  EquitiesModelPrivate* d_ptr;
  Q_DECLARE_PRIVATE(EquitiesModel);
};

#endif // EQUITIESMODEL_H
