/*
    SPDX-FileCopyrightText: Bernd Gonsior <bernd.gonsior@googlemail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tocitem.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeWidget>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

TocItem::TocItem(QTreeWidget* parent, QStringList columns) :
    QTreeWidgetItem(parent, columns),
    type(GROUP)
{}

TocItem::TocItem(QTreeWidgetItem* parent, QStringList columns) :
    QTreeWidgetItem(parent, columns),
    type(GROUP)
{}

bool TocItem::isReport()
{
    return (type == REPORT ? true : false);
}

/**
 * @link tocitem.h @endlink
 */
bool TocItem::operator<(const QTreeWidgetItem &other)const
{

    // this operator is used by QTreeWidgetItem to sort items

    QString cm = "TocItem::<:";

    // the table of contents for reports (TOC) has 2 columns:
    // 1st: name of report or group
    // 2nd: comment

    // if user clicks column 'name',
    // TOC is sorted alphabetically
    // by the sort key provided
    // in the items data of column 0, role Qt::UserRole

    // if user clicks column 'comment',
    // TOC is sorted alphabetically
    // by the content of column 'comment'

    // get the column clicked by user
    int column = treeWidget()->sortColumn();

    // preset compare result
    bool compareResult = false;

    if (column != 0) {
        // user clicked column 'comment',
        // so sort alphabetically
        // by the content this column
        compareResult = this->text(column) < other.text(column);
        return compareResult;
    }

    // if there is any error condition in the following code,
    // we should get an information via stderr
    // or an alternative message handler,
    // but the program should not be interrupted,
    // the only result will be that the sort is wrong - bad luck

    // user clicked column 'name',
    // so retrieve the sort key

    // get the data of this item
    QVariant thisItemsData = this->data(0, Qt::UserRole);
    if (thisItemsData.isNull()) {
        qWarning() << cm << "thisItemsData is NULL";
        return false;
    }

    // get the data of the other item
    QVariant otherItemsData = other.data(0, Qt::UserRole);
    if (otherItemsData.isNull()) {
        qWarning() << cm << "otherItemsData is NULL";
        return false;
    }

    // get the QStringList of this item
    QStringList thisItemsDataList = thisItemsData.toStringList();

    // get the QStringList of the other item
    QStringList otherItemsDataList = otherItemsData.toStringList();

    // get the type (report or group) of this item
    // this information is only read as a precaution,
    // if it is sure, that always the same types are compared,
    // this can be removed
    QString thisItemsType = thisItemsDataList.at(0);
    if (thisItemsType.isNull()) {
        qWarning() << cm << "thisItemsType is NULL";
        return false;
    }

    // get the type (report or group) of the other item
    // this information is only read as a precaution,
    // if it is sure, that always the same types are compared,
    // this can be removed
    QString otherItemsType = otherItemsDataList.at(0);
    if (otherItemsType.isNull()) {
        qWarning() << cm << "otherItemsType is NULL";
        return false;
    }

    // get the sort key of this item
    // (to enable a pseudo-numeric sort,
    // in case of reportgroups this key
    // is the string-representation of a number
    // with leading zeros, e.g. '001')
    QString thisItemsSortKey = thisItemsDataList.at(1);
    if (thisItemsSortKey.isNull()) {
        qWarning() << cm << "thisItemsSortKey is NULL";
        return false;
    }

    // get the sort key of the other item
    QString otherItemsSortKey = otherItemsDataList.at(1);
    if (otherItemsSortKey.isNull()) {
        qWarning() << cm << "otherItemsSortKey is NULL";
        return false;
    }

    // this is a safety check only,
    // if it is sure, that always the same types are compared,
    // this can be removed
    if (thisItemsType != otherItemsType) {
        qWarning() << cm << "comparing different types: thisItemsType:"
                   << thisItemsType << ", otherItemsType:" << otherItemsType;
        return false;
    }

    // compare both sort keys
    compareResult = thisItemsSortKey < otherItemsSortKey;

    return compareResult;
}
