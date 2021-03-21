/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef ONLINEBALANCEPROXYMODEL_H
#define ONLINEBALANCEPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

class OnlineBalanceProxyModelPrivate;
class KMM_MODELS_EXPORT OnlineBalanceProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum Column {
        Symbol = 0,
        Quantity,
        Price,
        Value,
    };

    OnlineBalanceProxyModel(QObject *parent = nullptr);
    ~OnlineBalanceProxyModel();

    void setSourceModel ( QAbstractItemModel* sourceModel ) override;

    QVariant data(const QModelIndex& idx, int role) const override;

    int columnCount ( const QModelIndex& parent = QModelIndex() ) const override;

    Qt::ItemFlags flags ( const QModelIndex& index ) const override;

    QModelIndex index ( int row, int column, const QModelIndex & parent ) const override;

protected:
    bool filterAcceptsRow ( int source_row, const QModelIndex& source_parent ) const override;

private:
    void init();
    void load();

private:
    OnlineBalanceProxyModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(OnlineBalanceProxyModel);
};

#endif // ONLINEBALANCEPROXYMODEL_H
