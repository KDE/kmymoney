/***************************************************************************
                          kreportsview_p.h  -  description
                             -------------------
    begin                : Sat Mar 27 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
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

#ifndef KREPORTSVIEW_P_H
#define KREPORTSVIEW_P_H

#include "kreportsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFile>
#include <QTimer>
#include <QClipboard>
#include <QList>
#include <QVBoxLayout>
#include <QMimeData>
#include <QIcon>
#include <QUrlQuery>
#include <QFileInfo>
#include <QFileDialog>
#include <QLocale>
#include <QTextCodec>
#include <QMenu>
#include <QPointer>
#include <QWheelEvent>
#ifdef ENABLE_WEBENGINE
#include <QWebEngineView>
#else
#include <KWebView>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>
#include <KChartAbstractCoordinatePlane>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_reportcontrol.h"

#include "kmymoneyviewbase_p.h"
#include "kreportconfigurationfilterdlg.h"
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#include "mymoneyexception.h"
#include "kmymoneysettings.h"
#include "querytable.h"
#include "objectinfotable.h"
#include "icons/icons.h"
#include <kmymoneywebpage.h>
#include "tocitem.h"
#include "tocitemgroup.h"
#include "tocitemreport.h"
#include "kreportchartview.h"
#include "pivottable.h"
#include "reporttable.h"
#include "reportcontrolimpl.h"
#include "mymoneyenums.h"
#include "kmm_printer.h"

using namespace reports;
using namespace eMyMoney;
using namespace Icons;

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

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
  void showEvent(QShowEvent * event) final override;
  void loadTab();

protected:
  void wheelEvent(QWheelEvent *event) override;

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

/**
  * KReportTab Implementation
  */
KReportTab::KReportTab(QTabWidget* parent, const MyMoneyReport& report, const KReportsView* eventHandler):
    QWidget(parent),
    #ifdef ENABLE_WEBENGINE
    m_tableView(new QWebEngineView(this)),
    #else
    m_tableView(new KWebView(this)),
    #endif
    m_chartView(new KReportChartView(this)),
    m_control(new ReportControl(this)),
    m_layout(new QVBoxLayout(this)),
    m_report(report),
    m_deleteMe(false),
    m_chartEnabled(false),
    m_showingChart(report.isChartByDefault()),
    m_needReload(true),
    m_isChartViewValid(false),
    m_isTableViewValid(false),
    m_table(0)
{
  m_layout->setSpacing(6);
  m_tableView->setPage(new MyQWebEnginePage(m_tableView));
  m_tableView->setZoomFactor(KMyMoneySettings::zoomFactor());

  //set button icons
  m_control->ui->buttonChart->setIcon(Icons::get(Icon::OfficeChartLine));
  m_control->ui->buttonClose->setIcon(Icons::get(Icon::DocumentClose));
  m_control->ui->buttonConfigure->setIcon(Icons::get(Icon::Configure));
  m_control->ui->buttonCopy->setIcon(Icons::get(Icon::EditCopy));
  m_control->ui->buttonDelete->setIcon(Icons::get(Icon::EditDelete));
  m_control->ui->buttonExport->setIcon(Icons::get(Icon::DocumentExport));
  m_control->ui->buttonNew->setIcon(Icons::get(Icon::DocumentNew));

  m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_chartView->hide();
  m_tableView->hide();
  m_layout->addWidget(m_control);
  m_layout->addWidget(m_tableView);
  m_layout->addWidget(m_chartView);
  m_layout->setStretch(1, 10);
  m_layout->setStretch(2, 10);

  connect(m_control->ui->buttonChart, &QAbstractButton::clicked,
          eventHandler, &KReportsView::slotToggleChart);

  connect(m_control->ui->buttonConfigure, &QAbstractButton::clicked,
          eventHandler, &KReportsView::slotConfigure);

  connect(m_control->ui->buttonNew, &QAbstractButton::clicked,
          eventHandler, &KReportsView::slotDuplicate);

  connect(m_control->ui->buttonCopy, &QAbstractButton::clicked,
          eventHandler, &KReportsView::slotCopyView);

  connect(m_control->ui->buttonExport, &QAbstractButton::clicked,
          eventHandler, &KReportsView::slotSaveView);

  connect(m_control->ui->buttonDelete, &QAbstractButton::clicked,
          eventHandler, &KReportsView::slotDelete);

  connect(m_control->ui->buttonClose, &QAbstractButton::clicked,
          eventHandler, &KReportsView::slotCloseCurrent);

  #ifdef ENABLE_WEBENGINE
  connect(m_tableView->page(), &QWebEnginePage::urlChanged,
          eventHandler, &KReportsView::slotOpenUrl);
  #else
  m_tableView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  connect(m_tableView->page(), &KWebPage::linkClicked,
          eventHandler, &KReportsView::slotOpenUrl);
  #endif

  // if this is a default report, then you can't delete it!
  if (report.id().isEmpty())
    m_control->ui->buttonDelete->setEnabled(false);

  int tabNr = parent->addTab(this,
                         Icons::get(Icon::Spreadsheet),
                         report.name());
  parent->setTabEnabled(tabNr, true);
  parent->setCurrentIndex(tabNr);

  // get users character set encoding
  m_encoding = QTextCodec::codecForLocale()->name();
}

