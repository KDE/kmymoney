/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "securityaccountsproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFont>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "journalmodel.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"

SecurityAccountsProxyModel::SecurityAccountsProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void SecurityAccountsProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    // make sure that data changes in the source model invalidate our filter
    connect(sourceModel, &QAbstractItemModel::dataChanged, this, [&]() {
        invalidateFilter();
    });
}

int SecurityAccountsProxyModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return MyMoneyFile::instance()->journalModel()->columnCount();
}

QModelIndex SecurityAccountsProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column >= AccountsModel::Column::MaxColumns) {
        column = AccountsModel::Column::MaxColumns - 1;
    }

    switch (column) {
    case JournalModel::Column::Detail:
        column = AccountsModel::Column::AccountName;
        break;
    default:
        break;
    }
    return QSortFilterProxyModel::index(row, column, parent);
}

QVariant SecurityAccountsProxyModel::data(const QModelIndex& idx, int role) const
{
    if (idx.column() >= AccountsModel::MaxColumns) {
        return {};
    }

    if (idx.isValid()) {
        switch (role) {
        case eMyMoney::Model::SecurityAccountNameEntryRole:
            return true;

        case eMyMoney::Model::JournalSplitSecurityNameRole:
            return QSortFilterProxyModel::data(idx, eMyMoney::Model::AccountNameRole);

        case Qt::DisplayRole:
            switch (idx.column()) {
            case JournalModel::Column::Detail:
            case AccountsModel::Column::AccountName:
                return QSortFilterProxyModel::data(idx, eMyMoney::Model::AccountNameRole);
            default:
                break;
            }
            return {};

        case eMyMoney::Model::SplitAccountIdRole:
            return QSortFilterProxyModel::data(idx, eMyMoney::Model::IdRole);

        case eMyMoney::Model::JournalSplitIdRole:
        case eMyMoney::Model::TransactionPostDateRole:
        case eMyMoney::Model::TransactionEntryDateRole:
        case Qt::ForegroundRole:
            return {};

        case Qt::FontRole: {
            QFont font;
            font.setBold(true);
            return font;
        }

        case Qt::TextAlignmentRole:
            switch (idx.column()) {
            case JournalModel::Column::Detail:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);

            default:
                break;
            }
            return {};

        case eMyMoney::Model::DelegateRole:
            return static_cast<int>(eMyMoney::Delegates::Types::SecurityAccountNameDelegate);
        }
    }
    return QSortFilterProxyModel::data(idx, role);
}

Qt::ItemFlags SecurityAccountsProxyModel::flags(const QModelIndex& idx) const
{
    Q_UNUSED(idx)
    return Qt::NoItemFlags;
}

bool SecurityAccountsProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    // never show a favorite entry
    auto idx = sourceModel()->index(source_row, 0, source_parent);
    if (idx.data(eMyMoney::Model::AccountIsFavoriteIndexRole).toBool() == true)
        return false;
    return true;
}
