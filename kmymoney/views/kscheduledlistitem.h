/***************************************************************************
                          kscheduledlistitem.h  -  description
                             -------------------
    begin                : Sun Jan 27 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSCHEDULEDLISTITEM_H
#define KSCHEDULEDLISTITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyscheduled.h>

/**
  * The list view item that describes a scheduled transaction.
  *
  * @author Michael Edwardes
  */
class KScheduledListItem : public K3ListViewItem
{
public:
  /**
    * This constructor is used to create a child of the main list view widget.
    *
    * The child should be a descriptor for the schedule type and one of
    * Bill,
    * Deposit or
    * Transfer.
    *
    * Other types may be added in the future.
    *
    * @param parent The list view to be a child of.
    * @param description The (translated) description.
    * @param pixmap A pixmap for the entry
    * @param sortKey a sortkey, if empty, @c description will be used.
    *
    * @see MyMoneySchedule
    */
  KScheduledListItem(K3ListView *parent, const QString& description, const QPixmap& pixmap = QPixmap(), const QString& sortKey = QString());

  /**
    * This constructor is used to create a child of one of the children
    * created by the above method.
    *
    * This child describes a schedule and represents the data in schedule.
    *
    * @param parent The list view item to be a child of.
    * @param schedule The schedule to be represented.
    *
    * @see MyMoneySchedule
    */
  KScheduledListItem(KScheduledListItem *parent, const MyMoneySchedule& schedule/*, bool even*/);

  /**
    * Standard destructor.
    */
  ~KScheduledListItem();

  /**
    * Returns the schedule id for the instance being represented.  To be used
    * selection slots by the view.
    *
    * Returns an empty string for the top level items.
    *
    * @return The schedule id.
    */
  const QString& scheduleId(void) const {
    return m_schedule.id();
  }

  int compare(Q3ListViewItem* i, int col, bool ascending) const;

protected:
  void paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align);

private:
  MyMoneySchedule m_schedule;
  QString         m_sortKey;
  MyMoneyMoney    m_amount;
};

#endif
