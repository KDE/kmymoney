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

#ifndef SCHEDULEPROXYMODEL_H
#define SCHEDULEPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <QSortFilterProxyModel>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <KItemModels/KRecursiveFilterProxyModel>
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif

/**
  * A proxy model for the schedules view to filter and sort schedule items
  *
  * @author Thomas Baumgart
  *
  */

class ScheduleProxyModelPrivate;
class KMM_MODELS_EXPORT ScheduleProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_DISABLE_COPY(ScheduleProxyModel)

public:
  explicit ScheduleProxyModel (QObject *parent = nullptr);
  virtual ~ScheduleProxyModel ();

  void setHideFinishedSchedules(bool hide);

protected:
  virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
  virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

protected:
  QScopedPointer<ScheduleProxyModelPrivate> d;

private:
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  // provide the interface for backward compatbility
  void setRecursiveFilteringEnabled(bool enable) { Q_UNUSED(enable); }
#endif

};

#undef QSortFilterProxyModel

#endif // SCHEDULEPROXYMODEL_H
