/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgersortorder.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "widgetenums.h"

LedgerSortOrder::LedgerSortOrder(const QString& sortOrder)
    : QList<ColumnSortItem>()
{
    setSortOrder(sortOrder);
}

LedgerSortOrder::LedgerSortOrder()
    : QList<ColumnSortItem>()
{
    ColumnSortItem defaultSortColumnItem({.sortRole = eMyMoney::Model::TransactionPostDateRole, .sortOrder = Qt::AscendingOrder});
    append(defaultSortColumnItem);
}

void LedgerSortOrder::setSortOrder(const QString& sortOrder)
{
    const QMap<eWidgets::SortField, int> sortFieldToColumn{
        {eWidgets::SortField::PostDate, eMyMoney::Model::TransactionPostDateRole},
        {eWidgets::SortField::EntryDate, eMyMoney::Model::TransactionEntryDateRole},
        {eWidgets::SortField::Payee, eMyMoney::Model::SplitPayeeRole},
        {eWidgets::SortField::Value, eMyMoney::Model::SplitSharesRole},
        {eWidgets::SortField::NoSort, eMyMoney::Model::SplitNumberRole},
        {eWidgets::SortField::EntryOrder, eMyMoney::Model::IdRole},
        {eWidgets::SortField::Category, eMyMoney::Model::TransactionCounterAccountRole},
        {eWidgets::SortField::ReconcileState, eMyMoney::Model::SplitReconcileFlagRole},
        {eWidgets::SortField::Security, eMyMoney::Model::JournalSplitSecurityNameRole},
        {eWidgets::SortField::Type, eMyMoney::Model::SplitSharesSuffixRole},
    };

    clear();
    const auto sortOrderList = sortOrder.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const auto& sortOrderEntry : sortOrderList) {
        const int numericEntry = sortOrderEntry.toInt();
        ColumnSortItem item = {.sortRole =
                                   sortFieldToColumn.value(static_cast<eWidgets::SortField>(qAbs(numericEntry)), eMyMoney::Model::TransactionPostDateRole),
                               .sortOrder = numericEntry >= 0 ? Qt::AscendingOrder : Qt::DescendingOrder};
        append(item);
    }
}
