/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEBANKINGACCOUNTSFILTERPROXYMODEL_H
#define ONLINEBANKINGACCOUNTSFILTERPROXYMODEL_H

#include "kmm_models_export.h"
#include <QSortFilterProxyModel>

class KMM_MODELS_EXPORT OnlineBankingAccountsFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit OnlineBankingAccountsFilterProxyModel(QObject* parent = 0);

    /**
     * @brief Makes accounts which do not support any onlineJob non-selectable
     */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const final override;

private:
    /**
     * @brief Has parent at least one visible child?
     */
    bool filterAcceptsParent(const QModelIndex& index) const;
};

#endif // ONLINEBANKINGACCOUNTSFILTERPROXYMODEL_H
