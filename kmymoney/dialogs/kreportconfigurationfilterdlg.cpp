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

#include "kreportconfigurationfilterdlg.h"
#include "kfindtransactiondlg_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QVariant>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
#include <KComboBox>
#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneybudget.h"
#include "mymoneyreport.h"
#include "daterangedlg.h"
#include "reporttabimpl.h"

#include "ui_kfindtransactiondlg.h"
#include <ui_reporttabgeneral.h>
#include <ui_reporttabrowcolpivot.h>
#include <ui_reporttabrowcolquery.h>
#include <ui_reporttabchart.h>
#include <ui_reporttabrange.h>
#include <ui_reporttabcapitalgain.h>
#include <ui_reporttabperformance.h>

class KReportConfigurationFilterDlgPrivate : public KFindTransactionDlgPrivate
{
  Q_DISABLE_COPY(KReportConfigurationFilterDlgPrivate)

public:
  KReportConfigurationFilterDlgPrivate(KReportConfigurationFilterDlg *qq) :
    KFindTransactionDlgPrivate(qq),
    m_tabRowColPivot(nullptr),
    m_tabRowColQuery(nullptr),
    m_tabChart(nullptr),
    m_tabRange(nullptr)
  {
  }

  QPointer<ReportTabGeneral>     m_tabGeneral;
  QPointer<ReportTabRowColPivot> m_tabRowColPivot;
  QPointer<ReportTabRowColQuery> m_tabRowColQuery;
  QPointer<ReportTabChart>       m_tabChart;
  QPointer<ReportTabRange>       m_tabRange;
  QPointer<ReportTabCapitalGain> m_tabCapitalGain;
  QPointer<ReportTabPerformance> m_tabPerformance;

  MyMoneyReport m_initialState;
  MyMoneyReport m_currentState;
  QVector<MyMoneyBudget> m_budgets;
};

KReportConfigurationFilterDlg::KReportConfigurationFilterDlg(MyMoneyReport report, QWidget *parent) :
  KFindTransactionDlg(*new KReportConfigurationFilterDlgPrivate(this), parent, (report.rowType() == MyMoneyReport::eAccount))
{
  Q_D(KReportConfigurationFilterDlg);

  d->m_initialState = report;
  d->m_currentState = report;

  //
  // Rework labeling
  //

  setWindowTitle(i18n("Report Configuration"));
  delete d->ui->TextLabel1;
  //
  // Rework the buttons
  //

  // the Apply button is always enabled
  disconnect(SIGNAL(selectionNotEmpty(bool)));
  d->ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
  d->ui->buttonBox->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip for report configuration apply button", "Apply the configuration changes to the report"));

  //
  // Add new tabs
  //

  d->m_tabGeneral = new ReportTabGeneral(d->ui->m_criteriaTab);
  d->ui->m_criteriaTab->insertTab(0, d->m_tabGeneral, i18nc("General tab", "General"));

  if (d->m_initialState.reportType() == MyMoneyReport::ePivotTable) {
    int tabNr = 1;
    if (!(d->m_initialState.isIncludingPrice() || d->m_initialState.isIncludingAveragePrice())) {
      d->m_tabRowColPivot = new ReportTabRowColPivot(d->ui->m_criteriaTab);
      d->ui->m_criteriaTab->insertTab(tabNr++, d->m_tabRowColPivot, i18n("Rows/Columns"));
      connect(d->m_tabRowColPivot->ui->m_comboRows, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KReportConfigurationFilterDlg::*)(int)>(&KReportConfigurationFilterDlg::slotRowTypeChanged));
      connect(d->m_tabRowColPivot->ui->m_comboRows, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KReportConfigurationFilterDlg::*)(int)>(&KReportConfigurationFilterDlg::slotUpdateColumnsCombo));
      //control the state of the includeTransfer check
      connect(d->ui->m_categoriesView, &KMyMoneySelector::stateChanged, this, &KReportConfigurationFilterDlg::slotUpdateCheckTransfers);
    }

    d->m_tabChart = new ReportTabChart(d->ui->m_criteriaTab);
    d->ui->m_criteriaTab->insertTab(tabNr++, d->m_tabChart, i18n("Chart"));

    d->m_tabRange = new ReportTabRange(d->ui->m_criteriaTab);
    d->ui->m_criteriaTab->insertTab(tabNr++, d->m_tabRange, i18n("Range"));

    // date tab is going to be replaced by range tab, so delete it
    d->ui->dateRangeLayout->removeWidget(d->m_dateRange);
    d->m_dateRange->deleteLater();
    d->ui->m_criteriaTab->removeTab(d->ui->m_criteriaTab->indexOf(d->ui->m_dateTab));
    d->ui->m_dateTab->deleteLater();

    d->m_dateRange = d->m_tabRange->m_dateRange;
    // reconnect signal
    connect(d->m_dateRange, &DateRangeDlg::rangeChanged, this, &KReportConfigurationFilterDlg::slotUpdateSelections);

    if (!(d->m_initialState.isIncludingPrice() || d->m_initialState.isIncludingAveragePrice())) {
      connect(d->m_tabRange->ui->m_comboColumns, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KReportConfigurationFilterDlg::slotColumnTypeChanged);
      connect(d->m_tabRange->ui->m_comboColumns, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KReportConfigurationFilterDlg::*)(int)>(&KReportConfigurationFilterDlg::slotUpdateColumnsCombo));
    }
    connect(d->m_tabChart->ui->m_logYaxis, &QCheckBox::stateChanged, this, &KReportConfigurationFilterDlg::slotLogAxisChanged);
  } else if (d->m_initialState.reportType() == MyMoneyReport::eQueryTable) {
    // eInvestmentHoldings is a special-case report, and you cannot configure the
    // rows & columns of that report.
    if (d->m_initialState.rowType() < MyMoneyReport::eAccountByTopAccount) {
      d->m_tabRowColQuery = new ReportTabRowColQuery(d->ui->m_criteriaTab);
      d->ui->m_criteriaTab->insertTab(1, d->m_tabRowColQuery, i18n("Rows/Columns"));
    }
    if (d->m_initialState.queryColumns() & MyMoneyReport::eQCcapitalgain) {
      d->m_tabCapitalGain = new ReportTabCapitalGain(d->ui->m_criteriaTab);
      d->ui->m_criteriaTab->insertTab(1, d->m_tabCapitalGain, i18n("Report"));
    }
    if (d->m_initialState.queryColumns() & MyMoneyReport::eQCperformance) {
      d->m_tabPerformance = new ReportTabPerformance(d->ui->m_criteriaTab);
      d->ui->m_criteriaTab->insertTab(1, d->m_tabPerformance, i18n("Report"));
    }
  }

  d->ui->m_criteriaTab->setCurrentIndex(d->ui->m_criteriaTab->indexOf(d->m_tabGeneral));
  d->ui->m_criteriaTab->setMinimumSize(500, 200);

  QList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
  QList<MyMoneyBudget>::const_iterator it_b;
  for (it_b = list.constBegin(); it_b != list.constEnd(); ++it_b) {
    d->m_budgets.push_back(*it_b);
  }

  //
  // Now set up the widgets with proper values
  //
  slotReset();
}

