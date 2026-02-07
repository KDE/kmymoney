/*
    SPDX-FileCopyrightText: 2026 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegularExpression>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "reportsmodel.h"

class ReportsViewProxyModel : public QSortFilterProxyModel
{
public:
    explicit ReportsViewProxyModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setFilterCaseSensitivity(Qt::CaseInsensitive);
        setRecursiveFilteringEnabled(true);
    }

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override
    {
        QString l = sourceModel()->data(left, Qt::DisplayRole).toString();
        QString r = sourceModel()->data(right, Qt::DisplayRole).toString();

        static const QRegularExpression re(R"(^(\d+))");
        auto ml = re.match(l);
        auto mr = re.match(r);

        if (ml.hasMatch() && mr.hasMatch()) {
            return ml.captured(1).toInt() < mr.captured(1).toInt();
        }

        if (ml.hasMatch())
            return true;

        if (mr.hasMatch())
            return false;

        return QString::localeAwareCompare(l.toCaseFolded(), r.toCaseFolded()) < 0;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        const int columns = sourceModel()->columnCount(sourceParent);
        for (int col = 0; col < columns; ++col) {
            const QModelIndex idx = sourceModel()->index(sourceRow, col, sourceParent);
            if (sourceModel()->data(idx, Qt::DisplayRole).toString().contains(filterRegularExpression()))
                return true;
        }
        return false;
    }
};
