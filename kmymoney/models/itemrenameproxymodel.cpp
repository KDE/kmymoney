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

ItemRenameProxyModel::ItemRenameProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
  , m_renameColumn(0)
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