KReportConfigurationFilterDlg::~KReportConfigurationFilterDlg()
{
}

MyMoneyReport KReportConfigurationFilterDlg::getConfig() const
{
  Q_D(const KReportConfigurationFilterDlg);
  return d->m_currentState;
}

void KReportConfigurationFilterDlg::slotSearch()
{
  Q_D(KReportConfigurationFilterDlg);
  // setup the filter from the dialog widgets
  d->setupFilter();

  // Copy the m_filter over to the filter part of m_currentConfig.
  d->m_currentState.assignFilter(d->m_filter);

  // Then extract the report properties
  d->m_currentState.setName(d->m_tabGeneral->ui->m_editName->text());
  d->m_currentState.setComment(d->m_tabGeneral->ui->m_editComment->text());
  d->m_currentState.setConvertCurrency(d->m_tabGeneral->ui->m_checkCurrency->isChecked());
  d->m_currentState.setFavorite(d->m_tabGeneral->ui->m_checkFavorite->isChecked());
  d->m_currentState.setSkipZero(d->m_tabGeneral->ui->m_skipZero->isChecked());

  if (d->m_tabRowColPivot) {
    MyMoneyReport::EDetailLevel dl[4] = { MyMoneyReport::eDetailAll, MyMoneyReport::eDetailTop, MyMoneyReport::eDetailGroup, MyMoneyReport::eDetailTotal };

    d->m_currentState.setDetailLevel(dl[d->m_tabRowColPivot->ui->m_comboDetail->currentIndex()]);

    // modify the rowtype only if the widget is enabled
    if (d->m_tabRowColPivot->ui->m_comboRows->isEnabled()) {
      MyMoneyReport::ERowType rt[2] = { MyMoneyReport::eExpenseIncome, MyMoneyReport::eAssetLiability };
      d->m_currentState.setRowType(rt[d->m_tabRowColPivot->ui->m_comboRows->currentIndex()]);
    }

    d->m_currentState.setShowingRowTotals(false);
    if (d->m_tabRowColPivot->ui->m_comboRows->currentIndex() == 0)
      d->m_currentState.setShowingRowTotals(d->m_tabRowColPivot->ui->m_checkTotalColumn->isChecked());

    d->m_currentState.setShowingColumnTotals(d->m_tabRowColPivot->ui->m_checkTotalRow->isChecked());
    d->m_currentState.setIncludingSchedules(d->m_tabRowColPivot->ui->m_checkScheduled->isChecked());

    d->m_currentState.setIncludingTransfers(d->m_tabRowColPivot->ui->m_checkTransfers->isChecked());

    d->m_currentState.setIncludingUnusedAccounts(d->m_tabRowColPivot->ui->m_checkUnused->isChecked());

    if (d->m_tabRowColPivot->ui->m_comboBudget->isEnabled()) {
      d->m_currentState.setBudget(d->m_budgets[d->m_tabRowColPivot->ui->m_comboBudget->currentItem()].id(), d->m_initialState.rowType() == MyMoneyReport::eBudgetActual);
    } else {
      d->m_currentState.setBudget(QString(), false);
    }

    //set moving average days
    if (d->m_tabRowColPivot->ui->m_movingAverageDays->isEnabled()) {
      d->m_currentState.setMovingAverageDays(d->m_tabRowColPivot->ui->m_movingAverageDays->value());
    }
  } else if (d->m_tabRowColQuery) {
    MyMoneyReport::ERowType rtq[8] = { MyMoneyReport::eCategory, MyMoneyReport::eTopCategory, MyMoneyReport::eTag, MyMoneyReport::ePayee, MyMoneyReport::eAccount, MyMoneyReport::eTopAccount, MyMoneyReport::eMonth, MyMoneyReport::eWeek };
    d->m_currentState.setRowType(rtq[d->m_tabRowColQuery->ui->m_comboOrganizeBy->currentIndex()]);

    unsigned qc = MyMoneyReport::eQCnone;

    if (d->m_currentState.queryColumns() & MyMoneyReport::eQCloan)
      // once a loan report, always a loan report
      qc = MyMoneyReport::eQCloan;

    if (d->m_tabRowColQuery->ui->m_checkNumber->isChecked())
      qc |= MyMoneyReport::eQCnumber;
    if (d->m_tabRowColQuery->ui->m_checkPayee->isChecked())
      qc |= MyMoneyReport::eQCpayee;
    if (d->m_tabRowColQuery->ui->m_checkTag->isChecked())
      qc |= MyMoneyReport::eQCtag;
    if (d->m_tabRowColQuery->ui->m_checkCategory->isChecked())
      qc |= MyMoneyReport::eQCcategory;
    if (d->m_tabRowColQuery->ui->m_checkMemo->isChecked())
      qc |= MyMoneyReport::eQCmemo;
    if (d->m_tabRowColQuery->ui->m_checkAccount->isChecked())
      qc |= MyMoneyReport::eQCaccount;
    if (d->m_tabRowColQuery->ui->m_checkReconciled->isChecked())
      qc |= MyMoneyReport::eQCreconciled;
    if (d->m_tabRowColQuery->ui->m_checkAction->isChecked())
      qc |= MyMoneyReport::eQCaction;
    if (d->m_tabRowColQuery->ui->m_checkShares->isChecked())
      qc |= MyMoneyReport::eQCshares;
    if (d->m_tabRowColQuery->ui->m_checkPrice->isChecked())
      qc |= MyMoneyReport::eQCprice;
    if (d->m_tabRowColQuery->ui->m_checkBalance->isChecked())
      qc |= MyMoneyReport::eQCbalance;

    d->m_currentState.setQueryColumns(static_cast<MyMoneyReport::EQueryColumns>(qc));

    d->m_currentState.setTax(d->m_tabRowColQuery->ui->m_checkTax->isChecked());
    d->m_currentState.setInvestmentsOnly(d->m_tabRowColQuery->ui->m_checkInvestments->isChecked());
    d->m_currentState.setLoansOnly(d->m_tabRowColQuery->ui->m_checkLoans->isChecked());

    d->m_currentState.setDetailLevel(d->m_tabRowColQuery->ui->m_checkHideSplitDetails->isChecked() ?
                                  MyMoneyReport::eDetailNone : MyMoneyReport::eDetailAll);
    d->m_currentState.setHideTransactions(d->m_tabRowColQuery->ui->m_checkHideTransactions->isChecked());
    d->m_currentState.setShowingColumnTotals(!d->m_tabRowColQuery->ui->m_checkHideTotals->isChecked());
  }

  if (d->m_tabChart) {
    MyMoneyReport::EChartType ct[5] = { MyMoneyReport::eChartLine, MyMoneyReport::eChartBar, MyMoneyReport::eChartStackedBar, MyMoneyReport::eChartPie, MyMoneyReport::eChartRing };
    d->m_currentState.setChartType(ct[d->m_tabChart->ui->m_comboType->currentIndex()]);

    d->m_currentState.setChartCHGridLines(d->m_tabChart->ui->m_checkCHGridLines->isChecked());
    d->m_currentState.setChartSVGridLines(d->m_tabChart->ui->m_checkSVGridLines->isChecked());
    d->m_currentState.setChartDataLabels(d->m_tabChart->ui->m_checkValues->isChecked());
    d->m_currentState.setChartByDefault(d->m_tabChart->ui->m_checkShowChart->isChecked());
    d->m_currentState.setChartLineWidth(d->m_tabChart->ui->m_lineWidth->value());
    d->m_currentState.setLogYAxis(d->m_tabChart->ui->m_logYaxis->isChecked());
  }

  if (d->m_tabRange) {
    d->m_currentState.setDataRangeStart(d->m_tabRange->ui->m_dataRangeStart->text());
    d->m_currentState.setDataRangeEnd(d->m_tabRange->ui->m_dataRangeEnd->text());
    d->m_currentState.setDataMajorTick(d->m_tabRange->ui->m_dataMajorTick->text());
    d->m_currentState.setDataMinorTick(d->m_tabRange->ui->m_dataMinorTick->text());
    d->m_currentState.setYLabelsPrecision(d->m_tabRange->ui->m_yLabelsPrecision->value());
    d->m_currentState.setDataFilter((MyMoneyReport::dataOptionE)d->m_tabRange->ui->m_dataLock->currentIndex());

    MyMoneyReport::EColumnType ct[6] = { MyMoneyReport::eDays, MyMoneyReport::eWeeks, MyMoneyReport::eMonths, MyMoneyReport::eBiMonths, MyMoneyReport::eQuarters, MyMoneyReport::eYears };
    bool dy[6] = { true, true, false, false, false, false };
    d->m_currentState.setColumnType(ct[d->m_tabRange->ui->m_comboColumns->currentIndex()]);

    //TODO (Ace) This should be implicit in the call above.  MMReport needs fixin'
    d->m_currentState.setColumnsAreDays(dy[d->m_tabRange->ui->m_comboColumns->currentIndex()]);
  }

  // setup the date lock
  eMyMoney::TransactionFilter::Date range = d->m_dateRange->dateRange();
  d->m_currentState.setDateFilter(range);

  if (d->m_tabCapitalGain) {
    d->m_currentState.setTermSeparator(d->m_tabCapitalGain->ui->m_termSeparator->date());
    d->m_currentState.setShowSTLTCapitalGains(d->m_tabCapitalGain->ui->m_showSTLTCapitalGains->isChecked());
    d->m_currentState.setSettlementPeriod(d->m_tabCapitalGain->ui->m_settlementPeriod->value());
    d->m_currentState.setShowingColumnTotals(!d->m_tabCapitalGain->ui->m_checkHideTotals->isChecked());
    d->m_currentState.setInvestmentSum(static_cast<MyMoneyReport::EInvestmentSum>(d->m_tabCapitalGain->ui->m_investmentSum->currentData().toInt()));
  }

  if (d->m_tabPerformance) {
    d->m_currentState.setShowingColumnTotals(!d->m_tabPerformance->ui->m_checkHideTotals->isChecked());
    d->m_currentState.setInvestmentSum(static_cast<MyMoneyReport::EInvestmentSum>(d->m_tabPerformance->ui->m_investmentSum->currentData().toInt()));
  }

  done(true);
}

