/*
    SPDX-FileCopyrightText: 2010 Alvaro Soliverez <asoliverez@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
