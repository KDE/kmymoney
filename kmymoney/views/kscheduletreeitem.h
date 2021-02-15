/***************************************************************************
                          kscheduletreeitem.h  -  description
                             -------------------
    begin                : Fri Jul 16 2010
    copyright            : (C) 2010 by Alvaro Soliverez
    email                : asoliverez@kde.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef KSCHEDULETREEITEM_H
#define KSCHEDULETREEITEM_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeWidgetItem>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


class KScheduleTreeItem : public QTreeWidgetItem
{
public:
  explicit KScheduleTreeItem(QTreeWidget* parent);

  explicit KScheduleTreeItem(QTreeWidgetItem* &parent);

  bool operator<(const QTreeWidgetItem &otherItem) const final override;

  enum ScheduleItemDataRole {
    ScheduleIdRole = Qt::UserRole,
    OrderRole = Qt::UserRole + 1
  };

};

#endif // KSCHEDULETREEITEM_H


