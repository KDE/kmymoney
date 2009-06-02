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



// Some STL headers in GCC4.3 contain operator new. Memory checker mangles these
#ifdef _CHECK_MEMORY
  #undef new
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <q3valuevector.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3ValueList>

class Q3VBoxLayout;
class Q3ListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

#include <khtml_part.h>
#include <k3listview.h>
#include <ktabwidget.h>

// ----------------------------------------------------------------------------
// Project Includes
#ifdef _CHECK_MEMORY
  #include <mymoneyutils.h>
#endif

#include <mymoneyscheduled.h>
#include <mymoneyaccount.h>
#include <mymoneyreport.h>
#include "pivottable.h"
#include "querytable.h"
#include "../widgets/kmymoneyreportcontrolimpl.h"
//FIXME: Port to KDE4
//#include "kreportchartview.h"
#include "kmymoneyview.h"

class MyMoneyReport;





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
    KHTMLPart* m_part;
//FIXME: Port to KDE4
//    reports::KReportChartView* m_chartView;
    kMyMoneyReportControl* m_control;
    Q3VBoxLayout* m_layout;
    MyMoneyReport m_report;
    bool m_deleteMe;
    bool m_showingChart;
    bool m_needReload;
    reports::ReportTable* m_table;

  public:
    KReportTab(KTabWidget* parent, const MyMoneyReport& report );
    ~KReportTab();
    const MyMoneyReport& report(void) const { return m_report; }
    void print(void);
    void toggleChart(void);
    void copyToClipboard(void);
    void saveAs( const QString& filename, bool includeCSS = false );
    void updateReport(void);
    QString createTable(const QString& links=QString());
    const kMyMoneyReportControlDecl* control(void) const { return m_control; }
    bool isReadyToDelete(void) const { return m_deleteMe; }
    void setReadyToDelete(bool f) { m_deleteMe = f; }
    void modifyReport( const MyMoneyReport& report ) { m_report = report; }
    void show(void);
    void loadTab(void);
  };

  /**
    * Helper class for KReportView.
    *
    * Associates a report id with a list view item.
    *
    * @author Ace Jones
    */

  class KReportListItem: public K3ListViewItem
  {
  private:
    QString m_id;
    MyMoneyReport m_report;

  public:
    KReportListItem( K3ListView* parent, const MyMoneyReport& report ):
      K3ListViewItem( parent, report.name(), report.comment() ),
      m_id( report.id() ),
      m_report( report )
    {}
    KReportListItem( K3ListViewItem* parent, const MyMoneyReport& report ):
      K3ListViewItem( parent, report.name(), report.comment() ),
      m_id( report.id() ),
      m_report( report )
    {}
    //const QString& id(void) const { return m_id; }
    const MyMoneyReport& report(void) const { return m_report; }
  };

  class KReportGroupListItem: public K3ListViewItem
  {
  private:
    int m_nr;
    QString m_name;

  public:
    KReportGroupListItem( K3ListView* parent,const int nr,const QString name);
    virtual QString key ( int column, bool ascending ) const;
    void setNr(const int nr);
  };

  /**
    * Helper class for KReportView.
    *
    * This is a named list of reports, which will be one section
    * in the list of default reports
    *
    * @author Ace Jones
    */
  class ReportGroup: public Q3ValueList<MyMoneyReport>
  {
  private:
    QString m_name;     ///< the title of the group in non-translated form
    QString m_title;    ///< the title of the group in i18n-ed form
  public:
    ReportGroup( void ) {}
    ReportGroup( const QString& name, const QString& title ): m_name( name ), m_title(title) {}
    const QString& name( void ) const { return m_name; }
    const QString& title(void) const { return m_title; }
  };

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
  KTabWidget* m_reportTabWidget;
  K3ListView* m_reportListView;
  QWidget* m_listTab;
  Q3VBoxLayout* m_listTabLayout;
  bool m_needReload;

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
  KReportsView(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    *
    * @return Nothing.
    *
    * @see KReportsView
    */
  ~KReportsView();

  /**
    * Overridden so we can reload the view if necessary.
    *
    * @return Nothing.
    */
  void show();

protected:
  void addReportTab(const MyMoneyReport&);
  void loadView(void);
  static void defaultReports(Q3ValueList<ReportGroup>&);

public slots:
  void slotOpenUrl(const KUrl &url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments& browArgs);

  void slotLoadView(void);
  void slotPrintView(void);
  void slotCopyView(void);
  void slotSaveView(void);
  void slotConfigure(void);
  void slotDuplicate(void);
  void slotToggleChart(void);
  void slotOpenReport(Q3ListViewItem*);
  void slotOpenReport(const QString&);
  void slotOpenReport(const MyMoneyReport&);
  void slotCloseCurrent(void);
  void slotClose(QWidget*);
  void slotCloseAll(void);
  void slotDelete(void);
  void slotListContextMenu(K3ListView*,Q3ListViewItem*,const QPoint &);
  void slotOpenFromList(void);
  void slotConfigureFromList(void);
  void slotNewFromList(void);
  void slotDeleteFromList(void);

protected slots:
  void slotSaveFilterChanged(const QString&);

signals:
  /**
    * This signal is emitted whenever a report is selected
    */
  void reportSelected(const MyMoneyReport&);


};

#endif
