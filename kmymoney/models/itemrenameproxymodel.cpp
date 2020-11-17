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

#include "itemrenameproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyfile.h"

ItemRenameProxyModel::ItemRenameProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
  , m_renameColumn(0)
  , m_referenceFilter(eAllItem)
{
}

ItemRenameProxyModel::~ItemRenameProxyModel()
{
}

void ItemRenameProxyModel::setRenameColumn(int column)
{
  m_renameColumn = column;
}

bool ItemRenameProxyModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  if (idx.column() == m_renameColumn && role == Qt::EditRole) {
    qDebug() << "Rename to" << value.toString();
    emit renameItem(idx, value);
    return true;
  }
  return QSortFilterProxyModel::setData(idx, value, role);
}

Qt::ItemFlags ItemRenameProxyModel::flags(const QModelIndex& idx) const
{
  auto flags = QSortFilterProxyModel::flags(idx);
  if (idx.column() == m_renameColumn) {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

void ItemRenameProxyModel::setReferenceFilter(const QVariant& filterType)
{
  const uint type = filterType.toUInt();
  if (type < eMaxItems) {
    setReferenceFilter(static_cast<ReferenceFilterType>(type));
  }
}

void ItemRenameProxyModel::setReferenceFilter(ItemRenameProxyModel::ReferenceFilterType filterType)
{
  if (m_referenceFilter != filterType) {
    m_referenceFilter = filterType;
    invalidateFilter();
  }
}

bool ItemRenameProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  if (m_referenceFilter != eAllItem) {
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    const auto itemId = idx.data(eMyMoney::Model::IdRole).toString();
    if (!itemId.isEmpty()) {
      QVariant rc;
      switch(m_referenceFilter) {
        case eReferencedItems:
          if (!MyMoneyFile::instance()->referencedObjects().contains(itemId))
            return false;
          break;
        case eUnReferencedItems:
          if (MyMoneyFile::instance()->referencedObjects().contains(itemId))
            return false;
          break;
        case eOpenedItems:
          rc = idx.data(eMyMoney::Model::ClosedRole);
          if (rc.isValid() && (rc.toBool() == true)) {
            return false;
          }
          break;
        case eClosedItems:
          rc = idx.data(eMyMoney::Model::ClosedRole);
          if (rc.isValid() && (rc.toBool() == false)) {
            return false;
          }
          break;
        default:
          break;
      }
    }
  }
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
