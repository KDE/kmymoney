/*
    SPDX-FileCopyrightText: 2009 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTSVIEWPROXYMODEL_H
#define ACCOUNTSVIEWPROXYMODEL_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>
#include <QPoint>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsproxymodel.h"
#include "modelenums.h"

class QPoint;

/**
  * This model is specialized to organize the data for the accounts tree view
  * based on the data of the @ref AccountsModel.
  */
class AccountsViewProxyModelPrivate;
class KMM_WIDGETS_EXPORT AccountsViewProxyModel : public AccountsProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsViewProxyModel)

public:
    explicit AccountsViewProxyModel(QObject *parent = nullptr);
    ~AccountsViewProxyModel() override;

    void setColumnVisibility(eAccountsModel::Column column, bool visible);
    QSet<eAccountsModel::Column> getVisibleColumns();

public Q_SLOTS:
    void slotColumnsMenu(const QPoint);

Q_SIGNALS:
    void columnToggled(const eAccountsModel::Column column, const bool show);

protected:
    AccountsViewProxyModel(AccountsViewProxyModelPrivate &dd, QObject *parent);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

private:
    Q_DECLARE_PRIVATE(AccountsViewProxyModel)
};

#endif
