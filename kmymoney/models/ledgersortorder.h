/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERSORTORDER_H
#define LEDGERSORTORDER_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_MODELS_EXPORT ColumnSortItem
{
public:
    int sortRole;
    Qt::SortOrder sortOrder;

    inline bool operator==(const ColumnSortItem& right) const
    {
        return (sortRole == right.sortRole) && (sortOrder == right.sortOrder);
    }

    inline bool operator!=(const ColumnSortItem& right) const
    {
        return (sortRole != right.sortRole) || (sortOrder != right.sortOrder);
    }

    inline bool lessThanIs(bool result) const
    {
        return (sortOrder == Qt::AscendingOrder) ? result : !result;
    }
};

class KMM_MODELS_EXPORT LedgerSortOrder : public QList<ColumnSortItem>
{
public:
    LedgerSortOrder();
    explicit LedgerSortOrder(const QString& sortOrder);
    void setSortOrder(const QString& sortOrder);
};

#endif // LEDGERSORTORDER_H
