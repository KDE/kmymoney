/*
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

#ifndef SECURITIESFILTERPROXYMODEL_H
#define SECURITIESFILTERPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <KItemModels/KRecursiveFilterProxyModel>
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "securitiesmodel.h"

/**
 * This model provides a filter for security accounts.
 */
class KMM_MODELS_EXPORT SecuritiesFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  SecuritiesFilterProxyModel(QObject *parent , SecuritiesModel *model);
  ~SecuritiesFilterProxyModel();

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  class Private;
  Private* const d;

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  // provide the interface for backward compatbility
  void setRecursiveFilteringEnabled(bool enable) { Q_UNUSED(enable); }
#endif

};

#undef QSortFilterProxyModel
#endif // SECURITIESFILTERPROXYMODEL_H
