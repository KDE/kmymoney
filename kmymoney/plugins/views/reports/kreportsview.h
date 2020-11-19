/***************************************************************************
                          kreportsview.h  -  description
                             -------------------
    begin                : Sat Mar 27 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.jones@hotpop.com>
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KREPORTSVIEW_H
#define KREPORTSVIEW_H

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

#ifdef _CHECK_MEMORY
#include "mymoneyutils.h"
#endif

#include "mymoneyreport.h"

namespace reports { class KReportChartView; }
namespace reports { class ReportTable; }

class QTreeWidget;
class QTreeWidgetItem;
class QListWidget;
class MyQWebEnginePage;
class TocItemGroup;
class ReportControl;
class ReportGroup;
class SelectedObjects;

#ifdef ENABLE_WEBENGINE
class QWebEngineView;
#else
class KWebView;
#endif

/**
  * Displays a page where reports can be placed.
  *
  * @author Ace Jones
  *
  * @short A view for reports.
**/
class KReportsViewPrivate;
class KReportsView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  /**
    * Standard constructor.
    *
    * @param parent The QWidget this is used in.
    * @param name The QT name.
    *
    * @return An object of type KReportsView
    *
    * @see ~KReportsView
    */
  explicit KReportsView(QWidget *parent = nullptr);
  ~KReportsView() override;

  void executeCustomAction(eView::Action action) override;
  void refresh();

Q_SIGNALS:
  /**
    * This signal is emitted whenever a report is selected
    */
  void reportSelected(const MyMoneyReport&);

  /**
    * This signal is emitted whenever a transaction is selected
    */
  void transactionSelected(const QString&, const QString&);

  void switchViewRequested(View view);

protected:
  /**
    * Overridden so we can reload the view if necessary.
    *
    * @return Nothing.
    */
  void showEvent(QShowEvent * event) override;

public Q_SLOTS:
  void slotOpenUrl(const QUrl &url);

  void slotPrintView();
  void slotCopyView();
  void slotSaveView();
  void slotConfigure();
  void slotDuplicate();
  void slotToggleChart();
  void slotItemDoubleClicked(QTreeWidgetItem* item, int);
  void slotOpenReport(const QString&);
  void slotOpenReport(const MyMoneyReport&);
  void slotCloseCurrent();
  void slotClose(int index);
  void slotCloseAll();
  void slotDelete();
  void slotListContextMenu(const QPoint &);
  void slotOpenFromList();
  void slotPrintFromList();
  void slotConfigureFromList();
  void slotNewFromList();
  void slotDeleteFromList();

  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;
  void slotSelectByVariant ( const QVariantList& args, eView::Intent intent ) override;

  void updateActions(const SelectedObjects& selections) override;

private:
  Q_DECLARE_PRIVATE(KReportsView)

private Q_SLOTS:
  /**
    * This slot creates a transaction report for the selected account
    * and opens it in the reports view.
    */
  void slotReportAccountTransactions();
};

#endif
