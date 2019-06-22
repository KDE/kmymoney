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

#include <kapplication.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneylineedit.h>
#include <kmymoneyaccountselector.h>
#include <kmymoneymvccombo.h>
#include <mymoneyfile.h>
#include <mymoneyreport.h>
#include <ktoolinvocation.h>
#include "ui_kfindtransactiondlgdecl.h"

KReportConfigurationFilterDlg::KReportConfigurationFilterDlg(
  MyMoneyReport report, QWidget *parent)
    : KFindTransactionDlg(parent, report.rowType() == MyMoneyReport::Row::Account),
    m_tab2(0),
    m_tab3(0),
    m_tabChart(0),
    m_initialState(report),
    m_currentState(report)
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
  enableButtonApply(true);
  setButtonGuiItem(KDialog::Apply, KStandardGuiItem::ok());
  setButtonToolTip(KDialog::Apply, i18nc("@info:tooltip for report configuration apply button", "Apply the configuration changes to the report"));

  //
  // Add new tabs
  //

  m_tab1 = new kMyMoneyReportConfigTab1Decl(m_ui->m_criteriaTab);
  m_tab1->setObjectName("kMyMoneyReportConfigTab1");
  m_ui->m_criteriaTab->insertTab(0, m_tab1, i18n("Report"));

  if (m_initialState.reportType() == MyMoneyReport::Report::PivotTable) {
    m_tab2 = new kMyMoneyReportConfigTab2Decl(m_ui->m_criteriaTab);
    m_tab2->setObjectName("kMyMoneyReportConfigTab2");
    m_ui->m_criteriaTab->insertTab(1, m_tab2, i18n("Rows/Columns"));
    connect(m_tab2->findChild<KComboBox*>("m_comboRows"), SIGNAL(activated(int)), this, SLOT(slotRowTypeChanged(int)));
    connect(m_tab2->findChild<KComboBox*>("m_comboColumns"), SIGNAL(activated(int)), this, SLOT(slotColumnTypeChanged(int)));
    connect(m_tab2->findChild<KComboBox*>("m_comboRows"), SIGNAL(activated(int)), this, SLOT(slotUpdateColumnsCombo()));
    connect(m_tab2->findChild<KComboBox*>("m_comboColumns"), SIGNAL(activated(int)), this, SLOT(slotUpdateColumnsCombo()));
    //control the state of the includeTransfer check
    connect(m_ui->m_categoriesView, SIGNAL(stateChanged()), this, SLOT(slotUpdateCheckTransfers()));

    m_tabChart = new kMyMoneyReportConfigTabChartDecl(m_ui->m_criteriaTab);
    m_tabChart->setObjectName("kMyMoneyReportConfigTabChart");
    m_ui->m_criteriaTab->insertTab(2, m_tabChart, i18n("Chart"));
  } else if (m_initialState.reportType() == MyMoneyReport::Report::QueryTable) {
    // eInvestmentHoldings is a special-case report, and you cannot configure the
    // rows & columns of that report.
    if (m_initialState.rowType() < MyMoneyReport::Row::AccountByTopAccount) {
      m_tab3 = new kMyMoneyReportConfigTab3Decl(m_ui->m_criteriaTab);
      m_tab3->setObjectName("kMyMoneyReportConfigTab3");
      m_ui->m_criteriaTab->insertTab(1, m_tab3, i18n("Rows/Columns"));
    }
  }

  m_ui->m_criteriaTab->setCurrentIndex(m_ui->m_criteriaTab->indexOf(m_tab1));
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
  m_currentState.setName(m_tab1->findChild<KLineEdit*>("m_editName")->text());
  m_currentState.setComment(m_tab1->findChild<KLineEdit*>("m_editComment")->text());
  m_currentState.setConvertCurrency(m_tab1->findChild<QCheckBox*>("m_checkCurrency")->isChecked());
  m_currentState.setFavorite(m_tab1->findChild<QCheckBox*>("m_checkFavorite")->isChecked());
  m_currentState.setSkipZero(m_tab1->findChild<QCheckBox*>("m_skipZero")->isChecked());

  if (m_tab2) {
    MyMoneyReport::DetailLevel::Type dl[4] = { MyMoneyReport::DetailLevel::All, MyMoneyReport::DetailLevel::Top, MyMoneyReport::DetailLevel::Group, MyMoneyReport::DetailLevel::Total };

    m_currentState.setDetailLevel(dl[m_tab2->findChild<KComboBox*>("m_comboDetail")->currentIndex()]);

    // modify the rowtype only if the widget is enabled
    if (m_tab2->findChild<KComboBox*>("m_comboRows")->isEnabled()) {
      MyMoneyReport::Row::Type rt[2] = { MyMoneyReport::Row::ExpenseIncome, MyMoneyReport::Row::AssetLiability };
      m_currentState.setRowType(rt[m_tab2->findChild<KComboBox*>("m_comboRows")->currentIndex()]);
    }

    m_currentState.setShowingRowTotals(false);
    if (m_tab2->findChild<KComboBox*>("m_comboRows")->currentIndex() == 0)
      m_currentState.setShowingRowTotals(m_tab2->findChild<QCheckBox*>("m_checkTotalColumn")->isChecked());

    MyMoneyReport::Column::Type ct[6] = { MyMoneyReport::Column::Days, MyMoneyReport::Column::Weeks, MyMoneyReport::Column::Months, MyMoneyReport::Column::BiMonths, MyMoneyReport::Column::Quarters, MyMoneyReport::Column::Years };
    bool dy[6] = { true, true, false, false, false, false };
    m_currentState.setColumnType(ct[m_tab2->findChild<KComboBox*>("m_comboColumns")->currentIndex()]);

    //TODO (Ace) This should be implicit in the call above.  MMReport needs fixin'
    m_currentState.setColumnsAreDays(dy[m_tab2->findChild<KComboBox*>("m_comboColumns")->currentIndex()]);

    m_currentState.setIncludingSchedules(m_tab2->findChild<QCheckBox*>("m_checkScheduled")->isChecked());

    m_currentState.setIncludingTransfers(m_tab2->findChild<QCheckBox*>("m_checkTransfers")->isChecked());

    m_currentState.setIncludingUnusedAccounts(m_tab2->findChild<QCheckBox*>("m_checkUnused")->isChecked());

    if (m_tab2->findChild<KMyMoneyGeneralCombo*>("m_comboBudget")->isEnabled()) {
      m_currentState.setBudget(m_budgets[m_tab2->findChild<KMyMoneyGeneralCombo*>("m_comboBudget")->currentItem()].id(), m_initialState.rowType() == MyMoneyReport::Row::BudgetActual);
    } else {
      m_currentState.setBudget(QString(), false);
    }

    //set moving average days
    if (m_tab2->findChild<QSpinBox*>("m_movingAverageDays")->isEnabled()) {
      m_currentState.setMovingAverageDays(m_tab2->findChild<QSpinBox*>("m_movingAverageDays")->value());
    }
  } else if (m_tab3) {
    MyMoneyReport::Row::Type rtq[8] = { MyMoneyReport::Row::Category, MyMoneyReport::Row::TopCategory, MyMoneyReport::Row::Tag, MyMoneyReport::Row::Payee, MyMoneyReport::Row::Account, MyMoneyReport::Row::TopAccount, MyMoneyReport::Row::Month, MyMoneyReport::Row::Week };
    m_currentState.setRowType(rtq[m_tab3->findChild<KComboBox*>("m_comboOrganizeBy")->currentIndex()]);

    unsigned qc = MyMoneyReport::QueryColumns::None;

    if (m_currentState.queryColumns() & MyMoneyReport::QueryColumns::Loan)
      // once a loan report, always a loan report
      qc = MyMoneyReport::QueryColumns::Loan;

    if (m_tab3->findChild<QCheckBox*>("m_checkNumber")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Number;
    if (m_tab3->findChild<QCheckBox*>("m_checkPayee")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Payee;
    if (m_tab3->findChild<QCheckBox*>("m_checkTag")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Tag;
    if (m_tab3->findChild<QCheckBox*>("m_checkCategory")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Category;
    if (m_tab3->findChild<QCheckBox*>("m_checkMemo")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Memo;
    if (m_tab3->findChild<QCheckBox*>("m_checkAccount")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Account;
    if (m_tab3->findChild<QCheckBox*>("m_checkReconciled")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Reconciled;
    if (m_tab3->findChild<QCheckBox*>("m_checkAction")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Action;
    if (m_tab3->findChild<QCheckBox*>("m_checkShares")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Shares;
    if (m_tab3->findChild<QCheckBox*>("m_checkPrice")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Price;
    if (m_tab3->findChild<QCheckBox*>("m_checkBalance")->isChecked())
      qc |= MyMoneyReport::QueryColumns::Balance;

    m_currentState.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(qc));

    m_currentState.setTax(m_tab3->findChild<QCheckBox*>("m_checkTax")->isChecked());
    m_currentState.setInvestmentsOnly(m_tab3->findChild<QCheckBox*>("m_checkInvestments")->isChecked());
    m_currentState.setLoansOnly(m_tab3->findChild<QCheckBox*>("m_checkLoans")->isChecked());

    m_currentState.setDetailLevel(m_tab3->findChild<QCheckBox*>("m_checkHideSplitDetails")->isChecked() ?
                                  MyMoneyReport::DetailLevel::None : MyMoneyReport::DetailLevel::All);
  }

  if (m_tabChart) {
    MyMoneyReport::Chart::Type ct[5] = { MyMoneyReport::Chart::Line, MyMoneyReport::Chart::Bar, MyMoneyReport::Chart::StackedBar, MyMoneyReport::Chart::Pie, MyMoneyReport::Chart::Ring };
    m_currentState.setChartType(ct[m_tabChart->findChild<KMyMoneyGeneralCombo*>("m_comboType")->currentIndex()]);
    MyMoneyReport::ChartPalette::Type cp[4] = { MyMoneyReport::ChartPalette::Application, MyMoneyReport::ChartPalette::Default, MyMoneyReport::ChartPalette::Rainbow, MyMoneyReport::ChartPalette::Subdued };
    m_currentState.setChartPalette(cp[m_tabChart->findChild<KMyMoneyGeneralCombo*>("m_chartPalette")->currentIndex()]);

    m_currentState.setChartGridLines(m_tabChart->findChild<QCheckBox*>("m_checkGridLines")->isChecked());
    m_currentState.setChartDataLabels(m_tabChart->findChild<QCheckBox*>("m_checkValues")->isChecked());
    m_currentState.setChartByDefault(m_tabChart->findChild<QCheckBox*>("m_checkShowChart")->isChecked());
    m_currentState.setChartLineWidth(m_tabChart->findChild<QSpinBox*>("m_lineWidth")->value());
  }

  // setup the date lock
  MyMoneyTransactionFilter::dateOptionE range = m_ui->m_dateRange->currentItem();
  m_currentState.setDateFilter(range);

  done(true);
}

void KReportConfigurationFilterDlg::slotRowTypeChanged(int row)
{
  m_tab2->findChild<QCheckBox*>("m_checkTotalColumn")->setEnabled(row == 0);
}

void KReportConfigurationFilterDlg::slotColumnTypeChanged(int row)
{
  if ((m_tab2->findChild<KMyMoneyGeneralCombo*>("m_comboBudget")->isEnabled() && row < 2)) {
    m_tab2->findChild<KComboBox*>("m_comboColumns")->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
  }
}

void KReportConfigurationFilterDlg::slotUpdateColumnsCombo()
{
  const int monthlyIndex = 2;
  const int incomeExpenseIndex = 0;
  const bool isIncomeExpenseForecast = m_currentState.isIncludingForecast() && m_tab2->findChild<KComboBox*>("m_comboRows")->currentIndex() == incomeExpenseIndex;
  if (isIncomeExpenseForecast && m_tab2->findChild<KComboBox*>("m_comboColumns")->currentIndex() != monthlyIndex) {
    m_tab2->findChild<KComboBox*>("m_comboColumns")->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
  }
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

  m_tab1->findChild<KLineEdit*>("m_editName")->setText(m_initialState.name());
  m_tab1->findChild<KLineEdit*>("m_editComment")->setText(m_initialState.comment());
  m_tab1->findChild<QCheckBox*>("m_checkCurrency")->setChecked(m_initialState.isConvertCurrency());
  m_tab1->findChild<QCheckBox*>("m_checkFavorite")->setChecked(m_initialState.isFavorite());

  if (m_initialState.isIncludingPrice() || m_initialState.isSkippingZero()) {
    m_tab1->findChild<QCheckBox*>("m_skipZero")->setChecked(m_initialState.isSkippingZero());
  } else {
    m_tab1->findChild<QCheckBox*>("m_skipZero")->setEnabled(false);
  }

  if (m_tab2) {
    KComboBox *combo = m_tab2->findChild<KComboBox*>("m_comboDetail");
    switch (m_initialState.detailLevel()) {
      case MyMoneyReport::DetailLevel::None:
      case MyMoneyReport::DetailLevel::End:
      case MyMoneyReport::DetailLevel::All:
        combo->setCurrentItem(i18nc("All accounts", "All"), false);
        break;
      case MyMoneyReport::DetailLevel::Top:
        combo->setCurrentItem(i18n("Top-Level"), false);
        break;
      case MyMoneyReport::DetailLevel::Group:
        combo->setCurrentItem(i18n("Groups"), false);
        break;
      case MyMoneyReport::DetailLevel::Total:
        combo->setCurrentItem(i18n("Totals"), false);
        break;
    }

    combo = m_tab2->findChild<KComboBox*>("m_comboRows");
    switch (m_initialState.rowType()) {
      case MyMoneyReport::Row::ExpenseIncome:
      case MyMoneyReport::Row::Budget:
      case MyMoneyReport::Row::BudgetActual:
        combo->setCurrentItem(i18n("Income & Expenses"), false); // income / expense
        break;
      default:
        combo->setCurrentItem(i18n("Assets & Liabilities"), false); // asset / liability
        break;
    }
    m_tab2->findChild<QCheckBox*>("m_checkTotalColumn")->setChecked(m_initialState.isShowingRowTotals());

    slotRowTypeChanged(combo->currentIndex());

    combo = m_tab2->findChild<KComboBox*>("m_comboColumns");
    if (m_initialState.isColumnsAreDays()) {
      switch (m_initialState.columnType()) {
        case MyMoneyReport::Column::NoColumns:
        case MyMoneyReport::Column::Days:
          combo->setCurrentItem(i18nc("@item the columns will display daily data", "Daily"), false);
          break;
        case MyMoneyReport::Column::Weeks:
          combo->setCurrentItem(i18nc("@item the columns will display weekly data", "Weekly"), false);
          break;
        default:
          break;
      }
    } else {
      switch (m_initialState.columnType()) {
        case MyMoneyReport::Column::NoColumns:
        case MyMoneyReport::Column::Months:
          combo->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
          break;
        case MyMoneyReport::Column::BiMonths:
          combo->setCurrentItem(i18nc("@item the columns will display bi-monthly data", "Bi-Monthly"), false);
          break;
        case MyMoneyReport::Column::Quarters:
          combo->setCurrentItem(i18nc("@item the columns will display quarterly data", "Quarterly"), false);
          break;
        case MyMoneyReport::Column::Years:
          combo->setCurrentItem(i18nc("@item the columns will display yearly data", "Yearly"), false);
          break;
        default:
          break;
      }
    }

    //load budgets combo
    if (m_initialState.rowType() == MyMoneyReport::Row::Budget
        || m_initialState.rowType() == MyMoneyReport::Row::BudgetActual) {
      m_tab2->findChild<KComboBox*>("m_comboRows")->setEnabled(false);
      m_tab2->findChild<QFrame*>("m_budgetFrame")->setEnabled(!m_budgets.empty());
      int i = 0;
      for (QVector<MyMoneyBudget>::const_iterator it_b = m_budgets.constBegin(); it_b != m_budgets.constEnd(); ++it_b) {
        m_tab2->findChild<KMyMoneyGeneralCombo*>("m_comboBudget")->insertItem((*it_b).name(), i);
        //set the current selected item
        if ((m_initialState.budget() == "Any" && (*it_b).budgetStart().year() == QDate::currentDate().year())
            || m_initialState.budget() == (*it_b).id())
          m_tab2->findChild<KMyMoneyGeneralCombo*>("m_comboBudget")->setCurrentItem(i);
        i++;
      }
    }

    //set moving average days spinbox
    QSpinBox *spinbox = m_tab2->findChild<QSpinBox*>("m_movingAverageDays");
    spinbox->setEnabled(m_initialState.isIncludingMovingAverage());
    if (m_initialState.isIncludingMovingAverage()) {
      spinbox->setValue(m_initialState.movingAverageDays());
    }

    m_tab2->findChild<QCheckBox*>("m_checkScheduled")->setChecked(m_initialState.isIncludingSchedules());
    m_tab2->findChild<QCheckBox*>("m_checkTransfers")->setChecked(m_initialState.isIncludingTransfers());
    m_tab2->findChild<QCheckBox*>("m_checkUnused")->setChecked(m_initialState.isIncludingUnusedAccounts());
  } else if (m_tab3) {
    KComboBox *combo = m_tab3->findChild<KComboBox*>("m_comboOrganizeBy");
    switch (m_initialState.rowType()) {
      case MyMoneyReport::Row::NoRows:
      case MyMoneyReport::Row::Category:
        combo->setCurrentItem(i18n("Categories"), false);
        break;
      case MyMoneyReport::Row::TopCategory:
        combo->setCurrentItem(i18n("Top Categories"), false);
        break;
      case MyMoneyReport::Row::Tag:
        combo->setCurrentItem(i18n("Tags"), false);
        break;
      case MyMoneyReport::Row::Payee:
        combo->setCurrentItem(i18n("Payees"), false);
        break;
      case MyMoneyReport::Row::Account:
        combo->setCurrentItem(i18n("Accounts"), false);
        break;
      case MyMoneyReport::Row::TopAccount:
        combo->setCurrentItem(i18n("Top Accounts"), false);
        break;
      case MyMoneyReport::Row::Month:
        combo->setCurrentItem(i18n("Month"), false);
        break;
      case MyMoneyReport::Row::Week:
        combo->setCurrentItem(i18n("Week"), false);
        break;
      default:
        throw MYMONEYEXCEPTION("KReportConfigurationFilterDlg::slotReset(): QueryTable report has invalid rowtype");
    }

    unsigned qc = m_initialState.queryColumns();
    m_tab3->findChild<QCheckBox*>("m_checkNumber")->setChecked(qc & MyMoneyReport::QueryColumns::Number);
    m_tab3->findChild<QCheckBox*>("m_checkPayee")->setChecked(qc & MyMoneyReport::QueryColumns::Payee);
    m_tab3->findChild<QCheckBox*>("m_checkTag")->setChecked(qc & MyMoneyReport::QueryColumns::Tag);
    m_tab3->findChild<QCheckBox*>("m_checkCategory")->setChecked(qc & MyMoneyReport::QueryColumns::Category);
    m_tab3->findChild<QCheckBox*>("m_checkMemo")->setChecked(qc & MyMoneyReport::QueryColumns::Memo);
    m_tab3->findChild<QCheckBox*>("m_checkAccount")->setChecked(qc & MyMoneyReport::QueryColumns::Account);
    m_tab3->findChild<QCheckBox*>("m_checkReconciled")->setChecked(qc & MyMoneyReport::QueryColumns::Reconciled);
    m_tab3->findChild<QCheckBox*>("m_checkAction")->setChecked(qc & MyMoneyReport::QueryColumns::Action);
    m_tab3->findChild<QCheckBox*>("m_checkShares")->setChecked(qc & MyMoneyReport::QueryColumns::Shares);
    m_tab3->findChild<QCheckBox*>("m_checkPrice")->setChecked(qc & MyMoneyReport::QueryColumns::Price);
    m_tab3->findChild<QCheckBox*>("m_checkBalance")->setChecked(qc & MyMoneyReport::QueryColumns::Balance);

    m_tab3->findChild<QCheckBox*>("m_checkTax")->setChecked(m_initialState.isTax());
    m_tab3->findChild<QCheckBox*>("m_checkInvestments")->setChecked(m_initialState.isInvestmentsOnly());
    m_tab3->findChild<QCheckBox*>("m_checkLoans")->setChecked(m_initialState.isLoansOnly());

    m_tab3->findChild<QCheckBox*>("m_checkHideSplitDetails")->setChecked
    (m_initialState.detailLevel() == MyMoneyReport::DetailLevel::None);
  }

  if (m_tabChart) {
    KMyMoneyGeneralCombo* combo = m_tabChart->findChild<KMyMoneyGeneralCombo*>("m_comboType");
    switch (m_initialState.chartType()) {
      case MyMoneyReport::Chart::None:
        combo->setCurrentItem(MyMoneyReport::Chart::Line);
        break;
      case MyMoneyReport::Chart::Line:
      case MyMoneyReport::Chart::Bar:
      case MyMoneyReport::Chart::StackedBar:
      case MyMoneyReport::Chart::Pie:
      case MyMoneyReport::Chart::Ring:
        combo->setCurrentItem(m_initialState.chartType());
        break;
      case MyMoneyReport::Chart::End:
        throw MYMONEYEXCEPTION("KReportConfigurationFilterDlg::slotReset(): Report has invalid charttype");
    }
    // keep in sync with kMyMoneyReportConfigTabChartDecl::kMyMoneyReportConfigTabChartDecl
    KMyMoneyGeneralCombo* palette = m_tabChart->findChild<KMyMoneyGeneralCombo*>("m_chartPalette");
    palette->setCurrentIndex(m_initialState.chartPalette());
    m_tabChart->findChild<QCheckBox*>("m_checkGridLines")->setChecked(m_initialState.isChartGridLines());
    m_tabChart->findChild<QCheckBox*>("m_checkValues")->setChecked(m_initialState.isChartDataLabels());
    m_tabChart->findChild<QCheckBox*>("m_checkShowChart")->setChecked(m_initialState.isChartByDefault());
    m_tabChart->findChild<QSpinBox*>("m_lineWidth")->setValue(m_initialState.chartLineWidth());
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
  // Validity Filters
  //
  int validity;
  if (m_initialState.firstValidity(validity))
    m_ui->m_validityBox->setCurrentIndex(validity);

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
    if (m_initialState.isUserDefined()) {
      m_ui->m_dateRange->setCurrentItem(MyMoneyTransactionFilter::userDefined);
      m_ui->m_fromDate->setDate(dateFrom);
      m_ui->m_toDate->setDate(dateTo);
    } else {
      m_ui->m_fromDate->setDate(dateFrom);
      m_ui->m_toDate->setDate(dateTo);
      KFindTransactionDlg::slotDateChanged();
    }
  } else {
    m_ui->m_dateRange->setCurrentItem(MyMoneyTransactionFilter::allDates);
    slotDateRangeChanged(MyMoneyTransactionFilter::allDates);
  }

  slotRightSize();
}

void KReportConfigurationFilterDlg::slotDateChanged()
{
  if (m_ui->m_dateRange->currentItem() != MyMoneyTransactionFilter::userDefined) {
    KFindTransactionDlg::slotDateChanged();
  }
  slotUpdateSelections();
}

void KReportConfigurationFilterDlg::slotShowHelp()
{
  KToolInvocation::invokeHelp("details.reports.config");
}

//TODO Fix the reports and engine to include transfers even if categories are filtered - bug #1523508
void KReportConfigurationFilterDlg::slotUpdateCheckTransfers()
{
  QCheckBox* cb = m_tab2->findChild<QCheckBox*>("m_checkTransfers");
  if (!m_ui->m_categoriesView->allItemsSelected()) {
    cb->setChecked(false);
    cb->setDisabled(true);
  } else {
    cb->setEnabled(true);
  }
}
