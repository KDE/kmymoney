/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
};

#undef QSortFilterProxyModel

#endif // SCHEDULEPROXYMODEL_H
