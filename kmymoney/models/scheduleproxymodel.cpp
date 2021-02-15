/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
