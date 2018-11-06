/***************************************************************************
                          kreportsview.cpp  -  description
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
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kreportsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QTimer>
#include <QClipboard>
#include <QList>
#include <QVBoxLayout>
#include <QVariant>
#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstandarddirs.h>
#include <khtmlview.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <mymoneyreport.h>
#include <kmymoneyglobalsettings.h>
#include "querytable.h"
#include "objectinfotable.h"
#include "kreportconfigurationfilterdlg.h"

using namespace reports;

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

/**
  * KReportsView::KReportTab Implementation
  */

KReportsView::KReportTab::KReportTab(KTabWidget* parent, const MyMoneyReport& report):
    QWidget(parent),
    m_part(new KHTMLPart(this)),
    m_chartView(new KReportChartView(this)),
    m_control(new kMyMoneyReportControl(this)),
    m_layout(new QVBoxLayout(this)),
    m_report(report),
    m_deleteMe(false),
    m_chartEnabled(false),
    m_showingChart(false),
    m_needReload(true),
    m_table(0)
{
  m_layout->setSpacing(6);
  m_part->setFontScaleFactor(KMyMoneyGlobalSettings::fontSizePercentage());

  //set button icons
  m_control->buttonChart->setIcon(KIcon("office-chart-line"));
  m_control->buttonClose->setIcon(KIcon("document-close"));
  m_control->buttonConfigure->setIcon(KIcon("configure"));
  m_control->buttonCopy->setIcon(KIcon("edit-copy"));
  m_control->buttonDelete->setIcon(KIcon("edit-delete"));
  m_control->buttonExport->setIcon(KIcon("document-export"));
  m_control->buttonNew->setIcon(KIcon("document-new"));

  m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_chartView->hide();
  m_layout->addWidget(m_control);
  m_layout->addWidget(m_part->view());
  m_layout->addWidget(m_chartView);

  parent->addTab(this, KIcon("application-vnd.oasis.opendocument.spreadsheet"), report.name());
  parent->setTabEnabled(parent->indexOf(this), true);

  // get users character set encoding
  m_encoding = KGlobal::locale()->encoding();
}

KReportsView::KReportTab::~KReportTab()
{
  delete m_table;
  //This is to prevent a crash on exit with KDE 4.3.2
  delete m_part;
}

void KReportsView::KReportTab::print()
{
  if (m_part && m_part->view())
    m_part->view()->print();
}

void KReportsView::KReportTab::copyToClipboard()
{
  QMimeData* pMimeData =  new QMimeData();
  pMimeData->setHtml(m_table->renderHTML(qobject_cast<QWidget*>(this),
                                         m_encoding, m_report.name(), true));
  QApplication::clipboard()->setMimeData(pMimeData);
}

void KReportsView::KReportTab::saveAs(const QString& filename, bool includeCSS)
{
  QFile file(filename);

  if (file.open(QIODevice::WriteOnly)) {
    if (QFileInfo(filename).suffix().toLower() == "csv") {
      QTextStream(&file) << m_table->renderCSV();
    } else {
      QString table =
        m_table->renderHTML(qobject_cast<QWidget*>(this), m_encoding,
                            m_report.name(), includeCSS);
      QTextStream stream(&file);
      stream << table;
    }
    file.close();
  }
}

void KReportsView::KReportTab::loadTab()
{
  m_needReload = true;
  if (isVisible()) {
    m_needReload = false;
    updateReport();
  }
}

void KReportsView::KReportTab::showEvent(QShowEvent * event)
{
  if (m_needReload) {
    m_needReload = false;
    updateReport();
  }
  QWidget::showEvent(event);
}

void KReportsView::KReportTab::updateReport()
{
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

  if (m_report.reportType() == MyMoneyReport::Report::PivotTable) {
    m_table = new PivotTable(m_report);
    m_chartEnabled = true;
  } else if (m_report.reportType() == MyMoneyReport::Report::QueryTable) {
    m_table = new QueryTable(m_report);
    m_chartEnabled = false;
  } else if (m_report.reportType() == MyMoneyReport::Report::InfoTable) {
    m_table = new ObjectInfoTable(m_report);
    m_chartEnabled = false;
  }

  m_part->begin();
  m_part->write(m_table->renderHTML(qobject_cast<QWidget*>(this),
                                    m_encoding, m_report.name()));
  m_part->end();

  m_table->drawChart(*m_chartView);
  m_control->buttonChart->setEnabled(m_chartEnabled);

  if (m_report.isChartByDefault() && !m_showingChart)
    toggleChart();
}

