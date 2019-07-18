/*
 * Copyright 2009       Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
// class AccountsViewProxyModelPrivate;
// class KMM_WIDGETS_EXPORT AccountsViewProxyModel : public AccountsProxyModel
// {
//   Q_OBJECT
//   Q_DISABLE_COPY(AccountsViewProxyModel)
//
// public:
//   explicit AccountsViewProxyModel(QObject *parent = nullptr);
//   ~AccountsViewProxyModel() override;
//
//   void setColumnVisibility(eAccountsModel::Column column, bool visible);
//   QSet<eAccountsModel::Column> getVisibleColumns();
//
// public Q_SLOTS:
//   void slotColumnsMenu(const QPoint);
//
// Q_SIGNALS:
//   void columnToggled(const eAccountsModel::Column column, const bool show);
//
// protected:
//   AccountsViewProxyModel(AccountsViewProxyModelPrivate &dd, QObject *parent);
//   bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
//   bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
//
// private:
//   Q_DECLARE_PRIVATE(AccountsViewProxyModel)
// };

#endif
