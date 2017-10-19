/***************************************************************************
                          kpricetreeitem.cpp  -  description
                             -------------------
    begin                : Sun Jul 18 2010
    copyright            : (C) 2010 by Alvaro Soliverez
    email                : asoliverez@kde.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

