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

#ifndef KPRICETREEITEM_H
#define KPRICETREEITEM_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeWidgetItem>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


class KMM_WIDGETS_EXPORT KPriceTreeItem : public QTreeWidgetItem
{
public:
  explicit KPriceTreeItem(QTreeWidget* parent);

  bool operator<(const QTreeWidgetItem &otherItem) const final override;

  enum PriceItemDataRole {
    ScheduleIdRole = Qt::UserRole,
    OrderRole = Qt::UserRole + 1
  };

  enum ePriceColumns { ePriceCommodity = 0, ePriceStockName, ePriceCurrency, ePriceDate, ePricePrice, ePriceSource };

};

#endif // KPRICETREEITEM_H

