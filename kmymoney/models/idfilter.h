/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IDFILTER_H
#define IDFILTER_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class IdFilterPrivate;
class KMM_MODELS_EXPORT IdFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(IdFilter)
    Q_DISABLE_COPY(IdFilter)

public:
    explicit IdFilter(QObject* parent);

    void setFilterList(const QStringList& idList);
    void addFilter(const QString& id);
    void removeFilter(const QString& id);
    QList<QString> filterList() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    IdFilterPrivate*  d_ptr;
};

#endif // IDFILTER_H

