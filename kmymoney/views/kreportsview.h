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

#include <QWidget>
#include <QList>
#include <QPrinter>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KChartAbstractCoordinatePlane>

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
class KReportsView : public KMyMoneyViewBase
{
  Q_OBJECT
public:

  /**
    * Helper class for KReportView.
    *
    * This is the widget which displays a single report in the TabWidget that comprises this view.
    *
    * @author Ace Jones
    */

  class KReportTab: public QWidget
  {
  private:
    #ifdef ENABLE_WEBENGINE
    QWebEngineView            *m_tableView;
    #else
    KWebView                  *m_tableView;
    #endif
    reports::KReportChartView *m_chartView;
    ReportControl             *m_control;
    QVBoxLayout               *m_layout;
    QPrinter                  *m_currentPrinter;
    MyMoneyReport m_report;
    bool m_deleteMe;
    bool m_chartEnabled;
    bool m_showingChart;
    bool m_needReload;
    bool m_isChartViewValid;
    bool m_isTableViewValid;
    QPointer<reports::ReportTable> m_table;

    /**
     * Users character set encoding.
     */
    QByteArray m_encoding;

  public:
    KReportTab(QTabWidget* parent, const MyMoneyReport& report, const KReportsView *eventHandler);
    ~KReportTab();
    const MyMoneyReport& report() const {
      return m_report;
    }
    void print();
    void toggleChart();
    /**
     * Updates information about ploted chart in report's data
     */
    void updateDataRange();
    void copyToClipboard();
    void saveAs(const QString& filename, bool includeCSS = false);
    void updateReport();
    QString createTable(const QString& links = QString());
    const ReportControl* control() const {
      return m_control;
    }
    bool isReadyToDelete() const {
      return m_deleteMe;
    }
    void setReadyToDelete(bool f) {
      m_deleteMe = f;
    }
    void modifyReport(const MyMoneyReport& report) {
      m_report = report;
    }
    void showEvent(QShowEvent * event);
    void loadTab();
  };

  /**
    * Helper class for KReportView.
    *
    * This is a named list of reports, which will be one section
    * in the list of default reports
    *
    * @author Ace Jones
    */
  class ReportGroup: public QList<MyMoneyReport>
  {
  private:
    QString m_name;     ///< the title of the group in non-translated form
    QString m_title;    ///< the title of the group in i18n-ed form
  public:
    ReportGroup() {}
    ReportGroup(const QString& name, const QString& title): m_name(name), m_title(title) {}
    const QString& name() const {
      return m_name;
    }
    const QString& title() const {
      return m_title;
    }
  };

private:
  bool m_needReload;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;

  QListWidget* m_reportListView;
  QTabWidget* m_reportTabWidget;
  QWidget* m_listTab;
  QVBoxLayout* m_listTabLayout;
  QTreeWidget* m_tocTreeWidget;
  QMap<QString, TocItemGroup*> m_allTocItemGroups;
  QString m_selectedExportFilter;

  bool m_columnsAlreadyAdjusted;

  void restoreTocExpandState(QMap<QString, bool>& expandStates);

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

  void setDefaultFocus();

  /**
    * Overridden so we can reload the view if necessary.
    *
    * @return Nothing.
    */
  void showEvent(QShowEvent * event);

protected:
  void addReportTab(const MyMoneyReport&);
  void loadView();
  static void defaultReports(QList<ReportGroup>&);
  bool columnsAlreadyAdjusted();
  void setColumnsAlreadyAdjusted(bool adjusted);

public Q_SLOTS:
  void slotOpenUrl(const QUrl &url);

  void slotLoadView();
  void slotPrintView();
  void slotCopyView();
  void slotSaveView();
  void slotConfigure();
  void slotDuplicate();
  void slotToggleChart();
  void slotItemDoubleClicked(QTreeWidgetItem* item, int);
  void slotOpenReport(const QString&);
  void slotOpenReport(const MyMoneyReport&);
  void slotReportAccountTransactions(const MyMoneyAccount& acc);
  void slotCloseCurrent();
  void slotClose(int index);
  void slotCloseAll();
  void slotDelete();
  void slotListContextMenu(const QPoint &);
  void slotOpenFromList();
  void slotConfigureFromList();
  void slotNewFromList();
  void slotDeleteFromList();

Q_SIGNALS:
  /**
    * This signal is emitted whenever a report is selected
    */
  void reportSelected(const MyMoneyReport&);

  /**
    * This signal is emitted whenever a transaction is selected
    */
  void ledgerSelected(const QString&, const QString&);

private:
  /**
    * Display a dialog to confirm report deletion
    */
  int deleteReportDialog(const QString&);

  /** Initializes page and sets its load status to initialized
   */
  void init();
};

#endif
