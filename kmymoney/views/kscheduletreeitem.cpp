/***************************************************************************
                          kscheduletreeitem.cpp  -  description
                             -------------------
    begin                : Fri Jul 16 2010
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

#include "kscheduletreeitem.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

KScheduleTreeItem::KScheduleTreeItem(QTreeWidget* parent) : QTreeWidgetItem(parent)
{
}

KScheduleTreeItem::KScheduleTreeItem(QTreeWidgetItem* &parent) : QTreeWidgetItem(parent)
{
}

bool KScheduleTreeItem::operator<(const QTreeWidgetItem &otherItem) const
{
  bool result = false;
  int column = 0;
  if (!isFirstColumnSpanned()) {
    column = this->treeWidget()->sortColumn();
  } else {
    column = 0;
  }

  switch (column) {
    case 0: //schedule name or id
    case 1: //account name
    case 2: //payee
      result = data(column, OrderRole).toString() < otherItem.data(column, OrderRole).toString();
      break;
    case 3: //amount
      result = data(column, OrderRole).value<MyMoneyMoney>() < otherItem.data(column, OrderRole).value<MyMoneyMoney>();
      break;
    case 4: //next due date
      result = data(column, OrderRole).toDate() < otherItem.data(column, OrderRole).toDate();
      break;
    case 6: //payment method
    case 5: //occurrence
    default:
      result = text(column).toLower() < otherItem.text(column).toLower();
  }

  return result;
}
