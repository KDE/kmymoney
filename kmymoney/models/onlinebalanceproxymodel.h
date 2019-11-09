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


#ifndef ONLINEBALANCEPROXYMODEL_H
#define ONLINEBALANCEPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

class OnlineBalanceProxyModelPrivate;
class KMM_MODELS_EXPORT OnlineBalanceProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  enum Column {
    Symbol = 0,
    Quantity,
    Price,
    Value,
  };

  OnlineBalanceProxyModel(QObject *parent = nullptr);
  ~OnlineBalanceProxyModel();

  void setSourceModel ( QAbstractItemModel* sourceModel ) override;

  QVariant data(const QModelIndex& idx, int role) const override;

  int columnCount ( const QModelIndex& parent = QModelIndex() ) const override;

  Qt::ItemFlags flags ( const QModelIndex& index ) const override;

  QModelIndex index ( int row, int column, const QModelIndex & parent ) const override;

protected:
  bool filterAcceptsRow ( int source_row, const QModelIndex& source_parent ) const override;

private:
  void init();
  void load();

private:
  OnlineBalanceProxyModelPrivate* d_ptr;
  Q_DECLARE_PRIVATE(OnlineBalanceProxyModel);
};

#endif // ONLINEBALANCEPROXYMODEL_H