void KReportsView::KReportTab::toggleChart()
{
  // for now it will just SHOW the chart.  In the future it actually has to toggle it.

  if (m_showingChart) {
    m_part->view()->show();
    m_chartView->hide();

    m_control->buttonChart->setText(i18n("Chart"));
    m_control->buttonChart->setToolTip(i18n("Show the chart version of this report"));
    m_control->buttonChart->setIcon(KIcon("office-chart-line"));
  } else {
    m_part->view()->hide();

    m_chartView->show();

    m_control->buttonChart->setText(i18n("Report"));
    m_control->buttonChart->setToolTip(i18n("Show the report version of this chart"));
    m_control->buttonChart->setIcon(KIcon("view-financial-list"));
  }
  m_showingChart = ! m_showingChart;
}

/**
  * KReportsView Implementation
  */

class KReportsView::Private
{
public:

  Private() {
    fSavProps = new FileSaveProperties();
  }

  ~Private() {
    delete fSavProps;
  }

  class FileSaveProperties
  {
  public:
    FileSaveProperties() {
      includeCssCheckBox = 0;
      cbIsChecked = true;
      filter = "*.csv";
      filtCsv = "*.csv|" + i18nc("CSV (Filefilter)", "CSV files");
      filtHtml = "*.html|" + i18nc("HTML (Filefilter)", "HTML files");
    }

    /**
     * 'include css' checkbox for file save dialog.
     */
    QCheckBox* includeCssCheckBox;

    /**
     * Persistent checked-state of includeCssCheckBox.
     */
    bool cbIsChecked;

    /**
     * Current filtername of file save dialog
     * without description, e.g. @c '*.csv'
     */
    QString filter;

    /**
     * CSV filtername and description.
     *
     * @see KFileDialog::KFileDialog
     */
    QString filtCsv;

    /**
     * HTML filtername and description.
     *
     * @see KFileDialog::KFileDialog
     */
    QString filtHtml;
  };

  FileSaveProperties* fSavProps;
};

KReportsView::KReportsView(QWidget *parent, const char *name) :
    KMyMoneyViewBase(parent, name, i18n("Reports")),
    d(new Private),
    m_needReload(false)
{
  // build reports toc

  setColumnsAlreadyAdjusted(false);

  m_reportTabWidget = new KTabWidget(this);
  addWidget(m_reportTabWidget);
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
  // so avoid any further action in case of doubleclick
  // (see slotItemDoubleClicked)
  m_tocTreeWidget->setExpandsOnDoubleClick(false);

  m_tocTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  m_tocTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  m_listTabLayout->addWidget(m_tocTreeWidget);
  m_reportTabWidget->addTab(m_listTab, i18n("Reports"));

  connect(m_reportTabWidget, SIGNAL(tabCloseRequested(int)),
          this, SLOT(slotClose(int)));

  connect(m_tocTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
          this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));

  connect(m_tocTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(slotListContextMenu(QPoint)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadView()));
}

KReportsView::~KReportsView()
{
  delete d;
}

void KReportsView::showEvent(QShowEvent * event)
{
  emit aboutToShow();

  if (m_needReload) {
    loadView();
    m_needReload = false;
  }

  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());
  if (tab)
    emit reportSelected(tab->report());
  else
    emit reportSelected(MyMoneyReport());

  // don't forget base class implementation
  KMyMoneyViewBase::showEvent(event);
}

void KReportsView::slotLoadView()
{
  m_needReload = true;
  if (isVisible()) {
    loadView();
    m_needReload = false;
  }
}

