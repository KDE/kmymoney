/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "idfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmset.h"
#include "mymoneyenums.h"

class IdFilterPrivate
{
public:
    IdFilterPrivate()
    {
    }

    KMMStringSet idList;
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

    // in case the object supports the ClosedRole, we evaluate it
    // and don't accept this item when it is true
    const auto closedRole = idx.data(eMyMoney::Model::ClosedRole);
    if (closedRole.isValid() && (closedRole.toBool() == true))
        return false;

    return !d->idList.contains(idx.data(eMyMoney::Model::IdRole).toString());
}

void IdFilter::setFilterList(const QStringList& idList)
{
    Q_D(IdFilter);
    d->idList = idList;
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
    if (d->idList.erase(id)) {
        invalidateFilter();
    }
}

QList<QString> IdFilter::filterList() const
{
    Q_D(const IdFilter);
    return d->idList.values();
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
