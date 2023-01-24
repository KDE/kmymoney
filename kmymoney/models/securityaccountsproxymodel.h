/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SECURITYACCOUNTSPROXYMODEL_H
#define SECURITYACCOUNTSPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_MODELS_EXPORT SecurityAccountsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SecurityAccountsProxyModel(QObject* parent = nullptr);
    ~SecurityAccountsProxyModel() = default;

    void setSourceModel(QAbstractItemModel* sourceModel) override;

    QVariant data(const QModelIndex& idx, int role) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

#endif // SECURITYACCOUNTSPROXYMODEL_H
