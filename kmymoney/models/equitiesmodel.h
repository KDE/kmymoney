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


#include <QSortFilterProxyModel>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <KItemModels/KRecursiveFilterProxyModel>
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif
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

class KMM_MODELS_EXPORT EquitiesFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  EquitiesFilterProxyModel(QObject *parent , EquitiesModel *model, const QList<EquitiesModel::Column> &columns = QList<EquitiesModel::Column>());
  ~EquitiesFilterProxyModel();

  QList<EquitiesModel::Column> &getVisibleColumns();
  void setHideClosedAccounts(const bool hideClosedAccounts);
  void setHideZeroBalanceAccounts(const bool hideZeroBalanceAccounts);

Q_SIGNALS:
  void columnToggled(const EquitiesModel::Column column, const bool show);

public Q_SLOTS:
  void slotColumnsMenu(const QPoint);

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
  class Private;
  Private* const d;

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  // provide the interface for backward compatbility
  void setRecursiveFilteringEnabled(bool enable) { Q_UNUSED(enable) }
#endif

};

#undef QSortFilterProxyModel
#endif // EQUITIESMODEL_H
