/*
 * Copyright 2016-2017  Thomas Baumgart <tbaumgart@kde.org>
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

#include <QAbstractListModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


/**
  */
class CostCenterModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit CostCenterModel(QObject* parent = 0);
  virtual ~CostCenterModel();

  enum Roles {
    CostCenterIdRole = Qt::UserRole,      // must remain Qt::UserRole due to KMyMoneyMVCCombo::selectedItem
    ShortNameRole,
  };

  int rowCount(const QModelIndex& parent = QModelIndex()) const final override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  Qt::ItemFlags flags(const QModelIndex& index) const final override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  /**
   * clears all objects currently in the model
   */
  void unload();

  /**
   * Loads the model with data from the engine
   */
  void load();

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // COSTCENTERMODEL_H