KReportTab::~KReportTab()
{
  delete m_table;
}


void KReportTab::wheelEvent(QWheelEvent* event)
{
  // Zoom text on Ctrl + Scroll
  if (event->modifiers() & Qt::CTRL) {
    if (!m_showingChart) {
      qreal factor = m_tableView->zoomFactor();
      if (event->delta() > 0)
        factor += 0.1;
      else if (event->delta() < 0)
        factor -= 0.1;
      m_tableView->setZoomFactor(factor);
      event->accept();
      return;
    }
  }
}


void KReportTab::print()
{
  if (m_tableView) {
    auto printer = KMyMoneyPrinter::startPrint();
    if (printer != nullptr) {
      if (m_showingChart) {
        QPainter painter(printer);
        m_chartView->paint(&painter, painter.window());
        QFont font = painter.font();
        font.setPointSizeF(font.pointSizeF() * 0.8);
        painter.setFont(font);
        QLocale locale;
        painter.drawText(0, 0, QDate::currentDate().toString(locale.dateFormat(QLocale::ShortFormat)));

        /// @todo extract url from KMyMoneyApp
        QUrl file;
        if (file.isValid()) {
          painter.drawText(0, painter.window().height(), file.toLocalFile());
        }
      } else {
    #ifdef ENABLE_WEBENGINE
        m_tableView->page()->print(printer, [=] (bool) {});
    #else
        m_tableView->print(printer);
    #endif
      }
    }
  }
}

void KReportTab::copyToClipboard()
{
  QMimeData* pMimeData =  new QMimeData();
  pMimeData->setHtml(m_table->renderReport(QLatin1String("html"), m_encoding, m_report.name(), true));
  QApplication::clipboard()->setMimeData(pMimeData);
}

void KReportTab::saveAs(const QString& filename, bool includeCSS)
{
  QFile file(filename);

  if (file.open(QIODevice::WriteOnly)) {
    if (QFileInfo(filename).suffix().toLower() == QLatin1String("csv")) {
      QTextStream(&file) << m_table->renderReport(QLatin1String("csv"), m_encoding, QString());
    } else {
      QString table =
        m_table->renderReport(QLatin1String("html"), m_encoding, m_report.name(), includeCSS);
      QTextStream stream(&file);
      stream << table;
    }
    file.close();
  }
}

void KReportTab::loadTab()
{
  m_needReload = true;
  if (isVisible()) {
    m_needReload = false;
    updateReport();
  }
}

void KReportTab::showEvent(QShowEvent * event)
{
  if (m_needReload) {
    m_needReload = false;
    updateReport();
  }
  QWidget::showEvent(event);
}

