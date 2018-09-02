/***************************************************************************
                          kscheduletreeitem.h  -  description
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


