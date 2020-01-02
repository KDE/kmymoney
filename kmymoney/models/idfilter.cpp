/*
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#include "idfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class IdFilterPrivate
{
public:
  IdFilterPrivate()
  {
  }

  QSet<QString>         idList;
};


IdFilter::IdFilter(QObject* parent)
  : QSortFilterProxyModel(parent)
  , d_ptr(new IdFilterPrivate)
{
}

bool IdFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  Q_D(const IdFilter);

  const auto idx = sourceModel()->index(source_row, 0, source_parent);
  return !d->idList.contains(idx.data(eMyMoney::Model::IdRole).toString());
}

void IdFilter::setFilterList(const QStringList& idList)
{
  Q_D(IdFilter);
  d->idList = QSet<QString>::fromList(idList);
  invalidateFilter();
}

void IdFilter::addFilter(const QString& id)
{
  Q_D(IdFilter);
  d->idList.insert(id);
  invalidateFilter();
}

void IdFilter::removeFilter(const QString& id)
{
  Q_D(IdFilter);
  if (d->idList.remove(id)) {
    invalidateFilter();
  }
}

QList<QString> IdFilter::filterList() const
{
  Q_D(const IdFilter);
  return d->idList.toList();
}

bool IdFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  // make sure that the empty item is shown first in any case
  if(left.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
    return true;

  } else if(right.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
    return false;
  }

  // let the base class do the real work
  return QSortFilterProxyModel::lessThan(left, right);
}