void KReportConfigurationFilterDlg::slotRowTypeChanged(int row)
{
  Q_D(KReportConfigurationFilterDlg);
  d->m_tabRowColPivot->ui->m_checkTotalColumn->setEnabled(row == 0);
}

void KReportConfigurationFilterDlg::slotColumnTypeChanged(int row)
{
  Q_D(KReportConfigurationFilterDlg);
  if ((d->m_tabRowColPivot->ui->m_comboBudget->isEnabled() && row < 2)) {
    d->m_tabRange->ui->m_comboColumns->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
  }
}

void KReportConfigurationFilterDlg::slotUpdateColumnsCombo()
{
  Q_D(KReportConfigurationFilterDlg);
  const int monthlyIndex = 2;
  const int incomeExpenseIndex = 0;
  const bool isIncomeExpenseForecast = d->m_currentState.isIncludingForecast() && d->m_tabRowColPivot->ui->m_comboRows->currentIndex() == incomeExpenseIndex;
  if (isIncomeExpenseForecast && d->m_tabRange->ui->m_comboColumns->currentIndex() != monthlyIndex) {
    d->m_tabRange->ui->m_comboColumns->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
  }
}

void KReportConfigurationFilterDlg::slotUpdateColumnsCombo(int)
{
  slotUpdateColumnsCombo();
}

