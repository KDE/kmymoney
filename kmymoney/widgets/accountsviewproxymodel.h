/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
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

#ifndef ACCOUNTVIEWPROXYMODEL_H
#define ACCOUNTVIEWPROXYMODEL_H

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
class AccountsViewProxyModel : public AccountsProxyModel
{
  Q_OBJECT

public:
  AccountsViewProxyModel(QObject *parent = nullptr);
  ~AccountsViewProxyModel();

  void setColumnVisibility(eAccountsModel::Column column, bool visible);
  QSet<eAccountsModel::Column> getVisibleColumns();

public slots:
  void slotColumnsMenu(const QPoint);

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

private:
  QSet<eAccountsModel::Column> m_visColumns;

signals:
  void columnToggled(const eAccountsModel::Column column, const bool show);
};

#endif
