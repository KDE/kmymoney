/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017-2018  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

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
