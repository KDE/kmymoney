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

};

#undef QSortFilterProxyModel
#endif // SECURITIESFILTERPROXYMODEL_H