void KReportsView::loadView()
{
  // remember the id of the current selected item
  QTreeWidgetItem* item = m_tocTreeWidget->currentItem();
  QString selectedItem = (item) ? item->text(0) : QString();

  // save expand states of all top-level items
  QMap<QString, bool> expandStates;
  for (int i = 0; i < m_tocTreeWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem* item = m_tocTreeWidget->topLevelItem(i);

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
    QString groupName = (*it_group).name();

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

    QString groupName = (*it_report).group();

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

        QString groupName = I18N_NOOP("Old Customized Reports");

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
    KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(index));
    if (tab->isReadyToDelete() /* || ! reports.count() */) {
      delete tab;
      --index;
    } else {
      tab->loadTab();
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

void KReportsView::slotOpenUrl(const KUrl &url, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)
{
  QString view = url.fileName();
  QString command = url.queryItem("command");
  QString id = url.queryItem("id");
  QString tid = url.queryItem("tid");

  if (view == VIEW_REPORTS) {

    if (command.isEmpty()) {
      // slotRefreshView();
    } else if (command == "print")
      slotPrintView();
    else if (command == "copy")
      slotCopyView();
    else if (command == "save")
      slotSaveView();
    else if (command == "configure")
      slotConfigure();
    else if (command == "duplicate")
      slotDuplicate();
    else if (command == "close")
      slotCloseCurrent();
    else if (command == "delete")
      slotDelete();
    else
      qWarning() << i18n("Unknown command '%1' in KReportsView::slotOpenUrl()", qPrintable(command));

  } else if (view == VIEW_LEDGER) {
    emit ledgerSelected(id, tid);
  } else {
    qWarning() << i18n("Unknown view '%1' in KReportsView::slotOpenUrl()", qPrintable(view));
  }
}

void KReportsView::slotPrintView()
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());
  if (tab)
    tab->print();
}

void KReportsView::slotCopyView()
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());
  if (tab)
    tab->copyToClipboard();
}

void KReportsView::slotSaveView()
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());
  if (tab) {
    d->fSavProps->includeCssCheckBox = new QCheckBox(i18n("Include Stylesheet"));
    // restore checkbox checked state
    d->fSavProps->includeCssCheckBox->setChecked(d->fSavProps->cbIsChecked);

    QString filterList = d->fSavProps->filtCsv + '\n' + d->fSavProps->filtHtml;
    QPointer<KFileDialog> dlg =
      new KFileDialog(KUrl("kfiledialog:///kmymoney-export"), filterList, this,
                      qobject_cast<QWidget*>(d->fSavProps->includeCssCheckBox));

    connect(dlg, SIGNAL(filterChanged(QString)),
            this, SLOT(slotSaveFilterChanged(QString)));

    dlg->setOperationMode(KFileDialog::Saving);
    dlg->setCaption(i18n("Export as"));

    // set current file filter && enable or disable includeCssCheckBox
    KFileFilterCombo* combo = dlg->filterWidget();
    if (d->fSavProps->filter == "*.csv") {
      combo->setCurrentFilter(d->fSavProps->filtCsv);
      d->fSavProps->includeCssCheckBox->setEnabled(false);
    } else {
      combo->setCurrentFilter(d->fSavProps->filtHtml);
      d->fSavProps->includeCssCheckBox->setEnabled(true);
    }

    if (dlg->exec() == QDialog::Accepted) {
      KUrl newURL = dlg->selectedUrl();
      if (!newURL.isEmpty()) {
        QString newName = newURL.pathOrUrl();

        if (newName.indexOf('.') == -1)
          newName.append(d->fSavProps->filter.remove('*'));

        // save checkbox checked state
        d->fSavProps->cbIsChecked = d->fSavProps->includeCssCheckBox->isChecked();

        try {
          tab->saveAs(newName, d->fSavProps->includeCssCheckBox->isEnabled()
                      && d->fSavProps->includeCssCheckBox->isChecked());
        } catch (const MyMoneyException &e) {
          KMessageBox::error(this, i18n("Failed to save: %1", e.what()));
        }
      }
    }
    delete d->fSavProps->includeCssCheckBox;
    delete dlg;
  }
}

void KReportsView::slotSaveFilterChanged(const QString& filter)
{
  d->fSavProps->filter = filter;

  if (filter ==  "*.html") {
    d->fSavProps->includeCssCheckBox->setEnabled(true);
  } else {
    d->fSavProps->includeCssCheckBox->setEnabled(false);
  }
}

