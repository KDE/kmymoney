/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KSCHEDULEDVIEW_H
#define KSCHEDULEDVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class QItemSelection;
class MyMoneySchedule;
class MyMoneyAccount;
class SelectedObjects;

namespace eDialogs {
enum class ScheduleResultCode;
}
namespace eView {
namespace Schedules {
enum class Requester;
}
}

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

    // TODO: remove that function
    /**
     * ugly proxy function
     */
    eDialogs::ScheduleResultCode enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys);

    void slotSettingsChanged() override;

public Q_SLOTS:
    void slotEditSchedule();

    void slotEnterOverdueSchedules(const MyMoneyAccount& acc);

    void updateActions(const SelectedObjects& selections) override;

Q_SIGNALS:
    void enterOverdueSchedulesFinished(eView::Schedules::Requester req);

protected:
    void showEvent(QShowEvent* event) override;

private:
    Q_DECLARE_PRIVATE(KScheduledView)

private Q_SLOTS:
    void slotListViewCollapsed(const QModelIndex& idx);
    void slotListViewExpanded(const QModelIndex& idx);

    void slotSetSelectedItem(const QItemSelection& selected, const QItemSelection& deselected);

    void slotNewSchedule();
    void slotDeleteSchedule();
    void slotDuplicateSchedule();
    void slotEnterSchedule();
    void slotSkipSchedule();
};

#endif
