/*
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

