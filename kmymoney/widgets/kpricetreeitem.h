/***************************************************************************
                          kpricetreeitem.h  -  description
                             -------------------
    begin                : Sun Jul 18 2010
    copyright            : (C) 2010 by Alvaro Soliverez
    email                : asoliverez@kde.org
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KPRICETREEITEM_H
#define KPRICETREEITEM_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeWidgetItem>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


class KPriceTreeItem : public QTreeWidgetItem
{
public:
  explicit KPriceTreeItem(QTreeWidget* parent);

  bool operator<(const QTreeWidgetItem &otherItem) const;

  enum PriceItemDataRole {
    ScheduleIdRole = Qt::UserRole,
    OrderRole = Qt::UserRole + 1
  };

  enum ePriceColumns { ePriceCommodity = 0, ePriceStockName, ePriceCurrency, ePriceDate, ePricePrice, ePriceSource };

};

#endif // KPRICETREEITEM_H

