/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SECURITIESFILTERPROXYMODEL_H
#define SECURITIESFILTERPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <KItemModels/KRecursiveFilterProxyModel>
#define QSortFilterProxyModel KRecursiveFilterProxyModel
#endif

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "securitiesmodel.h"

/**
 * This model provides a filter for security accounts.
 */
class KMM_MODELS_EXPORT SecuritiesFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SecuritiesFilterProxyModel(QObject *parent, SecuritiesModel *model);
    ~SecuritiesFilterProxyModel();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    class Private;
    Private* const d;

#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
    // provide the interface for backward compatibility
    void setRecursiveFilteringEnabled(bool enable) {
        Q_UNUSED(enable);
    }
#endif

};

#undef QSortFilterProxyModel
#endif // SECURITIESFILTERPROXYMODEL_H