void KReportTab::updateReport()
{
  m_isChartViewValid = false;
  m_isTableViewValid = false;
  // reload the report from the engine. It might have
  // been changed by the user

  try {
    // Don't try to reload default reports from the engine
    if (!m_report.id().isEmpty())
      m_report = MyMoneyFile::instance()->report(m_report.id());
  } catch (const MyMoneyException &) {
  }

  delete m_table;
  m_table = 0;

  if (m_report.reportType() == eMyMoney::Report::ReportType::PivotTable) {
    m_table = new PivotTable(m_report);
    m_chartEnabled = true;
  } else if (m_report.reportType() == eMyMoney::Report::ReportType::QueryTable) {
    m_table = new QueryTable(m_report);
    m_chartEnabled = false;
  } else if (m_report.reportType() == eMyMoney::Report::ReportType::InfoTable) {
    m_table = new ObjectInfoTable(m_report);
    m_chartEnabled = false;
  }

  m_control->ui->buttonChart->setEnabled(m_chartEnabled);

  m_showingChart = !m_showingChart;
  toggleChart();
}

void KReportTab::toggleChart()
{
  // for now it will just SHOW the chart.  In the future it actually has to toggle it.

  if (m_showingChart) {
    if (!m_isTableViewValid) {
      m_tableView->setHtml(m_table->renderReport(QLatin1String("html"), m_encoding, m_report.name()),
                                               QUrl("file://")); // workaround for access permission to css file
    }
    m_isTableViewValid = true;
    m_tableView->show();
    m_chartView->hide();

    m_control->ui->buttonChart->setText(i18n("Chart"));
    m_control->ui->buttonChart->setToolTip(i18n("Show the chart version of this report"));
    m_control->ui->buttonChart->setIcon(Icons::get(Icon::OfficeChartLine));
  } else {
    if (!m_isChartViewValid)
      m_table->drawChart(*m_chartView);
    m_isChartViewValid = true;
    m_tableView->hide();
    m_chartView->show();

    m_control->ui->buttonChart->setText(i18n("Report"));
    m_control->ui->buttonChart->setToolTip(i18n("Show the report version of this chart"));
    m_control->ui->buttonChart->setIcon(Icons::get(Icon::ViewFinancialList));
  }
  m_showingChart = ! m_showingChart;
}

void KReportTab::updateDataRange()
{
  QList<DataDimension> grids = m_chartView->coordinatePlane()->gridDimensionsList();    // get dimmensions of ploted graph
  if (grids.isEmpty())
    return;
  QChar separator = locale().groupSeparator();
  QChar decimalPoint = locale().decimalPoint();
  int precision = m_report.yLabelsPrecision();
  QList<QPair<QString, qreal>> dims;  // create list of dimension values in string and qreal

  // get qreal values
  dims.append(qMakePair(QString(), grids.at(1).start));
  dims.append(qMakePair(QString(), grids.at(1).end));
  dims.append(qMakePair(QString(), grids.at(1).stepWidth));
  dims.append(qMakePair(QString(), grids.at(1).subStepWidth));

  // convert qreal values to string variables
  for (int i = 0; i < 4; ++i) {
    if (i > 2)
      ++precision;
    if (precision == 0)
      dims[i].first = locale().toString(qRound(dims.at(i).second));
    else
      dims[i].first = locale().toString(dims.at(i).second, 'f', precision).remove(separator).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\" + decimalPoint + "$"));
  }

  // save string variables in report's data
  m_report.setDataRangeStart(dims.at(0).first);
  m_report.setDataRangeEnd(dims.at(1).first);
  m_report.setDataMajorTick(dims.at(2).first);
  m_report.setDataMinorTick(dims.at(3).first);
}

class KReportsViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KReportsView)

