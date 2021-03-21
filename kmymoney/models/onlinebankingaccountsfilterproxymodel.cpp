/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinebankingaccountsfilterproxymodel.h"

#include "mymoney/onlinejobadministration.h"
#include "mymoneyenums.h"
#include "accountsmodel.h"

OnlineBankingAccountsFilterProxyModel::OnlineBankingAccountsFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool OnlineBankingAccountsFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex sourceIndex = sourceModel()->index(source_row, AccountsModel::Column::AccountName, source_parent);
    const QString accountId = sourceModel()->data(sourceIndex, eMyMoney::Model::Roles::IdRole).toString();
    if (accountId.isEmpty())
        return false;
    else if (onlineJobAdministration::instance()->isAnyJobSupported(accountId))
        return true;

    return filterAcceptsParent(sourceIndex);
}

Qt::ItemFlags OnlineBankingAccountsFilterProxyModel::flags(const QModelIndex& index) const
{
    const QString accountId = sourceModel()->data(mapToSource(index), eMyMoney::Model::Roles::IdRole).toString();
    if (onlineJobAdministration::instance()->isAnyJobSupported(accountId))
        return QSortFilterProxyModel::flags(index);
    return QSortFilterProxyModel::flags(index) & ~Qt::ItemIsSelectable;
}


bool OnlineBankingAccountsFilterProxyModel::filterAcceptsParent(const QModelIndex& index) const
{
    auto const model = sourceModel();
    const auto rowCount = model->rowCount(index);
    for (auto i = 0; i < rowCount; ++i) {
        const auto childIndex = model->index(i, AccountsModel::Column::AccountName, index); // CAUTION! Assumption is being made that Account column number is always 0
        if (onlineJobAdministration::instance()->isAnyJobSupported(model->data(childIndex, eMyMoney::Model::Roles::IdRole).toString()))
            return true;
        if (filterAcceptsParent(childIndex))
            return true;
    }
    return false;
}
