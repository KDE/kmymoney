/***************************************************************************
                          kreportconfigurationdlg.cpp  -  description
                             -------------------
    begin                : Mon Jun 21 2004
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

#include "kreportconfigurationfilterdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QVariant>
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QLayout>
#include <QSpinBox>
#include <QToolTip>
#include <QTabWidget>
#include <QApplication>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>
#include <kcombobox.h>
#include <kguiitem.h>
#include <kstandardguiitem.h>
#include <khelpclient.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneylineedit.h>
#include <kmymoneyaccountselector.h>
#include <mymoneyfile.h>
#include <mymoneyreport.h>
#include <daterangedlg.h>

#include "ui_kfindtransactiondlgdecl.h"
#include <ui_daterangedlgdecl.h>
#include <ui_reporttabgeneral.h>
#include <ui_reporttabrowcolpivot.h>
#include <ui_reporttabrowcolquery.h>
#include <ui_reporttabchart.h>
#include <ui_reporttabrange.h>
#include <ui_reporttabcapitalgain.h>
#include <ui_reporttabperformance.h>

KReportConfigurationFilterDlg::KReportConfigurationFilterDlg(MyMoneyReport report, QWidget *parent)
  : KFindTransactionDlg(parent, (report.rowType() == MyMoneyReport::eAccount))
  , m_tabRowColPivot(0)
  , m_tabRowColQuery(0)
  , m_tabChart(0)
  , m_tabRange(0)
  , m_initialState(report)
  , m_currentState(report)
{
  //
  // Rework labeling
  //

  setWindowTitle(i18n("Report Configuration"));
  delete m_ui->TextLabel1;
  //
  // Rework the buttons
  //

  // the Apply button is always enabled
  disconnect(SIGNAL(selectionNotEmpty(bool)));
  m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
  KGuiItem::assign(m_ui->buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::ok());
  m_ui->buttonBox->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip for report configuration apply button", "Apply the configuration changes to the report"));

  //
  // Add new tabs
  //

  m_tabGeneral = new ReportTabGeneral(m_ui->m_criteriaTab);
  m_ui->m_criteriaTab->insertTab(0, m_tabGeneral, i18nc("General tab", "General"));

  if (m_initialState.reportType() == MyMoneyReport::ePivotTable) {
    int tabNr = 1;
    if (!(m_initialState.isIncludingPrice() || m_initialState.isIncludingAveragePrice())) {
      m_tabRowColPivot = new ReportTabRowColPivot(m_ui->m_criteriaTab);
      m_ui->m_criteriaTab->insertTab(tabNr++, m_tabRowColPivot, i18n("Rows/Columns"));
      connect(m_tabRowColPivot->ui->m_comboRows, SIGNAL(activated(int)), this, SLOT(slotRowTypeChanged(int)));
      connect(m_tabRowColPivot->ui->m_comboRows, SIGNAL(activated(int)), this, SLOT(slotUpdateColumnsCombo()));
      //control the state of the includeTransfer check
      connect(m_ui->m_categoriesView, SIGNAL(stateChanged()), this, SLOT(slotUpdateCheckTransfers()));
    }

    m_tabChart = new ReportTabChart(m_ui->m_criteriaTab);
    m_ui->m_criteriaTab->insertTab(tabNr++, m_tabChart, i18n("Chart"));

    m_tabRange = new ReportTabRange(m_ui->m_criteriaTab);
    m_ui->m_criteriaTab->insertTab(tabNr++, m_tabRange, i18n("Range"));

    // date tab is going to be replaced by range tab, so delete it
    m_ui->dateRangeLayout->removeWidget(m_dateRange);
    m_dateRange->deleteLater();
    m_ui->m_criteriaTab->removeTab(m_ui->m_criteriaTab->indexOf(m_ui->m_dateTab));
    m_ui->m_dateTab->deleteLater();

    m_dateRange = m_tabRange->m_dateRange;
    // reconnect signal
    connect(m_dateRange, SIGNAL(rangeChanged()), this, SLOT(slotUpdateSelections()));

    if (!(m_initialState.isIncludingPrice() || m_initialState.isIncludingAveragePrice())) {
      connect(m_tabRange->ui->m_comboColumns, SIGNAL(activated(int)), this, SLOT(slotColumnTypeChanged(int)));
      connect(m_tabRange->ui->m_comboColumns, SIGNAL(activated(int)), this, SLOT(slotUpdateColumnsCombo()));
    }
    connect(m_tabChart->ui->m_logYaxis, SIGNAL(stateChanged(int)), this, SLOT(slotLogAxisChanged(int)));
  } else if (m_initialState.reportType() == MyMoneyReport::eQueryTable) {
    // eInvestmentHoldings is a special-case report, and you cannot configure the
    // rows & columns of that report.
    if (m_initialState.rowType() < MyMoneyReport::eAccountByTopAccount) {
      m_tabRowColQuery = new ReportTabRowColQuery(m_ui->m_criteriaTab);
      m_ui->m_criteriaTab->insertTab(1, m_tabRowColQuery, i18n("Rows/Columns"));
    }
    if (m_initialState.queryColumns() & MyMoneyReport::eQCcapitalgain) {
      m_tabCapitalGain = new ReportTabCapitalGain(m_ui->m_criteriaTab);
      m_ui->m_criteriaTab->insertTab(1, m_tabCapitalGain, i18n("Report"));
    }
    if (m_initialState.queryColumns() & MyMoneyReport::eQCperformance) {
      m_tabPerformance = new ReportTabPerformance(m_ui->m_criteriaTab);
      m_ui->m_criteriaTab->insertTab(1, m_tabPerformance, i18n("Report"));
    }
  }

  m_ui->m_criteriaTab->setCurrentIndex(m_ui->m_criteriaTab->indexOf(m_tabGeneral));
  m_ui->m_criteriaTab->setMinimumSize(500, 200);

  QList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
  QList<MyMoneyBudget>::const_iterator it_b;
  for (it_b = list.constBegin(); it_b != list.constEnd(); ++it_b) {
    m_budgets.push_back(*it_b);
  }

  //
  // Now set up the widgets with proper values
  //
  slotReset();
}

KReportConfigurationFilterDlg::~KReportConfigurationFilterDlg()
{
}

void KReportConfigurationFilterDlg::slotSearch()
{
  // setup the filter from the dialog widgets
  setupFilter();

  // Copy the m_filter over to the filter part of m_currentConfig.
  m_currentState.assignFilter(m_filter);

  // Then extract the report properties
  m_currentState.setName(m_tabGeneral->ui->m_editName->text());
  m_currentState.setComment(m_tabGeneral->ui->m_editComment->text());
  m_currentState.setConvertCurrency(m_tabGeneral->ui->m_checkCurrency->isChecked());
  m_currentState.setFavorite(m_tabGeneral->ui->m_checkFavorite->isChecked());
  m_currentState.setSkipZero(m_tabGeneral->ui->m_skipZero->isChecked());

  if (m_tabRowColPivot) {
    MyMoneyReport::EDetailLevel dl[4] = { MyMoneyReport::eDetailAll, MyMoneyReport::eDetailTop, MyMoneyReport::eDetailGroup, MyMoneyReport::eDetailTotal };

    m_currentState.setDetailLevel(dl[m_tabRowColPivot->ui->m_comboDetail->currentIndex()]);

    // modify the rowtype only if the widget is enabled
    if (m_tabRowColPivot->ui->m_comboRows->isEnabled()) {
      MyMoneyReport::ERowType rt[2] = { MyMoneyReport::eExpenseIncome, MyMoneyReport::eAssetLiability };
      m_currentState.setRowType(rt[m_tabRowColPivot->ui->m_comboRows->currentIndex()]);
    }

    m_currentState.setShowingRowTotals(false);
    if (m_tabRowColPivot->ui->m_comboRows->currentIndex() == 0)
      m_currentState.setShowingRowTotals(m_tabRowColPivot->ui->m_checkTotalColumn->isChecked());

    m_currentState.setShowingColumnTotals(m_tabRowColPivot->ui->m_checkTotalRow->isChecked());
    m_currentState.setIncludingSchedules(m_tabRowColPivot->ui->m_checkScheduled->isChecked());

    m_currentState.setIncludingTransfers(m_tabRowColPivot->ui->m_checkTransfers->isChecked());

    m_currentState.setIncludingUnusedAccounts(m_tabRowColPivot->ui->m_checkUnused->isChecked());

    if (m_tabRowColPivot->ui->m_comboBudget->isEnabled()) {
      m_currentState.setBudget(m_budgets[m_tabRowColPivot->ui->m_comboBudget->currentItem()].id(), m_initialState.rowType() == MyMoneyReport::eBudgetActual);
    } else {
      m_currentState.setBudget(QString(), false);
    }

    //set moving average days
    if (m_tabRowColPivot->ui->m_movingAverageDays->isEnabled()) {
      m_currentState.setMovingAverageDays(m_tabRowColPivot->ui->m_movingAverageDays->value());
    }
  } else if (m_tabRowColQuery) {
    MyMoneyReport::ERowType rtq[8] = { MyMoneyReport::eCategory, MyMoneyReport::eTopCategory, MyMoneyReport::eTag, MyMoneyReport::ePayee, MyMoneyReport::eAccount, MyMoneyReport::eTopAccount, MyMoneyReport::eMonth, MyMoneyReport::eWeek };
    m_currentState.setRowType(rtq[m_tabRowColQuery->ui->m_comboOrganizeBy->currentIndex()]);

    unsigned qc = MyMoneyReport::eQCnone;

    if (m_currentState.queryColumns() & MyMoneyReport::eQCloan)
      // once a loan report, always a loan report
      qc = MyMoneyReport::eQCloan;

    if (m_tabRowColQuery->ui->m_checkNumber->isChecked())
      qc |= MyMoneyReport::eQCnumber;
    if (m_tabRowColQuery->ui->m_checkPayee->isChecked())
      qc |= MyMoneyReport::eQCpayee;
    if (m_tabRowColQuery->ui->m_checkTag->isChecked())
      qc |= MyMoneyReport::eQCtag;
    if (m_tabRowColQuery->ui->m_checkCategory->isChecked())
      qc |= MyMoneyReport::eQCcategory;
    if (m_tabRowColQuery->ui->m_checkMemo->isChecked())
      qc |= MyMoneyReport::eQCmemo;
    if (m_tabRowColQuery->ui->m_checkAccount->isChecked())
      qc |= MyMoneyReport::eQCaccount;
    if (m_tabRowColQuery->ui->m_checkReconciled->isChecked())
      qc |= MyMoneyReport::eQCreconciled;
    if (m_tabRowColQuery->ui->m_checkAction->isChecked())
      qc |= MyMoneyReport::eQCaction;
    if (m_tabRowColQuery->ui->m_checkShares->isChecked())
      qc |= MyMoneyReport::eQCshares;
    if (m_tabRowColQuery->ui->m_checkPrice->isChecked())
      qc |= MyMoneyReport::eQCprice;
    if (m_tabRowColQuery->ui->m_checkBalance->isChecked())
      qc |= MyMoneyReport::eQCbalance;

    m_currentState.setQueryColumns(static_cast<MyMoneyReport::EQueryColumns>(qc));

    m_currentState.setTax(m_tabRowColQuery->ui->m_checkTax->isChecked());
    m_currentState.setInvestmentsOnly(m_tabRowColQuery->ui->m_checkInvestments->isChecked());
    m_currentState.setLoansOnly(m_tabRowColQuery->ui->m_checkLoans->isChecked());

    m_currentState.setDetailLevel(m_tabRowColQuery->ui->m_checkHideSplitDetails->isChecked() ?
                                  MyMoneyReport::eDetailNone : MyMoneyReport::eDetailAll);
    m_currentState.setHideTransactions(m_tabRowColQuery->ui->m_checkHideTransactions->isChecked());
    m_currentState.setShowingColumnTotals(!m_tabRowColQuery->ui->m_checkHideTotals->isChecked());
  }

  if (m_tabChart) {
    MyMoneyReport::EChartType ct[5] = { MyMoneyReport::eChartLine, MyMoneyReport::eChartBar, MyMoneyReport::eChartStackedBar, MyMoneyReport::eChartPie, MyMoneyReport::eChartRing };
    m_currentState.setChartType(ct[m_tabChart->ui->m_comboType->currentIndex()]);

    m_currentState.setChartCHGridLines(m_tabChart->ui->m_checkCHGridLines->isChecked());
    m_currentState.setChartSVGridLines(m_tabChart->ui->m_checkSVGridLines->isChecked());
    m_currentState.setChartDataLabels(m_tabChart->ui->m_checkValues->isChecked());
    m_currentState.setChartByDefault(m_tabChart->ui->m_checkShowChart->isChecked());
    m_currentState.setChartLineWidth(m_tabChart->ui->m_lineWidth->value());
    m_currentState.setLogYAxis(m_tabChart->ui->m_logYaxis->isChecked());
  }

  if (m_tabRange) {
    m_currentState.setDataRangeStart(m_tabRange->ui->m_dataRangeStart->text());
    m_currentState.setDataRangeEnd(m_tabRange->ui->m_dataRangeEnd->text());
    m_currentState.setDataMajorTick(m_tabRange->ui->m_dataMajorTick->text());
    m_currentState.setDataMinorTick(m_tabRange->ui->m_dataMinorTick->text());
    m_currentState.setYLabelsPrecision(m_tabRange->ui->m_yLabelsPrecision->value());
    m_currentState.setDataFilter((MyMoneyReport::dataOptionE)m_tabRange->ui->m_dataLock->currentIndex());

    MyMoneyReport::EColumnType ct[6] = { MyMoneyReport::eDays, MyMoneyReport::eWeeks, MyMoneyReport::eMonths, MyMoneyReport::eBiMonths, MyMoneyReport::eQuarters, MyMoneyReport::eYears };
    bool dy[6] = { true, true, false, false, false, false };
    m_currentState.setColumnType(ct[m_tabRange->ui->m_comboColumns->currentIndex()]);

    //TODO (Ace) This should be implicit in the call above.  MMReport needs fixin'
    m_currentState.setColumnsAreDays(dy[m_tabRange->ui->m_comboColumns->currentIndex()]);
  }

  // setup the date lock
  MyMoneyTransactionFilter::dateOptionE range = m_dateRange->dateRange();
  m_currentState.setDateFilter(range);

  if (m_tabCapitalGain) {
    m_currentState.setTermSeparator(m_tabCapitalGain->ui->m_termSeparator->date());
    m_currentState.setShowSTLTCapitalGains(m_tabCapitalGain->ui->m_showSTLTCapitalGains->isChecked());
    m_currentState.setSettlementPeriod(m_tabCapitalGain->ui->m_settlementPeriod->value());
    m_currentState.setShowingColumnTotals(!m_tabCapitalGain->ui->m_checkHideTotals->isChecked());
    m_currentState.setInvestmentSum(static_cast<MyMoneyReport::EInvestmentSum>(m_tabCapitalGain->ui->m_investmentSum->currentData().toInt()));
  }

  if (m_tabPerformance) {
    m_currentState.setShowingColumnTotals(!m_tabPerformance->ui->m_checkHideTotals->isChecked());
    m_currentState.setInvestmentSum(static_cast<MyMoneyReport::EInvestmentSum>(m_tabPerformance->ui->m_investmentSum->currentData().toInt()));
  }

  done(true);
}

void KReportConfigurationFilterDlg::slotRowTypeChanged(int row)
{
  m_tabRowColPivot->ui->m_checkTotalColumn->setEnabled(row == 0);
}

void KReportConfigurationFilterDlg::slotColumnTypeChanged(int row)
{
  if ((m_tabRowColPivot->ui->m_comboBudget->isEnabled() && row < 2)) {
    m_tabRange->ui->m_comboColumns->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
  }
}

void KReportConfigurationFilterDlg::slotUpdateColumnsCombo()
{
  const int monthlyIndex = 2;
  const int incomeExpenseIndex = 0;
  const bool isIncomeExpenseForecast = m_currentState.isIncludingForecast() && m_tabRowColPivot->ui->m_comboRows->currentIndex() == incomeExpenseIndex;
  if (isIncomeExpenseForecast && m_tabRange->ui->m_comboColumns->currentIndex() != monthlyIndex) {
    m_tabRange->ui->m_comboColumns->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
  }
}

void KReportConfigurationFilterDlg::slotLogAxisChanged(int state)
{
  if (state == Qt::Checked)
    m_tabRange->setRangeLogarythmic(true);
  else
    m_tabRange->setRangeLogarythmic(false);
}

void KReportConfigurationFilterDlg::slotReset()
{
  //
  // Set up the widget from the initial filter
  //
  m_currentState = m_initialState;

  //
  // Report Properties
  //

  m_tabGeneral->ui->m_editName->setText(m_initialState.name());
  m_tabGeneral->ui->m_editComment->setText(m_initialState.comment());
  m_tabGeneral->ui->m_checkCurrency->setChecked(m_initialState.isConvertCurrency());
  m_tabGeneral->ui->m_checkFavorite->setChecked(m_initialState.isFavorite());

  if (m_initialState.isIncludingPrice() || m_initialState.isSkippingZero()) {
    m_tabGeneral->ui->m_skipZero->setChecked(m_initialState.isSkippingZero());
  } else {
    m_tabGeneral->ui->m_skipZero->setEnabled(false);
  }

  if (m_tabRowColPivot) {
    KComboBox *combo = m_tabRowColPivot->ui->m_comboDetail;
    switch (m_initialState.detailLevel()) {
      case MyMoneyReport::eDetailNone:
      case MyMoneyReport::eDetailEnd:
      case MyMoneyReport::eDetailAll:
        combo->setCurrentItem(i18nc("All accounts", "All"), false);
        break;
      case MyMoneyReport::eDetailTop:
        combo->setCurrentItem(i18n("Top-Level"), false);
        break;
      case MyMoneyReport::eDetailGroup:
        combo->setCurrentItem(i18n("Groups"), false);
        break;
      case MyMoneyReport::eDetailTotal:
        combo->setCurrentItem(i18n("Totals"), false);
        break;
    }

    combo = m_tabRowColPivot->ui->m_comboRows;
    switch (m_initialState.rowType()) {
      case MyMoneyReport::eExpenseIncome:
      case MyMoneyReport::eBudget:
      case MyMoneyReport::eBudgetActual:
        combo->setCurrentItem(i18n("Income & Expenses"), false); // income / expense
        break;
      default:
        combo->setCurrentItem(i18n("Assets & Liabilities"), false); // asset / liability
        break;
    }
    m_tabRowColPivot->ui->m_checkTotalColumn->setChecked(m_initialState.isShowingRowTotals());
    m_tabRowColPivot->ui->m_checkTotalRow->setChecked(m_initialState.isShowingColumnTotals());

    slotRowTypeChanged(combo->currentIndex());

    //load budgets combo
    if (m_initialState.rowType() == MyMoneyReport::eBudget
        || m_initialState.rowType() == MyMoneyReport::eBudgetActual) {
      m_tabRowColPivot->ui->m_comboRows->setEnabled(false);
      m_tabRowColPivot->ui->m_budgetFrame->setEnabled(!m_budgets.empty());
      int i = 0;
      for (QVector<MyMoneyBudget>::const_iterator it_b = m_budgets.constBegin(); it_b != m_budgets.constEnd(); ++it_b) {
        m_tabRowColPivot->ui->m_comboBudget->insertItem((*it_b).name(), i);
        //set the current selected item
        if ((m_initialState.budget() == "Any" && (*it_b).budgetStart().year() == QDate::currentDate().year())
            || m_initialState.budget() == (*it_b).id())
          m_tabRowColPivot->ui->m_comboBudget->setCurrentItem(i);
        i++;
      }
    }

    //set moving average days spinbox
    QSpinBox *spinbox = m_tabRowColPivot->ui->m_movingAverageDays;
    spinbox->setEnabled(m_initialState.isIncludingMovingAverage());
    if (m_initialState.isIncludingMovingAverage()) {
      spinbox->setValue(m_initialState.movingAverageDays());
    }

    m_tabRowColPivot->ui->m_checkScheduled->setChecked(m_initialState.isIncludingSchedules());
    m_tabRowColPivot->ui->m_checkTransfers->setChecked(m_initialState.isIncludingTransfers());
    m_tabRowColPivot->ui->m_checkUnused->setChecked(m_initialState.isIncludingUnusedAccounts());
  } else if (m_tabRowColQuery) {
    KComboBox *combo = m_tabRowColQuery->ui->m_comboOrganizeBy;
    switch (m_initialState.rowType()) {
      case MyMoneyReport::eNoColumns:
      case MyMoneyReport::eCategory:
        combo->setCurrentItem(i18n("Categories"), false);
        break;
      case MyMoneyReport::eTopCategory:
        combo->setCurrentItem(i18n("Top Categories"), false);
        break;
      case MyMoneyReport::eTag:
        combo->setCurrentItem(i18n("Tags"), false);
        break;
      case MyMoneyReport::ePayee:
        combo->setCurrentItem(i18n("Payees"), false);
        break;
      case MyMoneyReport::eAccount:
        combo->setCurrentItem(i18n("Accounts"), false);
        break;
      case MyMoneyReport::eTopAccount:
        combo->setCurrentItem(i18n("Top Accounts"), false);
        break;
      case MyMoneyReport::eMonth:
        combo->setCurrentItem(i18n("Month"), false);
        break;
      case MyMoneyReport::eWeek:
        combo->setCurrentItem(i18n("Week"), false);
        break;
      default:
        throw MYMONEYEXCEPTION("KReportConfigurationFilterDlg::slotReset(): QueryTable report has invalid rowtype");
    }

    unsigned qc = m_initialState.queryColumns();
    m_tabRowColQuery->ui->m_checkNumber->setChecked(qc & MyMoneyReport::eQCnumber);
    m_tabRowColQuery->ui->m_checkPayee->setChecked(qc & MyMoneyReport::eQCpayee);
    m_tabRowColQuery->ui->m_checkTag->setChecked(qc & MyMoneyReport::eQCtag);
    m_tabRowColQuery->ui->m_checkCategory->setChecked(qc & MyMoneyReport::eQCcategory);
    m_tabRowColQuery->ui->m_checkMemo->setChecked(qc & MyMoneyReport::eQCmemo);
    m_tabRowColQuery->ui->m_checkAccount->setChecked(qc & MyMoneyReport::eQCaccount);
    m_tabRowColQuery->ui->m_checkReconciled->setChecked(qc & MyMoneyReport::eQCreconciled);
    m_tabRowColQuery->ui->m_checkAction->setChecked(qc & MyMoneyReport::eQCaction);
    m_tabRowColQuery->ui->m_checkShares->setChecked(qc & MyMoneyReport::eQCshares);
    m_tabRowColQuery->ui->m_checkPrice->setChecked(qc & MyMoneyReport::eQCprice);
    m_tabRowColQuery->ui->m_checkBalance->setChecked(qc & MyMoneyReport::eQCbalance);

    m_tabRowColQuery->ui->m_checkTax->setChecked(m_initialState.isTax());
    m_tabRowColQuery->ui->m_checkInvestments->setChecked(m_initialState.isInvestmentsOnly());
    m_tabRowColQuery->ui->m_checkLoans->setChecked(m_initialState.isLoansOnly());

    m_tabRowColQuery->ui->m_checkHideTransactions->setChecked(m_initialState.isHideTransactions());
    m_tabRowColQuery->ui->m_checkHideTotals->setChecked(!m_initialState.isShowingColumnTotals());
    m_tabRowColQuery->ui->m_checkHideSplitDetails->setEnabled(!m_initialState.isHideTransactions());

    m_tabRowColQuery->ui->m_checkHideSplitDetails->setChecked
    (m_initialState.detailLevel() == MyMoneyReport::eDetailNone || m_initialState.isHideTransactions());
  }

  if (m_tabChart) {
    KMyMoneyGeneralCombo* combo = m_tabChart->ui->m_comboType;
    switch (m_initialState.chartType()) {
      case MyMoneyReport::eChartNone:
        combo->setCurrentItem(MyMoneyReport::eChartLine);
        break;
      case MyMoneyReport::eChartLine:
      case MyMoneyReport::eChartBar:
      case MyMoneyReport::eChartStackedBar:
      case MyMoneyReport::eChartPie:
      case MyMoneyReport::eChartRing:
        combo->setCurrentItem(m_initialState.chartType());
        break;
      default:
        throw MYMONEYEXCEPTION("KReportConfigurationFilterDlg::slotReset(): Report has invalid charttype");
    }
    m_tabChart->ui->m_checkCHGridLines->setChecked(m_initialState.isChartCHGridLines());
    m_tabChart->ui->m_checkSVGridLines->setChecked(m_initialState.isChartSVGridLines());
    m_tabChart->ui->m_checkValues->setChecked(m_initialState.isChartDataLabels());
    m_tabChart->ui->m_checkShowChart->setChecked(m_initialState.isChartByDefault());
    m_tabChart->ui->m_lineWidth->setValue(m_initialState.chartLineWidth());
    m_tabChart->ui->m_logYaxis->setChecked(m_initialState.isLogYAxis());
  }

  if (m_tabRange) {
    m_tabRange->ui->m_dataRangeStart->setText(m_initialState.dataRangeStart());
    m_tabRange->ui->m_dataRangeEnd->setText(m_initialState.dataRangeEnd());
    m_tabRange->ui->m_dataMajorTick->setText(m_initialState.dataMajorTick());
    m_tabRange->ui->m_dataMinorTick->setText(m_initialState.dataMinorTick());
    m_tabRange->ui->m_yLabelsPrecision->setValue(m_initialState.yLabelsPrecision());
    m_tabRange->ui->m_dataLock->setCurrentIndex((int)m_initialState.dataFilter());

    KComboBox *combo = m_tabRange->ui->m_comboColumns;
    if (m_initialState.isColumnsAreDays()) {
      switch (m_initialState.columnType()) {
        case MyMoneyReport::eNoColumns:
        case MyMoneyReport::eDays:
          combo->setCurrentItem(i18nc("@item the columns will display daily data", "Daily"), false);
          break;
        case MyMoneyReport::eWeeks:
          combo->setCurrentItem(i18nc("@item the columns will display weekly data", "Weekly"), false);
          break;
        default:
          break;
      }
    } else {
      switch (m_initialState.columnType()) {
        case MyMoneyReport::eNoColumns:
        case MyMoneyReport::eMonths:
          combo->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
          break;
        case MyMoneyReport::eBiMonths:
          combo->setCurrentItem(i18nc("@item the columns will display bi-monthly data", "Bi-Monthly"), false);
          break;
        case MyMoneyReport::eQuarters:
          combo->setCurrentItem(i18nc("@item the columns will display quarterly data", "Quarterly"), false);
          break;
        case MyMoneyReport::eYears:
          combo->setCurrentItem(i18nc("@item the columns will display yearly data", "Yearly"), false);
          break;
        default:
          break;
      }
    }
  }

  if (m_tabCapitalGain) {
    m_tabCapitalGain->ui->m_termSeparator->setDate(m_initialState.termSeparator());
    m_tabCapitalGain->ui->m_showSTLTCapitalGains->setChecked(m_initialState.isShowingSTLTCapitalGains());
    m_tabCapitalGain->ui->m_settlementPeriod->setValue(m_initialState.settlementPeriod());
    m_tabCapitalGain->ui->m_checkHideTotals->setChecked(!m_initialState.isShowingColumnTotals());
    m_tabCapitalGain->ui->m_investmentSum->blockSignals(true);
    m_tabCapitalGain->ui->m_investmentSum->clear();
    m_tabCapitalGain->ui->m_investmentSum->addItem(i18n("Only owned"), MyMoneyReport::eSumOwned);
    m_tabCapitalGain->ui->m_investmentSum->addItem(i18n("Only sold"), MyMoneyReport::eSumSold);
    m_tabCapitalGain->ui->m_investmentSum->blockSignals(false);
    m_tabCapitalGain->ui->m_investmentSum->setCurrentIndex(m_tabCapitalGain->ui->m_investmentSum->findData(m_initialState.investmentSum()));
  }

  if (m_tabPerformance) {
    m_tabPerformance->ui->m_checkHideTotals->setChecked(!m_initialState.isShowingColumnTotals());
    m_tabPerformance->ui->m_investmentSum->blockSignals(true);
    m_tabPerformance->ui->m_investmentSum->clear();
    m_tabPerformance->ui->m_investmentSum->addItem(i18n("From period"), MyMoneyReport::eSumPeriod);
    m_tabPerformance->ui->m_investmentSum->addItem(i18n("Owned and sold"), MyMoneyReport::eSumOwnedAndSold);
    m_tabPerformance->ui->m_investmentSum->addItem(i18n("Only owned"), MyMoneyReport::eSumOwned);
    m_tabPerformance->ui->m_investmentSum->addItem(i18n("Only sold"), MyMoneyReport::eSumSold);
    m_tabPerformance->ui->m_investmentSum->blockSignals(false);
    m_tabPerformance->ui->m_investmentSum->setCurrentIndex(m_tabPerformance->ui->m_investmentSum->findData(m_initialState.investmentSum()));
  }

  //
  // Text Filter
  //

  QRegExp textfilter;
  if (m_initialState.textFilter(textfilter)) {
    m_ui->m_textEdit->setText(textfilter.pattern());
    m_ui->m_caseSensitive->setChecked(Qt::CaseSensitive == textfilter.caseSensitivity());
    m_ui->m_regExp->setChecked(QRegExp::RegExp == textfilter.patternSyntax());
    m_ui->m_textNegate->setCurrentIndex(m_initialState.isInvertingText());
  }

  //
  // Type & State Filters
  //

  int type;
  if (m_initialState.firstType(type))
    m_ui->m_typeBox->setCurrentIndex(type);

  int state;
  if (m_initialState.firstState(state))
    m_ui->m_stateBox->setCurrentIndex(state);

  //
  // Number Filter
  //

  QString nrFrom, nrTo;
  if (m_initialState.numberFilter(nrFrom, nrTo)) {
    if (nrFrom == nrTo) {
      m_ui->m_nrEdit->setEnabled(true);
      m_ui->m_nrFromEdit->setEnabled(false);
      m_ui->m_nrToEdit->setEnabled(false);
      m_ui->m_nrEdit->setText(nrFrom);
      m_ui->m_nrFromEdit->setText(QString());
      m_ui->m_nrToEdit->setText(QString());
      m_ui->m_nrButton->setChecked(true);
      m_ui->m_nrRangeButton->setChecked(false);
    } else {
      m_ui->m_nrEdit->setEnabled(false);
      m_ui->m_nrFromEdit->setEnabled(true);
      m_ui->m_nrToEdit->setEnabled(false);
      m_ui->m_nrEdit->setText(QString());
      m_ui->m_nrFromEdit->setText(nrFrom);
      m_ui->m_nrToEdit->setText(nrTo);
      m_ui->m_nrButton->setChecked(false);
      m_ui->m_nrRangeButton->setChecked(true);
    }
  } else {
    m_ui->m_nrEdit->setEnabled(true);
    m_ui->m_nrFromEdit->setEnabled(false);
    m_ui->m_nrToEdit->setEnabled(false);
    m_ui->m_nrEdit->setText(QString());
    m_ui->m_nrFromEdit->setText(QString());
    m_ui->m_nrToEdit->setText(QString());
    m_ui->m_nrButton->setChecked(true);
    m_ui->m_nrRangeButton->setChecked(false);
  }

  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if (m_initialState.amountFilter(from, to)) { // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
    if (from == to) {
      m_ui->m_amountEdit->setEnabled(true);
      m_ui->m_amountFromEdit->setEnabled(false);
      m_ui->m_amountToEdit->setEnabled(false);
      m_ui->m_amountEdit->loadText(QString::number(from.toDouble()));
      m_ui->m_amountFromEdit->loadText(QString());
      m_ui->m_amountToEdit->loadText(QString());
      m_ui->m_amountButton->setChecked(true);
      m_ui->m_amountRangeButton->setChecked(false);
    } else {
      m_ui->m_amountEdit->setEnabled(false);
      m_ui->m_amountFromEdit->setEnabled(true);
      m_ui->m_amountToEdit->setEnabled(true);
      m_ui->m_amountEdit->loadText(QString());
      m_ui->m_amountFromEdit->loadText(QString::number(from.toDouble()));
      m_ui->m_amountToEdit->loadText(QString::number(to.toDouble()));
      m_ui->m_amountButton->setChecked(false);
      m_ui->m_amountRangeButton->setChecked(true);
    }
  } else {
    m_ui->m_amountEdit->setEnabled(true);
    m_ui->m_amountFromEdit->setEnabled(false);
    m_ui->m_amountToEdit->setEnabled(false);
    m_ui->m_amountEdit->loadText(QString());
    m_ui->m_amountFromEdit->loadText(QString());
    m_ui->m_amountToEdit->loadText(QString());
    m_ui->m_amountButton->setChecked(true);
    m_ui->m_amountRangeButton->setChecked(false);
  }

  //
  // Payees Filter
  //

  QStringList payees;
  if (m_initialState.payees(payees)) {
    if (payees.empty()) {
      m_ui->m_emptyPayeesButton->setChecked(true);
    } else {
      selectAllItems(m_ui->m_payeesView, false);
      selectItems(m_ui->m_payeesView, payees, true);
    }
  } else {
    selectAllItems(m_ui->m_payeesView, true);
  }

  //
  // Tags Filter
  //

  QStringList tags;
  if (m_initialState.tags(tags)) {
    if (tags.empty()) {
      m_ui->m_emptyTagsButton->setChecked(true);
    } else {
      selectAllItems(m_ui->m_tagsView, false);
      selectItems(m_ui->m_tagsView, tags, true);
    }
  } else {
    selectAllItems(m_ui->m_tagsView, true);
  }

  //
  // Accounts Filter
  //

  QStringList accounts;
  if (m_initialState.accounts(accounts)) {
    m_ui->m_accountsView->selectAllItems(false);
    m_ui->m_accountsView->selectItems(accounts, true);
  } else
    m_ui->m_accountsView->selectAllItems(true);

  //
  // Categories Filter
  //

  if (m_initialState.categories(accounts)) {
    m_ui->m_categoriesView->selectAllItems(false);
    m_ui->m_categoriesView->selectItems(accounts, true);
  } else
    m_ui->m_categoriesView->selectAllItems(true);

  //
  // Date Filter
  //

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last

  m_initialState.updateDateFilter();
  QDate dateFrom, dateTo;
  if (m_initialState.dateFilter(dateFrom, dateTo)) {
    if (m_initialState.isDateUserDefined()) {
      m_dateRange->setDateRange(dateFrom, dateTo);
    } else {
      m_dateRange->setDateRange(m_initialState.dateRange());
    }
  } else {
    m_dateRange->setDateRange(MyMoneyTransactionFilter::allDates);
  }

  slotRightSize();
}

void KReportConfigurationFilterDlg::slotShowHelp()
{
  KHelpClient::invokeHelp("details.reports.config");
}

//TODO Fix the reports and engine to include transfers even if categories are filtered - bug #1523508
void KReportConfigurationFilterDlg::slotUpdateCheckTransfers()
{
  QCheckBox* cb = m_tabRowColPivot->ui->m_checkTransfers;
  if (!m_ui->m_categoriesView->allItemsSelected()) {
    cb->setChecked(false);
    cb->setDisabled(true);
  } else {
    cb->setEnabled(true);
  }
}
