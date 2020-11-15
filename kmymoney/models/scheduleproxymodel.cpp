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

#include "scheduleproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "schedulesmodel.h"

class ScheduleProxyModelPrivate
{
public:
  ScheduleProxyModelPrivate()
    : m_hideFinishedSchedules(false)
  {}

  bool m_hideFinishedSchedules;
};

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif

ScheduleProxyModel::ScheduleProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
  , d(new ScheduleProxyModelPrivate)
{
  setRecursiveFilteringEnabled(true);
}

#undef QSortFilterProxyModel

ScheduleProxyModel::~ScheduleProxyModel()
{
}

bool ScheduleProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  // we only need to take care of sub-items here
  if (source_parent.isValid()) {
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    if (d->m_hideFinishedSchedules && idx.data(eMyMoney::Model::ScheduleIsFinishedRole).toBool()) {
      return false;
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
  }
  return true;
}

void ScheduleProxyModel::setHideFinishedSchedules(bool hide)
{
  d->m_hideFinishedSchedules = hide;
}

bool ScheduleProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
  if (source_left.parent().isValid() && source_right.parent().isValid()) {
    switch (source_left.column()) {
      case SchedulesModel::Column::Name:
      case SchedulesModel::Column::Account:
      case SchedulesModel::Column::Payee:
        break;

      case SchedulesModel::Column::NextDueDate:
        return source_left.data(eMyMoney::Model::ScheduleNextDueDateRole).toDate() < source_right.data(eMyMoney::Model::ScheduleNextDueDateRole).toDate();

      case SchedulesModel::Column::Frequency:
        return source_left.data(eMyMoney::Model::ScheduleFrequencyRole).toInt() < source_right.data(eMyMoney::Model::ScheduleFrequencyRole).toInt();
    }
  }
  return QSortFilterProxyModel::lessThan(source_left, source_right);
}