void KReportsView::slotConfigure()
{
  QString cm = "KReportsView::slotConfigure";

  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());

  if (!tab) {
    // nothing to do
    return;
  }

  MyMoneyReport report = tab->report();
  if (report.comment() == i18n("Default Report") || report.comment() == i18n("Generated Report")) {
    report.setComment(i18n("Custom Report"));
    report.setName(i18n("%1 (Customized)", report.name()));
  }

  QPointer<KReportConfigurationFilterDlg> dlg = new KReportConfigurationFilterDlg(report);

  if (dlg->exec()) {
    MyMoneyReport newreport = dlg->getConfig();

    // If this report has an ID, then MODIFY it, otherwise ADD it
    MyMoneyFileTransaction ft;
    try {
      if (! newreport.id().isEmpty()) {
        MyMoneyFile::instance()->modifyReport(newreport);
        ft.commit();
        tab->modifyReport(newreport);

        m_reportTabWidget->setTabText(m_reportTabWidget->indexOf(tab), newreport.name());
        m_reportTabWidget->setCurrentIndex(m_reportTabWidget->indexOf(tab)) ;
      } else {
        MyMoneyFile::instance()->addReport(newreport);
        ft.commit();

        QString reportGroupName = newreport.group();

        // find report group
        TocItemGroup* tocItemGroup = m_allTocItemGroups[reportGroupName];
        if (!tocItemGroup) {
          QString error = i18n("Could not find reportgroup \"%1\" for report \"%2\".\nPlease report this error to the developer's list: kmymoney-devel@kde.org", reportGroupName, newreport.name());

          // write to messagehandler
          qWarning() << cm << error;

          // also inform user
          KMessageBox::error(m_reportTabWidget, error, i18n("Critical Error"));

          // cleanup
          delete dlg;

          return;
        }

        // do not add TocItemReport to TocItemGroup here,
        // this is done in loadView

        addReportTab(newreport);
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::error(this, i18n("Failed to configure report: %1", e.what()));
    }
  }
  delete dlg;
}

void KReportsView::slotDuplicate()
{
  QString cm = "KReportsView::slotDuplicate";

  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());

  if (!tab) {
    // nothing to do
    return;
  }

  MyMoneyReport dupe = tab->report();
  dupe.setName(i18n("Copy of %1", dupe.name()));
  if (dupe.comment() == i18n("Default Report"))
    dupe.setComment(i18n("Custom Report"));
  dupe.clearId();

  QPointer<KReportConfigurationFilterDlg> dlg = new KReportConfigurationFilterDlg(dupe);
  if (dlg->exec()) {
    MyMoneyReport newReport = dlg->getConfig();
    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->addReport(newReport);
      ft.commit();

      QString reportGroupName = newReport.group();

      // find report group
      TocItemGroup* tocItemGroup = m_allTocItemGroups[reportGroupName];
      if (!tocItemGroup) {
        QString error = i18n("Could not find reportgroup \"%1\" for report \"%2\".\nPlease report this error to the developer's list: kmymoney-devel@kde.org", reportGroupName, newReport.name());

        // write to messagehandler
        qWarning() << cm << error;

        // also inform user
        KMessageBox::error(m_reportTabWidget, error, i18n("Critical Error"));

        // cleanup
        delete dlg;

        return;
      }

      // do not add TocItemReport to TocItemGroup here,
      // this is done in loadView

      addReportTab(newReport);
    } catch (const MyMoneyException &e) {
      QString error = i18n("Cannot add report, reason: \"%1\"", e.what());

      // write to messagehandler
      qWarning() << cm << error;

      // also inform user
      KMessageBox::error(m_reportTabWidget, error, i18n("Critical Error"));
    }
  }
  delete dlg;
}

void KReportsView::slotDelete()
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());
  if (!tab) {
    // nothing to do
    return;
  }

  MyMoneyReport report = tab->report();
  if (! report.id().isEmpty()) {
    if (KMessageBox::Continue == deleteReportDialog(report.name())) {
      // close the tab and then remove the report so that it is not
      // generated again during the following loadView() call
      slotClose(m_reportTabWidget->currentIndex());

      MyMoneyFileTransaction ft;
      MyMoneyFile::instance()->removeReport(report);
      ft.commit();
    }
  } else {
    KMessageBox::information(this,
                             QString("<qt>") +
                             i18n("<b>%1</b> is a default report, so it cannot be deleted.",
                                  report.name()) + QString("</qt>"), i18n("Delete Report?"));
  }
}

int KReportsView::deleteReportDialog(const QString &reportName)
{
  return KMessageBox::warningContinueCancel(this, QString("<qt>") +
         i18n("Are you sure you want to delete report <b>%1</b>?  There is no way to recover it.",
              reportName) + QString("</qt>"), i18n("Delete Report?"));
}

