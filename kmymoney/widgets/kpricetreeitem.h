/*
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

