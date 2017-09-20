/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ONLINEBANKINGACCOUNTSFILTERPROXYMODEL_H
#define ONLINEBANKINGACCOUNTSFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class OnlineBankingAccountsFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  OnlineBankingAccountsFilterProxyModel(QObject* parent = 0);
  
  /**
   * @brief Makes accounts which do not support any onlineJob non-selectable
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

protected:
  virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
  /**
   * @brief Has parent at least one visible child?
   */
  bool filterAcceptsParent(const QModelIndex& index) const;
};

#endif // ONLINEBANKINGACCOUNTSFILTERPROXYMODEL_H