void KReportsView::slotOpenReport(const QString& id)
{
  if (id.isEmpty()) {
    // nothing to  do
    return;
  }

  KReportTab* page = 0;

  // Find the tab which contains the report
  int index = 1;
  while (index < m_reportTabWidget->count()) {
    KReportTab* current = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(index));

    if (current->report().id() == id) {
      page = current;
      break;
    }

    ++index;
  }

  // Show the tab, or create a new one, as needed
  if (page)
    m_reportTabWidget->setCurrentIndex(m_reportTabWidget->indexOf(page));
  else
    addReportTab(MyMoneyFile::instance()->report(id));
}

void KReportsView::slotOpenReport(const MyMoneyReport& report)
{
  kDebug(2) << Q_FUNC_INFO << " " << report.name();
  KReportTab* page = 0;

  // Find the tab which contains the report indicated by this list item
  int index = 1;
  while (index < m_reportTabWidget->count()) {
    KReportTab* current = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(index));

    if (current->report().name() == report.name()) {
      page = current;
      break;
    }

    ++index;
  }

  // Show the tab, or create a new one, as needed
  if (page)
    m_reportTabWidget->setCurrentIndex(m_reportTabWidget->indexOf(page));
  else
    addReportTab(report);

}

void KReportsView::slotItemDoubleClicked(QTreeWidgetItem* item, int)
{
  TocItem* tocItem = dynamic_cast<TocItem*>(item);
  if (!tocItem->isReport()) {
    // toggle the expanded-state for reportgroup-items
    item->setExpanded(item->isExpanded() ? false : true);

    // nothing else to do for reportgroup-items
    return;
  }

  TocItemReport* reportTocItem = dynamic_cast<TocItemReport*>(tocItem);

  MyMoneyReport& report = reportTocItem->getReport();

  KReportTab* page = 0;

  // Find the tab which contains the report indicated by this list item
  int index = 1;
  while (index < m_reportTabWidget->count()) {
    KReportTab* current = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(index));

    // If this report has an ID, we'll use the ID to match
    if (! report.id().isEmpty()) {
      if (current->report().id() == report.id()) {
        page = current;
        break;
      }
    }
    // Otherwise, use the name to match.  THIS ASSUMES that no 2 default reports
    // have the same name...but that would be pretty a boneheaded thing to do.
    else {
      if (current->report().name() == report.name()) {
        page = current;
        break;
      }
    }

    ++index;
  }

  // Show the tab, or create a new one, as needed
  if (page)
    m_reportTabWidget->setCurrentIndex(m_reportTabWidget->indexOf(page));
  else
    addReportTab(report);
}

void KReportsView::slotToggleChart()
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentWidget());

  if (tab)
    tab->toggleChart();
}

void KReportsView::slotCloseCurrent()
{
  slotClose(m_reportTabWidget->currentIndex());
}

void KReportsView::slotClose(int index)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(index));
  if (tab) {
    m_reportTabWidget->removeTab(index);
    tab->setReadyToDelete(true);
  }
}


void KReportsView::slotCloseAll()
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(1));
  while (tab) {
    m_reportTabWidget->removeTab(m_reportTabWidget->indexOf(tab));
    tab->setReadyToDelete(true);

    tab = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(1));
  }
}

