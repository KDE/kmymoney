/***************************************************************************
                          kscheduledview.h  -  description
                             -------------------
    begin                : Sun Jan 27 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
#ifndef KSCHEDULEDVIEW_H
#define KSCHEDULEDVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class QTreeWidgetItem;
class KTreeWidgetSearchLineWidget;
class MyMoneySchedule;
class MyMoneyAccount;

namespace eDialogs { enum class ScheduleResultCode; }
namespace eView { namespace Schedules { enum class Requester; } }

/**
  * Contains all the scheduled transactions be they bills, deposits or transfers.
  * Encapsulates all the operations including adding, editing and deleting.
  * Used by the KMyMoneyView class to show the view.
  *
  * @author Michael Edwardes 2000-2002
  * $Id: kscheduledview.h,v 1.33 2009/03/01 19:13:08 ipwizard Exp $
  *
  * @short A class to encapsulate recurring transaction operations.
  */

class KScheduledViewPrivate;
class KScheduledView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  /**
    * Standard constructor for QWidgets.
    */
  explicit KScheduledView(QWidget *parent = nullptr);

  /**
    * Standard destructor.
    */
  ~KScheduledView() override;

  void executeCustomAction(eView::Action action) override;
  void refresh();
  void updateActions(const MyMoneyObject& obj);

  // TODO: remove that function
  /**
   * ugly proxy function
   */
  eDialogs::ScheduleResultCode enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys);

public Q_SLOTS:
  void slotSelectSchedule(const QString& schedule);
  void slotShowScheduleMenu(const MyMoneySchedule& sch);
  void slotEditSchedule();

  void slotEnterOverdueSchedules(const MyMoneyAccount& acc);

  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;

Q_SIGNALS:
  void enterOverdueSchedulesFinished(eView::Schedules::Requester req);

protected:
  void showEvent(QShowEvent* event) override;

private:
  Q_DECLARE_PRIVATE(KScheduledView)

private Q_SLOTS:

  void customContextMenuRequested(const QPoint);
  void slotListItemExecuted(QTreeWidgetItem*, int);

  void slotAccountActivated();

  void slotListViewCollapsed(QTreeWidgetItem* item);
  void slotListViewExpanded(QTreeWidgetItem* item);

  void slotTimerDone();

  void slotSetSelectedItem();

  void slotRearrange();

  void slotNewSchedule();
  void slotDeleteSchedule();
  void slotDuplicateSchedule();
  void slotEnterSchedule();
  void slotSkipSchedule();
};

#endif
