/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "securitiesfilterproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class SecuritiesFilterProxyModel::Private
{
public:
    Private()
    {
    }

    ~Private() {}

};

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif

SecuritiesFilterProxyModel::SecuritiesFilterProxyModel(QObject *parent, SecuritiesModel *model)
    : QSortFilterProxyModel(parent), d(new Private)
{
    setRecursiveFilteringEnabled(true);
    setDynamicSortFilter(true);
    setFilterKeyColumn(-1);
    setSortLocaleAware(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(model);
}

#undef QSortFilterProxyModel

SecuritiesFilterProxyModel::~SecuritiesFilterProxyModel()
{
    delete d;
}

bool SecuritiesFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    return !sourceModel()->data(idx, eMyMoney::Model::Roles::IdRole).toString().isEmpty();
}
