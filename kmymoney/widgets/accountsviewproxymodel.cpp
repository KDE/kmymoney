/*
    SPDX-FileCopyrightText: 2009 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "accountsviewproxymodel.h"
#include "accountsviewproxymodel_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMenu>
#include <QPoint>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "modelenums.h"

using namespace eAccountsModel;

AccountsViewProxyModel::AccountsViewProxyModel(QObject *parent) :
    AccountsProxyModel(*new AccountsViewProxyModelPrivate, parent)
{
    setFilterKeyColumn(-1); // set-up filter to search in all columns
}

AccountsViewProxyModel::AccountsViewProxyModel(AccountsViewProxyModelPrivate &dd, QObject *parent) :
    AccountsProxyModel(dd, parent)
{
}

AccountsViewProxyModel::~AccountsViewProxyModel()
{
}

/*
* Reimplemented to filter all but the account displayed in the accounts view.
*/
bool AccountsViewProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!source_parent.isValid()) {
        const auto data = sourceModel()->index(source_row, (int)(Column::Account), source_parent).data((int)Role::ID);
        if (data.isValid() && data.toString() == AccountsModel::favoritesAccountId)
            return false;
    }
    return AccountsProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool AccountsViewProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_D(const AccountsViewProxyModel);
    Q_UNUSED(source_parent)
    if (d->m_visColumns.contains(d->m_mdlColumns->at(source_column)))
        return true;
    return false;
}

QSet<Column> AccountsViewProxyModel::getVisibleColumns()
{
    Q_D(AccountsViewProxyModel);
    return d->m_visColumns;
}

void AccountsViewProxyModel::setColumnVisibility(Column column, bool show)
{
    Q_D(AccountsViewProxyModel);
    if (show)
        d->m_visColumns.insert(column);                                                  // add column to filter
    else
        d->m_visColumns.remove(column);
}

void AccountsViewProxyModel::slotColumnsMenu(const QPoint)
{
    Q_D(AccountsViewProxyModel);
    // construct all hideable columns list
    const QVector<Column> idColumns = { Column::Type, Column::Tax,
                                        Column::VAT, Column::CostCenter,
                                        Column::TotalBalance, Column::PostedValue,
                                        Column::TotalValue, Column::AccountNumber,
                                        Column::AccountSortCode
                                      };

    // create menu
    QMenu menu(i18n("Displayed columns"));
    QList<QAction *> actions;
    foreach (const auto idColumn, idColumns) {
        auto a = new QAction(nullptr);
        a->setObjectName(QString::number(static_cast<int>(idColumn)));
        a->setText(AccountsModel::getHeaderName(idColumn));
        a->setCheckable(true);
        a->setChecked(d->m_visColumns.contains(idColumn));
        actions.append(a);
    }
    menu.addActions(actions);

    // execute menu and get result
    const auto retAction = menu.exec(QCursor::pos());
    if (retAction) {
        const auto idColumn = static_cast<Column>(retAction->objectName().toInt());
        const auto isChecked = retAction->isChecked();
        setColumnVisibility(idColumn, isChecked);
        emit columnToggled(idColumn, isChecked);  // emit signal for method to possible load column into modela
        invalidateFilter();                       // refresh filter to reflect recent changes
    }
}