void KReportConfigurationFilterDlg::slotLogAxisChanged(int state)
{
  Q_D(KReportConfigurationFilterDlg);
  if (state == Qt::Checked)
    d->m_tabRange->setRangeLogarythmic(true);
  else
    d->m_tabRange->setRangeLogarythmic(false);
}

void KReportConfigurationFilterDlg::slotReset()
{
  Q_D(KReportConfigurationFilterDlg);
  //
  // Set up the widget from the initial filter
  //
  d->m_currentState = d->m_initialState;

  //
  // Report Properties
  //

  d->m_tabGeneral->ui->m_editName->setText(d->m_initialState.name());
  d->m_tabGeneral->ui->m_editComment->setText(d->m_initialState.comment());
  d->m_tabGeneral->ui->m_checkCurrency->setChecked(d->m_initialState.isConvertCurrency());
  d->m_tabGeneral->ui->m_checkFavorite->setChecked(d->m_initialState.isFavorite());

  if (d->m_initialState.isIncludingPrice() || d->m_initialState.isSkippingZero()) {
    d->m_tabGeneral->ui->m_skipZero->setChecked(d->m_initialState.isSkippingZero());
  } else {
    d->m_tabGeneral->ui->m_skipZero->setEnabled(false);
  }

  if (d->m_tabRowColPivot) {
    KComboBox *combo = d->m_tabRowColPivot->ui->m_comboDetail;
    switch (d->m_initialState.detailLevel()) {
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

    combo = d->m_tabRowColPivot->ui->m_comboRows;
    switch (d->m_initialState.rowType()) {
      case MyMoneyReport::eExpenseIncome:
      case MyMoneyReport::eBudget:
      case MyMoneyReport::eBudgetActual:
        combo->setCurrentItem(i18n("Income & Expenses"), false); // income / expense
        break;
      default:
        combo->setCurrentItem(i18n("Assets & Liabilities"), false); // asset / liability
        break;
    }
    d->m_tabRowColPivot->ui->m_checkTotalColumn->setChecked(d->m_initialState.isShowingRowTotals());
    d->m_tabRowColPivot->ui->m_checkTotalRow->setChecked(d->m_initialState.isShowingColumnTotals());

    slotRowTypeChanged(combo->currentIndex());

    //load budgets combo
    if (d->m_initialState.rowType() == MyMoneyReport::eBudget
        || d->m_initialState.rowType() == MyMoneyReport::eBudgetActual) {
      d->m_tabRowColPivot->ui->m_comboRows->setEnabled(false);
      d->m_tabRowColPivot->ui->m_budgetFrame->setEnabled(!d->m_budgets.empty());
      auto i = 0;
      for (QVector<MyMoneyBudget>::const_iterator it_b = d->m_budgets.constBegin(); it_b != d->m_budgets.constEnd(); ++it_b) {
        d->m_tabRowColPivot->ui->m_comboBudget->insertItem((*it_b).name(), i);
        //set the current selected item
        if ((d->m_initialState.budget() == "Any" && (*it_b).budgetStart().year() == QDate::currentDate().year())
            || d->m_initialState.budget() == (*it_b).id())
          d->m_tabRowColPivot->ui->m_comboBudget->setCurrentItem(i);
        i++;
      }
    }

    //set moving average days spinbox
    QSpinBox *spinbox = d->m_tabRowColPivot->ui->m_movingAverageDays;
    spinbox->setEnabled(d->m_initialState.isIncludingMovingAverage());
    if (d->m_initialState.isIncludingMovingAverage()) {
      spinbox->setValue(d->m_initialState.movingAverageDays());
    }

    d->m_tabRowColPivot->ui->m_checkScheduled->setChecked(d->m_initialState.isIncludingSchedules());
    d->m_tabRowColPivot->ui->m_checkTransfers->setChecked(d->m_initialState.isIncludingTransfers());
    d->m_tabRowColPivot->ui->m_checkUnused->setChecked(d->m_initialState.isIncludingUnusedAccounts());
  } else if (d->m_tabRowColQuery) {
    KComboBox *combo = d->m_tabRowColQuery->ui->m_comboOrganizeBy;
    switch (d->m_initialState.rowType()) {
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

    unsigned qc = d->m_initialState.queryColumns();
    d->m_tabRowColQuery->ui->m_checkNumber->setChecked(qc & MyMoneyReport::eQCnumber);
    d->m_tabRowColQuery->ui->m_checkPayee->setChecked(qc & MyMoneyReport::eQCpayee);
    d->m_tabRowColQuery->ui->m_checkTag->setChecked(qc & MyMoneyReport::eQCtag);
    d->m_tabRowColQuery->ui->m_checkCategory->setChecked(qc & MyMoneyReport::eQCcategory);
    d->m_tabRowColQuery->ui->m_checkMemo->setChecked(qc & MyMoneyReport::eQCmemo);
    d->m_tabRowColQuery->ui->m_checkAccount->setChecked(qc & MyMoneyReport::eQCaccount);
    d->m_tabRowColQuery->ui->m_checkReconciled->setChecked(qc & MyMoneyReport::eQCreconciled);
    d->m_tabRowColQuery->ui->m_checkAction->setChecked(qc & MyMoneyReport::eQCaction);
    d->m_tabRowColQuery->ui->m_checkShares->setChecked(qc & MyMoneyReport::eQCshares);
    d->m_tabRowColQuery->ui->m_checkPrice->setChecked(qc & MyMoneyReport::eQCprice);
    d->m_tabRowColQuery->ui->m_checkBalance->setChecked(qc & MyMoneyReport::eQCbalance);

    d->m_tabRowColQuery->ui->m_checkTax->setChecked(d->m_initialState.isTax());
    d->m_tabRowColQuery->ui->m_checkInvestments->setChecked(d->m_initialState.isInvestmentsOnly());
    d->m_tabRowColQuery->ui->m_checkLoans->setChecked(d->m_initialState.isLoansOnly());

    d->m_tabRowColQuery->ui->m_checkHideTransactions->setChecked(d->m_initialState.isHideTransactions());
    d->m_tabRowColQuery->ui->m_checkHideTotals->setChecked(!d->m_initialState.isShowingColumnTotals());
    d->m_tabRowColQuery->ui->m_checkHideSplitDetails->setEnabled(!d->m_initialState.isHideTransactions());

    d->m_tabRowColQuery->ui->m_checkHideSplitDetails->setChecked
    (d->m_initialState.detailLevel() == MyMoneyReport::eDetailNone || d->m_initialState.isHideTransactions());
  }

  if (d->m_tabChart) {
    KMyMoneyGeneralCombo* combo = d->m_tabChart->ui->m_comboType;
    switch (d->m_initialState.chartType()) {
      case MyMoneyReport::eChartNone:
        combo->setCurrentItem(MyMoneyReport::eChartLine);
        break;
      case MyMoneyReport::eChartLine:
      case MyMoneyReport::eChartBar:
      case MyMoneyReport::eChartStackedBar:
      case MyMoneyReport::eChartPie:
      case MyMoneyReport::eChartRing:
        combo->setCurrentItem(d->m_initialState.chartType());
        break;
      default:
        throw MYMONEYEXCEPTION("KReportConfigurationFilterDlg::slotReset(): Report has invalid charttype");
    }
    d->m_tabChart->ui->m_checkCHGridLines->setChecked(d->m_initialState.isChartCHGridLines());
    d->m_tabChart->ui->m_checkSVGridLines->setChecked(d->m_initialState.isChartSVGridLines());
    d->m_tabChart->ui->m_checkValues->setChecked(d->m_initialState.isChartDataLabels());
    d->m_tabChart->ui->m_checkShowChart->setChecked(d->m_initialState.isChartByDefault());
    d->m_tabChart->ui->m_lineWidth->setValue(d->m_initialState.chartLineWidth());
    d->m_tabChart->ui->m_logYaxis->setChecked(d->m_initialState.isLogYAxis());
  }

  if (d->m_tabRange) {
    d->m_tabRange->ui->m_dataRangeStart->setText(d->m_initialState.dataRangeStart());
    d->m_tabRange->ui->m_dataRangeEnd->setText(d->m_initialState.dataRangeEnd());
    d->m_tabRange->ui->m_dataMajorTick->setText(d->m_initialState.dataMajorTick());
    d->m_tabRange->ui->m_dataMinorTick->setText(d->m_initialState.dataMinorTick());
    d->m_tabRange->ui->m_yLabelsPrecision->setValue(d->m_initialState.yLabelsPrecision());
    d->m_tabRange->ui->m_dataLock->setCurrentIndex((int)d->m_initialState.dataFilter());

    KComboBox *combo = d->m_tabRange->ui->m_comboColumns;
    if (d->m_initialState.isColumnsAreDays()) {
      switch (d->m_initialState.columnType()) {
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
      switch (d->m_initialState.columnType()) {
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

  if (d->m_tabCapitalGain) {
    d->m_tabCapitalGain->ui->m_termSeparator->setDate(d->m_initialState.termSeparator());
    d->m_tabCapitalGain->ui->m_showSTLTCapitalGains->setChecked(d->m_initialState.isShowingSTLTCapitalGains());
    d->m_tabCapitalGain->ui->m_settlementPeriod->setValue(d->m_initialState.settlementPeriod());
    d->m_tabCapitalGain->ui->m_checkHideTotals->setChecked(!d->m_initialState.isShowingColumnTotals());
    d->m_tabCapitalGain->ui->m_investmentSum->blockSignals(true);
    d->m_tabCapitalGain->ui->m_investmentSum->clear();
    d->m_tabCapitalGain->ui->m_investmentSum->addItem(i18n("Only owned"), MyMoneyReport::eSumOwned);
    d->m_tabCapitalGain->ui->m_investmentSum->addItem(i18n("Only sold"), MyMoneyReport::eSumSold);
    d->m_tabCapitalGain->ui->m_investmentSum->blockSignals(false);
    d->m_tabCapitalGain->ui->m_investmentSum->setCurrentIndex(d->m_tabCapitalGain->ui->m_investmentSum->findData(d->m_initialState.investmentSum()));
  }

  if (d->m_tabPerformance) {
    d->m_tabPerformance->ui->m_checkHideTotals->setChecked(!d->m_initialState.isShowingColumnTotals());
    d->m_tabPerformance->ui->m_investmentSum->blockSignals(true);
    d->m_tabPerformance->ui->m_investmentSum->clear();
    d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("From period"), MyMoneyReport::eSumPeriod);
    d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("Owned and sold"), MyMoneyReport::eSumOwnedAndSold);
    d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("Only owned"), MyMoneyReport::eSumOwned);
    d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("Only sold"), MyMoneyReport::eSumSold);
    d->m_tabPerformance->ui->m_investmentSum->blockSignals(false);
    d->m_tabPerformance->ui->m_investmentSum->setCurrentIndex(d->m_tabPerformance->ui->m_investmentSum->findData(d->m_initialState.investmentSum()));
  }

  //
  // Text Filter
  //

  QRegExp textfilter;
  if (d->m_initialState.textFilter(textfilter)) {
    d->ui->m_textEdit->setText(textfilter.pattern());
    d->ui->m_caseSensitive->setChecked(Qt::CaseSensitive == textfilter.caseSensitivity());
    d->ui->m_regExp->setChecked(QRegExp::RegExp == textfilter.patternSyntax());
    d->ui->m_textNegate->setCurrentIndex(d->m_initialState.isInvertingText());
  }

  //
  // Type & State Filters
  //

  int type;
  if (d->m_initialState.firstType(type))
    d->ui->m_typeBox->setCurrentIndex(type);

  int state;
  if (d->m_initialState.firstState(state))
    d->ui->m_stateBox->setCurrentIndex(state);

  //
  // Number Filter
  //

  QString nrFrom, nrTo;
  if (d->m_initialState.numberFilter(nrFrom, nrTo)) {
    if (nrFrom == nrTo) {
      d->ui->m_nrEdit->setEnabled(true);
      d->ui->m_nrFromEdit->setEnabled(false);
      d->ui->m_nrToEdit->setEnabled(false);
      d->ui->m_nrEdit->setText(nrFrom);
      d->ui->m_nrFromEdit->setText(QString());
      d->ui->m_nrToEdit->setText(QString());
      d->ui->m_nrButton->setChecked(true);
      d->ui->m_nrRangeButton->setChecked(false);
    } else {
      d->ui->m_nrEdit->setEnabled(false);
      d->ui->m_nrFromEdit->setEnabled(true);
      d->ui->m_nrToEdit->setEnabled(false);
      d->ui->m_nrEdit->setText(QString());
      d->ui->m_nrFromEdit->setText(nrFrom);
      d->ui->m_nrToEdit->setText(nrTo);
      d->ui->m_nrButton->setChecked(false);
      d->ui->m_nrRangeButton->setChecked(true);
    }
  } else {
    d->ui->m_nrEdit->setEnabled(true);
    d->ui->m_nrFromEdit->setEnabled(false);
    d->ui->m_nrToEdit->setEnabled(false);
    d->ui->m_nrEdit->setText(QString());
    d->ui->m_nrFromEdit->setText(QString());
    d->ui->m_nrToEdit->setText(QString());
    d->ui->m_nrButton->setChecked(true);
    d->ui->m_nrRangeButton->setChecked(false);
  }

  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if (d->m_initialState.amountFilter(from, to)) { // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
    if (from == to) {
      d->ui->m_amountEdit->setEnabled(true);
      d->ui->m_amountFromEdit->setEnabled(false);
      d->ui->m_amountToEdit->setEnabled(false);
      d->ui->m_amountEdit->loadText(QString::number(from.toDouble()));
      d->ui->m_amountFromEdit->loadText(QString());
      d->ui->m_amountToEdit->loadText(QString());
      d->ui->m_amountButton->setChecked(true);
      d->ui->m_amountRangeButton->setChecked(false);
    } else {
      d->ui->m_amountEdit->setEnabled(false);
      d->ui->m_amountFromEdit->setEnabled(true);
      d->ui->m_amountToEdit->setEnabled(true);
      d->ui->m_amountEdit->loadText(QString());
      d->ui->m_amountFromEdit->loadText(QString::number(from.toDouble()));
      d->ui->m_amountToEdit->loadText(QString::number(to.toDouble()));
      d->ui->m_amountButton->setChecked(false);
      d->ui->m_amountRangeButton->setChecked(true);
    }
  } else {
    d->ui->m_amountEdit->setEnabled(true);
    d->ui->m_amountFromEdit->setEnabled(false);
    d->ui->m_amountToEdit->setEnabled(false);
    d->ui->m_amountEdit->loadText(QString());
    d->ui->m_amountFromEdit->loadText(QString());
    d->ui->m_amountToEdit->loadText(QString());
    d->ui->m_amountButton->setChecked(true);
    d->ui->m_amountRangeButton->setChecked(false);
  }

  //
  // Payees Filter
  //

  QStringList payees;
  if (d->m_initialState.payees(payees)) {
    if (payees.empty()) {
      d->ui->m_emptyPayeesButton->setChecked(true);
    } else {
      d->selectAllItems(d->ui->m_payeesView, false);
      d->selectItems(d->ui->m_payeesView, payees, true);
    }
  } else {
    d->selectAllItems(d->ui->m_payeesView, true);
  }

  //
  // Tags Filter
  //

  QStringList tags;
  if (d->m_initialState.tags(tags)) {
    if (tags.empty()) {
      d->ui->m_emptyTagsButton->setChecked(true);
    } else {
      d->selectAllItems(d->ui->m_tagsView, false);
      d->selectItems(d->ui->m_tagsView, tags, true);
    }
  } else {
    d->selectAllItems(d->ui->m_tagsView, true);
  }

  //
  // Accounts Filter
  //

  QStringList accounts;
  if (d->m_initialState.accounts(accounts)) {
    d->ui->m_accountsView->selectAllItems(false);
    d->ui->m_accountsView->selectItems(accounts, true);
  } else
    d->ui->m_accountsView->selectAllItems(true);

  //
  // Categories Filter
  //

  if (d->m_initialState.categories(accounts)) {
    d->ui->m_categoriesView->selectAllItems(false);
    d->ui->m_categoriesView->selectItems(accounts, true);
  } else
    d->ui->m_categoriesView->selectAllItems(true);

  //
  // Date Filter
  //

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last

  d->m_initialState.updateDateFilter();
  QDate dateFrom, dateTo;
  if (d->m_initialState.dateFilter(dateFrom, dateTo)) {
    if (d->m_initialState.isDateUserDefined()) {
      d->m_dateRange->setDateRange(dateFrom, dateTo);
    } else {
      d->m_dateRange->setDateRange(d->m_initialState.dateRange());
    }
  } else {
    d->m_dateRange->setDateRange(eMyMoney::TransactionFilter::Date::All);
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
  Q_D(KReportConfigurationFilterDlg);
  auto cb = d->m_tabRowColPivot->ui->m_checkTransfers;
  if (!d->ui->m_categoriesView->allItemsSelected()) {
    cb->setChecked(false);
    cb->setDisabled(true);
  } else {
    cb->setEnabled(true);
  }
}