public:
  explicit KReportsViewPrivate(KReportsView *qq):
    q_ptr(qq),
    m_needLoad(true),
    m_reportListView(nullptr),
    m_reportTabWidget(nullptr),
    m_listTab(nullptr),
    m_listTabLayout(nullptr),
    m_tocTreeWidget(nullptr),
    m_columnsAlreadyAdjusted(false)
  {
  }

  ~KReportsViewPrivate()
  {
  }

  void init()
  {
    Q_Q(KReportsView);
    m_needLoad = false;
    auto vbox = new QVBoxLayout(q);
    q->setLayout(vbox);
    vbox->setSpacing(6);
    vbox->setMargin(0);


    // build reports toc

    setColumnsAlreadyAdjusted(false);

    m_reportTabWidget = new QTabWidget(q);
    vbox->addWidget(m_reportTabWidget);
    m_reportTabWidget->setTabsClosable(true);

    m_listTab = new QWidget(m_reportTabWidget);
    m_listTabLayout = new QVBoxLayout(m_listTab);
    m_listTabLayout->setSpacing(6);

    m_tocTreeWidget = new QTreeWidget(m_listTab);

    // report-group items have only 1 column (name of group),
    // report items have 2 columns (report name and comment)
    m_tocTreeWidget->setColumnCount(2);

    // headers
    QStringList headers;
    headers << i18n("Reports") << i18n("Comment");
    m_tocTreeWidget->setHeaderLabels(headers);

    m_tocTreeWidget->setAlternatingRowColors(true);
    m_tocTreeWidget->setSortingEnabled(true);
    m_tocTreeWidget->sortByColumn(0, Qt::AscendingOrder);

    // for report group items:
    // doubleclick toggles the expand-state,
    m_tocTreeWidget->setExpandsOnDoubleClick(false);

    m_tocTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_tocTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_listTabLayout->addWidget(m_tocTreeWidget);
    m_reportTabWidget->addTab(m_listTab, i18n("Reports"));

    q->connect(m_reportTabWidget, &QTabWidget::tabCloseRequested,
            q, &KReportsView::slotClose);

    q->connect(m_tocTreeWidget, &QTreeWidget::itemDoubleClicked,
            q, &KReportsView::slotItemDoubleClicked);

    q->connect(m_tocTreeWidget, &QWidget::customContextMenuRequested,
            q, &KReportsView::slotListContextMenu);

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KReportsView::refresh);
  }

  void restoreTocExpandState(QMap<QString, bool>& expandStates)
  {
    for (auto i = 0; i < m_tocTreeWidget->topLevelItemCount(); ++i) {
      QTreeWidgetItem* item = m_tocTreeWidget->topLevelItem(i);

      if (item) {
        QString itemLabel = item->text(0);

        if (expandStates.contains(itemLabel)) {
          item->setExpanded(expandStates[itemLabel]);
        } else {
          item->setExpanded(false);
        }
      }
    }
  }

  /**
    * Display a dialog to confirm report deletion
    */
  int deleteReportDialog(const QString &reportName)
  {
    Q_Q(KReportsView);
    return KMessageBox::warningContinueCancel(q,
           i18n("<qt>Are you sure you want to delete report <b>%1</b>?  There is no way to recover it.</qt>",
                reportName), i18n("Delete Report?"));
  }

  void addReportTab(const MyMoneyReport& report)
  {
    Q_Q(KReportsView);
    new KReportTab(m_reportTabWidget, report, q);
  }

  void loadView()
  {
    // remember the id of the current selected item
    QTreeWidgetItem* item = m_tocTreeWidget->currentItem();
    QString selectedItem = (item) ? item->text(0) : QString();

    // save expand states of all top-level items
    QMap<QString, bool> expandStates;
    for (int i = 0; i < m_tocTreeWidget->topLevelItemCount(); ++i) {
      item = m_tocTreeWidget->topLevelItem(i);

      if (item) {
        QString itemLabel = item->text(0);

        if (item->isExpanded()) {
          expandStates.insert(itemLabel, true);
        } else {
          expandStates.insert(itemLabel, false);
        }
      }
    }

    // find the item visible on top
    QTreeWidgetItem* visibleTopItem = m_tocTreeWidget->itemAt(0, 0);

    // text of column 0 identifies the item visible on top
    QString visibleTopItemText;

    bool visibleTopItemFound = true;
    if (visibleTopItem == NULL) {
      visibleTopItemFound = false;
    } else {
      // this assumes, that all item-texts in column 0 are unique,
      // no matter, whether the item is a report- or a group-item
      visibleTopItemText = visibleTopItem->text(0);
    }

    // turn off updates to avoid flickering during reload
    //m_reportListView->setUpdatesEnabled(false);

    //
    // Rebuild the list page
    //
    m_tocTreeWidget->clear();

    // Default Reports
    QList<ReportGroup> defaultreports;
    defaultReports(defaultreports);

    QList<ReportGroup>::const_iterator it_group = defaultreports.constBegin();

    // the item to be set as current item
    QTreeWidgetItem* currentItem = 0L;

    // group number, this will be used as sort key for reportgroup items
    // we have:
    // 1st some default groups
    // 2nd a chart group
    // 3rd maybe a favorite group
    // 4th maybe an orphan group (for old reports)
    int defaultGroupNo = 1;
    int chartGroupNo = defaultreports.size() + 1;

    // group for diagrams
    QString groupName = I18N_NOOP("Charts");

    TocItemGroup* chartTocItemGroup =
      new TocItemGroup(m_tocTreeWidget, chartGroupNo,
                       i18n(groupName.toLatin1().data()));

    m_allTocItemGroups.insert(groupName, chartTocItemGroup);

    while (it_group != defaultreports.constEnd()) {
      groupName = (*it_group).name();

      TocItemGroup* defaultTocItemGroup =
        new TocItemGroup(m_tocTreeWidget, defaultGroupNo++,
                         i18n(groupName.toLatin1().data()));

      m_allTocItemGroups.insert(groupName, defaultTocItemGroup);

      if (groupName == selectedItem) {
        currentItem = defaultTocItemGroup;
      }

      QList<MyMoneyReport>::const_iterator it_report = (*it_group).begin();
      while (it_report != (*it_group).end()) {
        MyMoneyReport report = *it_report;
        report.setGroup(groupName);

        TocItemReport* reportTocItemReport =
          new TocItemReport(defaultTocItemGroup, report);

        if (report.name() == selectedItem) {
          currentItem = reportTocItemReport;
        }

        // ALSO place it into the Charts list if it's displayed as a chart by default
        if (report.isChartByDefault()) {
          new TocItemReport(chartTocItemGroup, report);
        }

        ++it_report;
      }

      ++it_group;
    }

    // group for custom (favorite) reports
    int favoriteGroupNo = chartGroupNo + 1;

    groupName = I18N_NOOP("Favorite Reports");

    TocItemGroup* favoriteTocItemGroup =
      new TocItemGroup(m_tocTreeWidget, favoriteGroupNo,
                       i18n(groupName.toLatin1().data()));

    m_allTocItemGroups.insert(groupName, favoriteTocItemGroup);

    TocItemGroup* orphanTocItemGroup = 0;

    QList<MyMoneyReport> customreports = MyMoneyFile::instance()->reportList();
    QList<MyMoneyReport>::const_iterator it_report = customreports.constBegin();
    while (it_report != customreports.constEnd()) {

      MyMoneyReport report = *it_report;

      groupName = (*it_report).group();

      // If this report is in a known group, place it there
      // KReportGroupListItem* groupnode = groupitems[(*it_report).group()];
      TocItemGroup* groupNode = m_allTocItemGroups[groupName];

      if (groupNode) {
        new TocItemReport(groupNode, report);
      } else {
        // otherwise, place it in the orphanage
        if (!orphanTocItemGroup) {

          // group for orphaned reports
          int orphanGroupNo = favoriteGroupNo + 1;

          groupName = I18N_NOOP("Old Customized Reports");

          orphanTocItemGroup =
            new TocItemGroup(m_tocTreeWidget, orphanGroupNo,
                             i18n(groupName.toLatin1().data()));
          m_allTocItemGroups.insert(groupName, orphanTocItemGroup);
        }
        new TocItemReport(orphanTocItemGroup, report);
      }

      // ALSO place it into the Favorites list if it's a favorite
      if ((*it_report).isFavorite()) {
        new TocItemReport(favoriteTocItemGroup, report);
      }

      // ALSO place it into the Charts list if it's displayed as a chart by default
      if ((*it_report).isChartByDefault()) {
        new TocItemReport(chartTocItemGroup, report);
      }

      ++it_report;
    }

    //
    // Go through the tabs to set their update flag or delete them if needed
    //

    int index = 1;
    while (index < m_reportTabWidget->count()) {
      // TODO: Find some way of detecting the file is closed and kill these tabs!!
      if (auto tab = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(index))) {
        if (tab->isReadyToDelete() /* || ! reports.count() */) {
          delete tab;
          --index;
        } else {
          tab->loadTab();
        }
      }
      ++index;
    }

    if (visibleTopItemFound) {
      // try to find the visibleTopItem that we had at the start of this method

      // intentionally not using 'Qt::MatchCaseSensitive' here
      // to avoid 'item not found' if someone corrected a typo only
      QList<QTreeWidgetItem*> visibleTopItemList =
        m_tocTreeWidget->findItems(visibleTopItemText,
                                   Qt::MatchFixedString |
                                   Qt::MatchRecursive);

      if (visibleTopItemList.isEmpty()) {
        // the item could not be found, it was deleted or renamed
        visibleTopItemFound = false;
      } else {
        visibleTopItem = visibleTopItemList.at(0);
        if (visibleTopItem == NULL) {
          visibleTopItemFound = false;
        }
      }
    }

    // adjust column widths,
    // but only the first time when the view is loaded,
    // maybe the user sets other column widths later,
    // so don't disturb him
    if (columnsAlreadyAdjusted()) {

      // restore expand states of all top-level items
      restoreTocExpandState(expandStates);

      // restore current item
      m_tocTreeWidget->setCurrentItem(currentItem);

      // try to scroll to the item visible on top
      // when this method started
      if (visibleTopItemFound) {
        m_tocTreeWidget->scrollToItem(visibleTopItem);
      } else {
        m_tocTreeWidget->scrollToTop();
      }
      return;
    }

    // avoid flickering
    m_tocTreeWidget->setUpdatesEnabled(false);

    // expand all top-level items
    m_tocTreeWidget->expandAll();

    // resize columns
    m_tocTreeWidget->resizeColumnToContents(0);
    m_tocTreeWidget->resizeColumnToContents(1);

    // restore expand states of all top-level items
    restoreTocExpandState(expandStates);

    // restore current item
    m_tocTreeWidget->setCurrentItem(currentItem);

    // try to scroll to the item visible on top
    // when this method started
    if (visibleTopItemFound) {
      m_tocTreeWidget->scrollToItem(visibleTopItem);
    } else {
      m_tocTreeWidget->scrollToTop();
    }

    setColumnsAlreadyAdjusted(true);

    m_tocTreeWidget->setUpdatesEnabled(true);
  }

  void defaultReports(QList<ReportGroup>& groups)
  {
    {
      ReportGroup list("Income and Expenses", i18n("Income and Expenses"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::ExpenseIncome,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::CurrentMonth,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Income and Expenses This Month"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::ExpenseIncome,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Income and Expenses This Year"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::ExpenseIncome,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Years),
                       TransactionFilter::Date::All,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Income and Expenses By Year"),
                       i18n("Default Report")
                     ));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::ExpenseIncome,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Last12Months,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Income and Expenses Graph"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);
      list.back().setChartDataLabels(false);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::ExpenseIncome,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::Group,
                       i18n("Income and Expenses Pie Chart"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartType(eMyMoney::Report::ChartType::Pie);
      list.back().setShowingRowTotals(false);

      groups.push_back(list);
    }
    {
      ReportGroup list("Net Worth", i18n("Net Worth"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Net Worth By Month"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Today,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Net Worth Today"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Years),
                       TransactionFilter::Date::All,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Net Worth By Year"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Next7Days,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("7-day Cash Flow Forecast"),
                       i18n("Default Report")
                     ));
      list.back().setIncludingSchedules(true);
      list.back().setColumnsAreDays(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Last12Months,
                       eMyMoney::Report::DetailLevel::Total,
                       i18n("Net Worth Graph"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Institution,
                       eMyMoney::Report::QueryColumn::None,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Account Balances by Institution"),
                       i18n("Default Report")
                     ));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AccountType,
                       eMyMoney::Report::QueryColumn::None,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Account Balances by Type"),
                       i18n("Default Report")
                     ));

      groups.push_back(list);
    }
    {
      ReportGroup list("Transactions", i18n("Transactions"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Account,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Tag | eMyMoney::Report::QueryColumn::Balance,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Transactions by Account"),
                       i18n("Default Report")
                     ));
      //list.back().setConvertCurrency(false);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Category,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account | eMyMoney::Report::QueryColumn::Tag,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Transactions by Category"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Payee,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Tag,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Transactions by Payee"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Tag,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Transactions by Tag"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Month,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Tag,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Transactions by Month"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Week,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Tag,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Transactions by Week"),
                       i18n("Default Report")
                     ));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Account,
                       eMyMoney::Report::QueryColumn::Loan,
                       TransactionFilter::Date::All,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Loan Transactions"),
                       i18n("Default Report")
                     ));
      list.back().setLoansOnly(true);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AccountReconcile,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Balance,
                       TransactionFilter::Date::Last3Months,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Transactions by Reconciliation Status"),
                       i18n("Default Report")
                     ));
      groups.push_back(list);
    }
    {
      ReportGroup list("CashFlow", i18n("Cash Flow"));
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::CashFlow,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Cash Flow Transactions This Month"),
                       i18n("Default Report")
                     ));
      groups.push_back(list);
    }
    {
      ReportGroup list("Investments", i18n("Investments"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::TopAccount,
                       eMyMoney::Report::QueryColumn::Action | eMyMoney::Report::QueryColumn::Shares | eMyMoney::Report::QueryColumn::Price,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Transactions"),
                       i18n("Default Report")
                     ));
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AccountByTopAccount,
                       eMyMoney::Report::QueryColumn::Shares | eMyMoney::Report::QueryColumn::Price,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Holdings by Account"),
                       i18n("Default Report")
                     ));
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::EquityType,
                       eMyMoney::Report::QueryColumn::Shares | eMyMoney::Report::QueryColumn::Price,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Holdings by Type"),
                       i18n("Default Report")
                     ));
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AccountByTopAccount,
                       eMyMoney::Report::QueryColumn::Performance,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Performance by Account"),
                       i18n("Default Report")
                     ));
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::EquityType,
                       eMyMoney::Report::QueryColumn::Performance,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Performance by Type"),
                       i18n("Default Report")
                     ));
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AccountByTopAccount,
                       eMyMoney::Report::QueryColumn::CapitalGain,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Capital Gains by Account"),
                       i18n("Default Report")
                     ));
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::EquityType,
                       eMyMoney::Report::QueryColumn::CapitalGain,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Capital Gains by Type"),
                       i18n("Default Report")
                     ));
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Today,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Holdings Pie"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Pie);
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Last12Months,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Worth Graph"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);
      list.back().setColumnsAreDays(true);
      list.back().setInvestmentsOnly(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Last12Months,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Price Graph"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);
      list.back().setColumnsAreDays(true);
      list.back().setInvestmentsOnly(true);
      list.back().setIncludingBudgetActuals(false);
      list.back().setIncludingPrice(true);
      list.back().setConvertCurrency(true);
      list.back().setChartDataLabels(false);
      list.back().setSkipZero(true);
      list.back().setShowingColumnTotals(false);
      list.back().setShowingRowTotals(false);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Last12Months,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Moving Average Price Graph"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);
      list.back().setColumnsAreDays(true);
      list.back().setInvestmentsOnly(true);
      list.back().setIncludingBudgetActuals(false);
      list.back().setIncludingAveragePrice(true);
      list.back().setMovingAverageDays(10);
      list.back().setConvertCurrency(true);
      list.back().setChartDataLabels(false);
      list.back().setShowingColumnTotals(false);
      list.back().setShowingRowTotals(false);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Last30Days,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Moving Average"),
                       i18n("Default Report")
                     ));
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);
      list.back().setColumnsAreDays(true);
      list.back().setInvestmentsOnly(true);
      list.back().setIncludingBudgetActuals(false);
      list.back().setIncludingMovingAverage(true);
      list.back().setMovingAverageDays(10);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Last30Days,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Investment Moving Average vs Actual"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);
      list.back().setColumnsAreDays(true);
      list.back().setInvestmentsOnly(true);
      list.back().setIncludingBudgetActuals(true);
      list.back().setIncludingMovingAverage(true);
      list.back().setMovingAverageDays(10);
      groups.push_back(list);
    }
    {
      ReportGroup list("Taxes", i18n("Taxes"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Category,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Tax Transactions by Category"),
                       i18n("Default Report")
                     ));
      list.back().setTax(true);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Payee,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Account,
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Tax Transactions by Payee"),
                       i18n("Default Report")
                     ));
      list.back().setTax(true);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Category,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account,
                       TransactionFilter::Date::LastFiscalYear,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Tax Transactions by Category Last Fiscal Year"),
                       i18n("Default Report")
                     ));
      list.back().setTax(true);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Payee,
                       eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Account,
                       TransactionFilter::Date::LastFiscalYear,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Tax Transactions by Payee Last Fiscal Year"),
                       i18n("Default Report")
                     ));
      list.back().setTax(true);
      groups.push_back(list);
    }
    {
      ReportGroup list("Budgeting", i18n("Budgeting"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::BudgetActual,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::YearToDate,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Budgeted vs. Actual This Year"),
                       i18n("Default Report")
                     ));
      list.back().setShowingRowTotals(true);
      list.back().setBudget("Any", true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::BudgetActual,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::YearToMonth,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Budgeted vs. Actual This Year (YTM)"),
                       i18n("Default Report")
                     ));
      list.back().setShowingRowTotals(true);
      list.back().setBudget("Any", true);
      // in case we're in January, we show the last year
      if (QDate::currentDate().month() == 1) {
        list.back().setDateFilter(TransactionFilter::Date::LastYear);
      }

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::BudgetActual,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::CurrentMonth,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Monthly Budgeted vs. Actual"),
                       i18n("Default Report")
                     ));
      list.back().setBudget("Any", true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::BudgetActual,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::CurrentYear,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Yearly Budgeted vs. Actual"),
                       i18n("Default Report")
                     ));
      list.back().setBudget("Any", true);
      list.back().setShowingRowTotals(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Budget,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::CurrentMonth,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Monthly Budget"),
                       i18n("Default Report")
                     ));
      list.back().setBudget("Any", false);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Budget,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::CurrentYear,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Yearly Budget"),
                       i18n("Default Report")
                     ));
      list.back().setBudget("Any", false);
      list.back().setShowingRowTotals(true);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::BudgetActual,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::CurrentYear,
                       eMyMoney::Report::DetailLevel::Group,
                       i18n("Yearly Budgeted vs Actual Graph"),
                       i18n("Default Report")
                     ));
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setBudget("Any", true);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);

      groups.push_back(list);
    }
    {
      ReportGroup list("Forecast", i18n("Forecast"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Next12Months,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Forecast By Month"),
                       i18n("Default Report")
                     ));
      list.back().setIncludingForecast(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::NextQuarter,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Forecast Next Quarter"),
                       i18n("Default Report")
                     ));
      list.back().setColumnsAreDays(true);
      list.back().setIncludingForecast(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::ExpenseIncome,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::CurrentYear,
                       eMyMoney::Report::DetailLevel::Top,
                       i18n("Income and Expenses Forecast This Year"),
                       i18n("Default Report")
                     ));
      list.back().setIncludingForecast(true);

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AssetLiability,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Next3Months,
                       eMyMoney::Report::DetailLevel::Total,
                       i18n("Net Worth Forecast Graph"),
                       i18n("Default Report")
                     ));
      list.back().setColumnsAreDays(true);
      list.back().setIncludingForecast(true);
      list.back().setChartByDefault(true);
      list.back().setChartCHGridLines(false);
      list.back().setChartSVGridLines(false);
      list.back().setChartType(eMyMoney::Report::ChartType::Line);
      groups.push_back(list);
    }
    {
      ReportGroup list("Information", i18n("General Information"));

      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Schedule,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Next12Months,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Schedule Information"),
                       i18n("Default Report")
                     ));
      list.back().setDetailLevel(eMyMoney::Report::DetailLevel::All);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::Schedule,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Next12Months,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Schedule Summary Information"),
                       i18n("Default Report")
                     ));
      list.back().setDetailLevel(eMyMoney::Report::DetailLevel::Top);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AccountInfo,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Today,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Account Information"),
                       i18n("Default Report")
                     ));
      list.back().setConvertCurrency(false);
      list.push_back(MyMoneyReport(
                       eMyMoney::Report::RowType::AccountLoanInfo,
                       static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                       TransactionFilter::Date::Today,
                       eMyMoney::Report::DetailLevel::All,
                       i18n("Loan Information"),
                       i18n("Default Report")
                     ));
      list.back().setConvertCurrency(false);
      groups.push_back(list);
    }
  }

  bool columnsAlreadyAdjusted() const
  {
    return m_columnsAlreadyAdjusted;
  }

  void setColumnsAlreadyAdjusted(bool adjusted)
  {
    m_columnsAlreadyAdjusted = adjusted;
  }

  KReportsView       *q_ptr;

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
  MyMoneyAccount m_currentAccount;
};

#endif
