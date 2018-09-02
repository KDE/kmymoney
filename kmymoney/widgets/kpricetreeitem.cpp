/*
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
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

#include "kpricetreeitem.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

KPriceTreeItem::KPriceTreeItem(QTreeWidget* parent) : QTreeWidgetItem(parent)
{
}

bool KPriceTreeItem::operator<(const QTreeWidgetItem &otherItem) const
{
  bool result = false;
  int column = 0;
  column = this->treeWidget()->sortColumn();

  switch (column) {
    case ePricePrice: //price
      result = data(column, OrderRole).value<MyMoneyMoney>() < otherItem.data(column, OrderRole).value<MyMoneyMoney>();
      break;
    case ePriceDate: //price date
      result = data(column, OrderRole).toDate() < otherItem.data(column, OrderRole).toDate();
      break;
    default:
      result = text(column).toLower() < otherItem.text(column).toLower();
  }

  return result;
}

