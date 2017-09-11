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

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktreewidgetsearchline.h>
#include <ktreewidgetsearchlinewidget.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kscheduledviewdecl.h"
#include <mymoneyfile.h>
#include <mymoneyaccount.h>

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
class KScheduledView : public QWidget, private Ui::KScheduledViewDecl
{
  Q_OBJECT

public:
  /**
    * Standard constructor for QWidgets.
    */
  KScheduledView(QWidget *parent = 0);

  /**
    * Standard destructor.
    */
  ~KScheduledView();

  void setDefaultFocus();
  /**
    * Called by KMyMoneyView.
    */
  void showEvent(QShowEvent* event);

public slots:
  void slotSelectSchedule(const QString& schedule);
  void slotReloadView();

signals:
  void scheduleSelected(const MyMoneySchedule& schedule);
  void openContextMenu();
  void skipSchedule();
  void enterSchedule();
  void editSchedule();

  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

protected slots:
  /**
    * Shows the context menu when the user right clicks or presses
    * a 'windows' key when an item is selected.
    *
    * @param pos The position where to popup
    * @return none
  **/
  void slotListViewContextMenu(const QPoint& pos);

  void slotListItemExecuted(QTreeWidgetItem*, int);

  void slotAccountActivated();

  void slotListViewCollapsed(QTreeWidgetItem* item);
  void slotListViewExpanded(QTreeWidgetItem* item);

  void slotBriefSkipClicked(const MyMoneySchedule& schedule, const QDate&);
  void slotBriefEnterClicked(const MyMoneySchedule& schedule, const QDate&);

  void slotTimerDone();

  void slotSetSelectedItem();

  void slotRearrange();

protected:

  QTreeWidgetItem* addScheduleItem(QTreeWidgetItem* parent, MyMoneySchedule& schedule);

private:

  /// The selected schedule id in the list view.
  QString m_selectedSchedule;

  /// Read config file
  void readConfig();

  /// Write config file
  void writeConfig();

  /**
    * Refresh the view.
    */
  void refresh(bool full = true, const QString& schedId = QString());

  /**
    * Loads the accounts into the combo box.
    */
//  void loadAccounts();

  QMenu *m_kaccPopup;
  QStringList m_filterAccounts;
  bool m_openBills;
  bool m_openDeposits;
  bool m_openTransfers;
  bool m_openLoans;
  bool m_needReload;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;

  /**
   * Search widget for the list
   */
  KTreeWidgetSearchLineWidget*  m_searchWidget;

  /** Initializes page and sets its load status to initialized
   */
  void init();
};

#endif