void KReportsView::addReportTab(const MyMoneyReport& report)
{
  KReportTab* tab = new KReportTab(m_reportTabWidget, report);

  connect(tab->control()->buttonChart, SIGNAL(clicked()),
          this, SLOT(slotToggleChart()));

  connect(tab->control()->buttonConfigure, SIGNAL(clicked()),
          this, SLOT(slotConfigure()));

  connect(tab->control()->buttonNew, SIGNAL(clicked()),
          this, SLOT(slotDuplicate()));

  connect(tab->control()->buttonCopy, SIGNAL(clicked()),
          this, SLOT(slotCopyView()));

  connect(tab->control()->buttonExport, SIGNAL(clicked()),
          this, SLOT(slotSaveView()));

  connect(tab->control()->buttonDelete, SIGNAL(clicked()),
          this, SLOT(slotDelete()));

  connect(tab->control()->buttonClose, SIGNAL(clicked()),
          this, SLOT(slotCloseCurrent()));

  connect(tab->browserExtenstion(), SIGNAL(openUrlRequest(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
          this, SLOT(slotOpenUrl(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)));

  // if this is a default report, then you can't delete it!
  if (report.id().isEmpty())
    tab->control()->buttonDelete->setEnabled(false);

  m_reportTabWidget->setCurrentIndex(m_reportTabWidget->indexOf(tab));
}

void KReportsView::slotListContextMenu(const QPoint & p)
{
  QTreeWidgetItem *item = m_tocTreeWidget->itemAt(p);

  if (!item) {
    return;
  }

  TocItem* tocItem = dynamic_cast<TocItem*>(item);

  if (!tocItem->isReport()) {
    // currently there is no context menu for reportgroup items
    return;
  }

  KMenu* contextmenu = new KMenu(this);

  contextmenu->addAction(i18nc("To open a new report", "&Open"),
                         this, SLOT(slotOpenFromList()));

  contextmenu->addAction(i18nc("Configure a report", "&Configure"),
                         this, SLOT(slotConfigureFromList()));

  contextmenu->addAction(i18n("&New report"),
                         this, SLOT(slotNewFromList()));

  // Only add this option if it's a custom report. Default reports cannot be deleted
  TocItemReport* reportTocItem = dynamic_cast<TocItemReport*>(tocItem);
  MyMoneyReport& report = reportTocItem->getReport();
  if (! report.id().isEmpty()) {
    contextmenu->addAction(i18n("&Delete"),
                           this, SLOT(slotDeleteFromList()));
  }

  contextmenu->popup(m_tocTreeWidget->mapToGlobal(p));
}

void KReportsView::slotOpenFromList()
{
  TocItem* tocItem = dynamic_cast<TocItem*>(m_tocTreeWidget->currentItem());

  if (tocItem)
    slotItemDoubleClicked(tocItem, 0);
}

void KReportsView::slotConfigureFromList()
{
  TocItem* tocItem = dynamic_cast<TocItem*>(m_tocTreeWidget->currentItem());

  if (tocItem) {
    slotItemDoubleClicked(tocItem, 0);
    slotConfigure();
  }
}

void KReportsView::slotNewFromList()
{
  TocItem* tocItem = dynamic_cast<TocItem*>(m_tocTreeWidget->currentItem());

  if (tocItem) {
    slotItemDoubleClicked(tocItem, 0);
    slotDuplicate();
  }
}

void KReportsView::slotDeleteFromList()
{
  TocItem* tocItem = dynamic_cast<TocItem*>(m_tocTreeWidget->currentItem());

  if (tocItem) {
    TocItemReport* reportTocItem = dynamic_cast<TocItemReport*>(tocItem);

    MyMoneyReport& report = reportTocItem->getReport();

    // If this report does not have an ID, it's a default report and cannot be deleted
    if (! report.id().isEmpty() &&
        KMessageBox::Continue == deleteReportDialog(report.name())) {
      // check if report's tab is open; start from 1 because 0 is toc tab
      for (int i = 1; i < m_reportTabWidget->count(); ++i) {
        KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->widget(i));
        if (tab->report().id() == report.id()) {
          slotClose(i); // if open, close it, so no crash when switching to it
          break;
        }
      }
      MyMoneyFileTransaction ft;
      MyMoneyFile::instance()->removeReport(report);
      ft.commit();
    }
  }
}

void KReportsView::defaultReports(QList<ReportGroup>& groups)
{
  {
    ReportGroup list("Income and Expenses", i18n("Income and Expenses"));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::ExpenseIncome,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::currentMonth,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Income and Expenses This Month"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::ExpenseIncome,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Income and Expenses This Year"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::ExpenseIncome,
                     MyMoneyReport::Column::Years,
                     MyMoneyTransactionFilter::allDates,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Income and Expenses By Year"),
                     i18n("Default Report")
                   ));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::ExpenseIncome,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::last12Months,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Income and Expenses Graph"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartType(MyMoneyReport::Chart::Line);
    list.back().setChartDataLabels(false);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::ExpenseIncome,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::Group,
                     i18n("Income and Expenses Pie Chart"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartType(MyMoneyReport::Chart::Pie);
    list.back().setShowingRowTotals(false);

    groups.push_back(list);
  }
  {
    ReportGroup list("Net Worth", i18n("Net Worth"));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Net Worth By Month"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::today,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Net Worth Today"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Years,
                     MyMoneyTransactionFilter::allDates,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Net Worth By Year"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::next7Days,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("7-day Cash Flow Forecast"),
                     i18n("Default Report")
                   ));
    list.back().setIncludingSchedules(true);
    list.back().setColumnsAreDays(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::last12Months,
                     MyMoneyReport::DetailLevel::Total,
                     i18n("Net Worth Graph"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Line);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Institution,
                     MyMoneyReport::QueryColumns::None,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Account Balances by Institution"),
                     i18n("Default Report")
                   ));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AccountType,
                     MyMoneyReport::QueryColumns::None,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Account Balances by Type"),
                     i18n("Default Report")
                   ));

    groups.push_back(list);
  }
  {
    ReportGroup list("Transactions", i18n("Transactions"));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Account,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Tag | MyMoneyReport::QueryColumns::Balance,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Transactions by Account"),
                     i18n("Default Report")
                   ));
    //list.back().setConvertCurrency(false);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Category,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Account | MyMoneyReport::QueryColumns::Tag,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Transactions by Category"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Payee,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Tag,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Transactions by Payee"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Tag,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Category,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Transactions by Tag"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Month,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Tag,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Transactions by Month"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Week,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Tag,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Transactions by Week"),
                     i18n("Default Report")
                   ));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Account,
                     MyMoneyReport::QueryColumns::Loan,
                     MyMoneyTransactionFilter::allDates,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Loan Transactions"),
                     i18n("Default Report")
                   ));
    list.back().setLoansOnly(true);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AccountReconcile,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Balance,
                     MyMoneyTransactionFilter::last3Months,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Transactions by Reconciliation Status"),
                     i18n("Default Report")
                   ));
    groups.push_back(list);
  }
  {
    ReportGroup list("CashFlow", i18n("Cash Flow"));
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::CashFlow,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Account,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Cash Flow Transactions This Month"),
                     i18n("Default Report")
                   ));
    groups.push_back(list);
  }
  {
    ReportGroup list("Investments", i18n("Investments"));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::TopAccount,
                     MyMoneyReport::QueryColumns::Action | MyMoneyReport::QueryColumns::Shares | MyMoneyReport::QueryColumns::Price,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Transactions"),
                     i18n("Default Report")
                   ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AccountByTopAccount,
                     MyMoneyReport::QueryColumns::Shares | MyMoneyReport::QueryColumns::Price,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Holdings by Account"),
                     i18n("Default Report")
                   ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::EquityType,
                     MyMoneyReport::QueryColumns::Shares | MyMoneyReport::QueryColumns::Price,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Holdings by Type"),
                     i18n("Default Report")
                   ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AccountByTopAccount,
                     MyMoneyReport::QueryColumns::Performance,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Performance by Account"),
                     i18n("Default Report")
                   ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::EquityType,
                     MyMoneyReport::QueryColumns::Performance,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Performance by Type"),
                     i18n("Default Report")
                   ));
    list.back().setInvestmentsOnly(true);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::today,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Holdings Pie"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Pie);
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::last12Months,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Worth Graph"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Line);
    list.back().setColumnsAreDays(true);
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::last12Months,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Price Graph"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Line);
    list.back().setColumnsAreDays(true);
    list.back().setInvestmentsOnly(true);
    list.back().setIncludingBudgetActuals(false);
    list.back().setIncludingPrice(true);
    list.back().setConvertCurrency(true);
    list.back().setChartDataLabels(false);
    list.back().setSkipZero(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::last12Months,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Moving Average Price Graph"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Line);
    list.back().setColumnsAreDays(true);
    list.back().setInvestmentsOnly(true);
    list.back().setIncludingBudgetActuals(false);
    list.back().setIncludingAveragePrice(true);
    list.back().setMovingAverageDays(10);
    list.back().setConvertCurrency(true);
    list.back().setChartDataLabels(false);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::last30Days,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Moving Average"),
                     i18n("Default Report")
                   ));
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Line);
    list.back().setColumnsAreDays(true);
    list.back().setInvestmentsOnly(true);
    list.back().setIncludingBudgetActuals(false);
    list.back().setIncludingMovingAverage(true);
    list.back().setMovingAverageDays(10);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::last30Days,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Investment Moving Average vs Actual"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Line);
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
                     MyMoneyReport::Row::Category,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Account,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Tax Transactions by Category"),
                     i18n("Default Report")
                   ));
    list.back().setTax(true);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Payee,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Account,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Tax Transactions by Payee"),
                     i18n("Default Report")
                   ));
    list.back().setTax(true);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Category,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Account,
                     MyMoneyTransactionFilter::lastFiscalYear,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Tax Transactions by Category Last Fiscal Year"),
                     i18n("Default Report")
                   ));
    list.back().setTax(true);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Payee,
                     MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Account,
                     MyMoneyTransactionFilter::lastFiscalYear,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Tax Transactions by Payee Last Fiscal Year"),
                     i18n("Default Report")
                   ));
    list.back().setTax(true);
    groups.push_back(list);
  }
  {
    ReportGroup list("Budgeting", i18n("Budgeting"));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::BudgetActual,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::yearToDate,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Budgeted vs. Actual This Year"),
                     i18n("Default Report")
                   ));
    list.back().setShowingRowTotals(true);
    list.back().setBudget("Any", true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::BudgetActual,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::yearToMonth,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Budgeted vs. Actual This Year (YTM)"),
                     i18n("Default Report")
                   ));
    list.back().setShowingRowTotals(true);
    list.back().setBudget("Any", true);
    // in case we're in January, we show the last year
    if (QDate::currentDate().month() == 1) {
      list.back().setDateFilter(MyMoneyTransactionFilter::lastYear);
    }

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::BudgetActual,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::currentMonth,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Monthly Budgeted vs. Actual"),
                     i18n("Default Report")
                   ));
    list.back().setBudget("Any", true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::BudgetActual,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::currentYear,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Yearly Budgeted vs. Actual"),
                     i18n("Default Report")
                   ));
    list.back().setBudget("Any", true);
    list.back().setShowingRowTotals(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Budget,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::currentMonth,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Monthly Budget"),
                     i18n("Default Report")
                   ));
    list.back().setBudget("Any", false);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Budget,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::currentYear,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Yearly Budget"),
                     i18n("Default Report")
                   ));
    list.back().setBudget("Any", false);
    list.back().setShowingRowTotals(true);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::BudgetActual,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::currentYear,
                     MyMoneyReport::DetailLevel::Group,
                     i18n("Yearly Budgeted vs Actual Graph"),
                     i18n("Default Report")
                   ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setBudget("Any", true);
    list.back().setChartType(MyMoneyReport::Chart::Line);

    groups.push_back(list);
  }
  {
    ReportGroup list("Forecast", i18n("Forecast"));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::next12Months,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Forecast By Month"),
                     i18n("Default Report")
                   ));
    list.back().setIncludingForecast(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::nextQuarter,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Forecast Next Quarter"),
                     i18n("Default Report")
                   ));
    list.back().setColumnsAreDays(true);
    list.back().setIncludingForecast(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::ExpenseIncome,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::currentYear,
                     MyMoneyReport::DetailLevel::Top,
                     i18n("Income and Expenses Forecast This Year"),
                     i18n("Default Report")
                   ));
    list.back().setIncludingForecast(true);

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AssetLiability,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::next3Months,
                     MyMoneyReport::DetailLevel::Total,
                     i18n("Net Worth Forecast Graph"),
                     i18n("Default Report")
                   ));
    list.back().setColumnsAreDays(true);
    list.back().setIncludingForecast(true);
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setChartType(MyMoneyReport::Chart::Line);
    groups.push_back(list);
  }
  {
    ReportGroup list("Information", i18n("General Information"));

    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Schedule,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::next12Months,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Schedule Information"),
                     i18n("Default Report")
                   ));
    list.back().setDetailLevel(MyMoneyReport::DetailLevel::All);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::Schedule,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::next12Months,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Schedule Summary Information"),
                     i18n("Default Report")
                   ));
    list.back().setDetailLevel(MyMoneyReport::DetailLevel::Top);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AccountInfo,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::today,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Account Information"),
                     i18n("Default Report")
                   ));
    list.back().setConvertCurrency(false);
    list.push_back(MyMoneyReport(
                     MyMoneyReport::Row::AccountLoanInfo,
                     MyMoneyReport::Column::Months,
                     MyMoneyTransactionFilter::today,
                     MyMoneyReport::DetailLevel::All,
                     i18n("Loan Information"),
                     i18n("Default Report")
                   ));
    list.back().setConvertCurrency(false);
    groups.push_back(list);
  }
}

bool KReportsView::columnsAlreadyAdjusted()
{
  return m_columnsAlreadyAdjusted;
}

void KReportsView::setColumnsAlreadyAdjusted(bool adjusted)
{
  m_columnsAlreadyAdjusted = adjusted;
}

void KReportsView::restoreTocExpandState(QMap<QString, bool>& expandStates)
{
  for (int i = 0; i < m_tocTreeWidget->topLevelItemCount(); i++) {
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

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef VIEW_LEDGER
#undef VIEW_SCHEDULE
#undef VIEW_WELCOME
#undef VIEW_HOME
#undef VIEW_REPORTS
