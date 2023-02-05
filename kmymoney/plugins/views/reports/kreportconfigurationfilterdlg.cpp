/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Ace Jones <ace.j@hotpop.com>
    SPDX-FileCopyrightText: 2017, 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kreportconfigurationfilterdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ktransactionfilter.h"
#include "kmymoneyaccountselector.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneybudget.h"
#include "mymoneyreport.h"
#include "daterangedlg.h"
#include "reporttabimpl.h"
#include "mymoneyenums.h"

#include <ui_kreportconfigurationfilterdlg.h>
#include <ui_reporttabgeneral.h>
#include <ui_reporttabrowcolpivot.h>
#include <ui_reporttabrowcolquery.h>
#include <ui_reporttabchart.h>
#include <ui_reporttabrange.h>
#include <ui_reporttabcapitalgain.h>
#include <ui_reporttabperformance.h>

class KReportConfigurationFilterDlgPrivate
{
    Q_DISABLE_COPY(KReportConfigurationFilterDlgPrivate)

public:
    KReportConfigurationFilterDlgPrivate(KReportConfigurationFilterDlg *qq) :
        q_ptr(qq),
        ui(new Ui::KReportConfigurationFilterDlg),
        m_tabRowColPivot(nullptr),
        m_tabRowColQuery(nullptr),
        m_tabChart(nullptr),
        m_tabRange(nullptr),
        m_dateRange(nullptr)
    {
    }

    ~KReportConfigurationFilterDlgPrivate()
    {
        delete ui;
    }

    KReportConfigurationFilterDlg      *q_ptr;
    Ui::KReportConfigurationFilterDlg  *ui;

    QPointer<ReportTabGeneral>     m_tabGeneral;
    QPointer<ReportTabRowColPivot> m_tabRowColPivot;
    QPointer<ReportTabRowColQuery> m_tabRowColQuery;
    QPointer<ReportTabChart>       m_tabChart;
    QPointer<ReportTabRange>       m_tabRange;
    QPointer<ReportTabCapitalGain> m_tabCapitalGain;
    QPointer<ReportTabPerformance> m_tabPerformance;
    QPointer<KTransactionFilter>           m_tabFilters;

    MyMoneyReport m_initialState;
    MyMoneyReport m_currentState;
    QVector<MyMoneyBudget> m_budgets;
    DateRangeDlg                    *m_dateRange;
};

KReportConfigurationFilterDlg::KReportConfigurationFilterDlg(MyMoneyReport report, QWidget *parent) :
    QDialog(parent),
    d_ptr(new KReportConfigurationFilterDlgPrivate(this))
{
    Q_D(KReportConfigurationFilterDlg);

    d->ui->setupUi(this);
    d->m_initialState = report;
    d->m_currentState = report;

    //
    // Rework labeling
    //

    setWindowTitle(i18n("Report Configuration"));
    //
    // Rework the buttons
    //

    // the Apply button is always enabled
    d->ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
    d->ui->buttonBox->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip for report configuration apply button", "Apply the configuration changes to the report"));


    connect(d->ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &KReportConfigurationFilterDlg::slotSearch);
    connect(d->ui->buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked, this, &QDialog::close);
    connect(d->ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &KReportConfigurationFilterDlg::slotReset);
    connect(d->ui->buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, this, &KReportConfigurationFilterDlg::slotShowHelp);

    //
    // Add new tabs
    //
    if (d->m_initialState.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        // we will use date range together with data range
        d->m_tabFilters = new KTransactionFilter(this, (report.rowType() == eMyMoney::Report::RowType::Account), true, false);
    } else {
        d->m_tabFilters = new KTransactionFilter(this, (report.rowType() == eMyMoney::Report::RowType::Account), report.isInvestmentsOnly());
        d->m_dateRange = d->m_tabFilters->dateRange();
    }

    d->ui->m_tabWidget->addTab(d->m_tabFilters, i18nc("Filters tab", "Filters"));

    d->m_tabGeneral = new ReportTabGeneral(d->ui->m_criteriaTab);
    d->ui->m_criteriaTab->insertTab(0, d->m_tabGeneral, i18nc("General tab", "General"));

    if (d->m_initialState.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        int tabNr = 1;
        if (!(d->m_initialState.isIncludingPrice() || d->m_initialState.isIncludingAveragePrice())) {
            d->m_tabRowColPivot = new ReportTabRowColPivot(d->ui->m_criteriaTab);
            d->ui->m_criteriaTab->insertTab(tabNr++, d->m_tabRowColPivot, i18n("Rows/Columns"));
            connect(d->m_tabRowColPivot->ui->m_comboRows, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KReportConfigurationFilterDlg::*)(int)>(&KReportConfigurationFilterDlg::slotRowTypeChanged));
            connect(d->m_tabRowColPivot->ui->m_comboRows, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KReportConfigurationFilterDlg::*)(int)>(&KReportConfigurationFilterDlg::slotUpdateColumnsCombo));
            //control the state of the includeTransfer check
            connect(d->m_tabFilters->categoriesView(), &KMyMoneySelector::stateChanged, this, &KReportConfigurationFilterDlg::slotUpdateCheckTransfers);
        }

        d->m_tabChart = new ReportTabChart(d->ui->m_criteriaTab);
        d->ui->m_criteriaTab->insertTab(tabNr++, d->m_tabChart, i18n("Chart"));

        d->m_tabRange = new ReportTabRange(d->ui->m_criteriaTab);
        d->ui->m_criteriaTab->insertTab(tabNr, d->m_tabRange, i18n("Range"));

        d->m_dateRange = d->m_tabRange->m_dateRange;

        if (!(d->m_initialState.isIncludingPrice() || d->m_initialState.isIncludingAveragePrice())) {
            connect(d->m_tabRange->ui->m_comboColumns, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KReportConfigurationFilterDlg::slotColumnTypeChanged);
            connect(d->m_tabRange->ui->m_comboColumns, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KReportConfigurationFilterDlg::*)(int)>(&KReportConfigurationFilterDlg::slotUpdateColumnsCombo));
        }
        connect(d->m_tabChart->ui->m_logYaxis, &QCheckBox::stateChanged, this, &KReportConfigurationFilterDlg::slotLogAxisChanged);
        connect(d->m_tabChart->ui->m_negExpenses, &QCheckBox::stateChanged, this, &KReportConfigurationFilterDlg::slotNegExpensesChanged);
    } else if (d->m_initialState.reportType() == eMyMoney::Report::ReportType::QueryTable) {
        // eInvestmentHoldings is a special-case report, and you cannot configure the
        // rows & columns of that report.
        if (d->m_initialState.rowType() < eMyMoney::Report::RowType::AccountByTopAccount) {
            d->m_tabRowColQuery = new ReportTabRowColQuery(d->ui->m_criteriaTab);
            d->ui->m_criteriaTab->insertTab(1, d->m_tabRowColQuery, i18n("Rows/Columns"));
        }
        if (d->m_initialState.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain) {
            d->m_tabCapitalGain = new ReportTabCapitalGain(d->ui->m_criteriaTab);
            d->ui->m_criteriaTab->insertTab(1, d->m_tabCapitalGain, i18n("Report"));
        }
        if (d->m_initialState.queryColumns() & eMyMoney::Report::QueryColumn::Performance) {
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
    auto filter = d->m_tabFilters->setupFilter();

    // Copy the m_filter over to the filter part of m_currentConfig.
    d->m_currentState.assignFilter(filter);

    // Then extract the report properties
    d->m_currentState.setName(d->m_tabGeneral->ui->m_editName->text());
    d->m_currentState.setComment(d->m_tabGeneral->ui->m_editComment->text());
    d->m_currentState.setConvertCurrency(d->m_tabGeneral->ui->m_checkCurrency->isChecked());
    d->m_currentState.setFavorite(d->m_tabGeneral->ui->m_checkFavorite->isChecked());
    d->m_currentState.setSkipZero(d->m_tabGeneral->ui->m_skipZero->isChecked());

    if (d->m_tabRowColPivot) {
        eMyMoney::Report::DetailLevel dl[4] = { eMyMoney::Report::DetailLevel::All, eMyMoney::Report::DetailLevel::Top, eMyMoney::Report::DetailLevel::Group, eMyMoney::Report::DetailLevel::Total };

        d->m_currentState.setDetailLevel(dl[d->m_tabRowColPivot->ui->m_comboDetail->currentIndex()]);

        // modify the rowtype only if the widget is enabled
        if (d->m_tabRowColPivot->ui->m_comboRows->isEnabled()) {
            eMyMoney::Report::RowType rt[2] = { eMyMoney::Report::RowType::ExpenseIncome, eMyMoney::Report::RowType::AssetLiability };
            d->m_currentState.setRowType(rt[d->m_tabRowColPivot->ui->m_comboRows->currentIndex()]);
        }

        d->m_currentState.setShowingRowTotals(false);
        if (d->m_tabRowColPivot->ui->m_comboRows->currentIndex() == 0)
            d->m_currentState.setShowingRowTotals(d->m_tabRowColPivot->ui->m_checkTotalColumn->isChecked());

        d->m_currentState.setShowingColumnTotals(d->m_tabRowColPivot->ui->m_checkTotalRow->isChecked());
        d->m_currentState.setIncludingSchedules(d->m_tabRowColPivot->ui->m_checkScheduled->isChecked());
        d->m_currentState.setPropagateBudgetDifference(d->m_tabRowColPivot->ui->m_propagateRemainder->isChecked());
        d->m_currentState.setIncludingTransfers(d->m_tabRowColPivot->ui->m_checkTransfers->isChecked());

        d->m_currentState.setIncludingUnusedAccounts(d->m_tabRowColPivot->ui->m_checkUnused->isChecked());

        if (d->m_tabRowColPivot->ui->m_comboBudget->isEnabled() && (d->m_budgets.count() > 0)) {
            d->m_currentState.setBudget(d->m_budgets[d->m_tabRowColPivot->ui->m_comboBudget->currentItem()].id(), d->m_initialState.rowType() == eMyMoney::Report::RowType::BudgetActual);
        } else {
            d->m_currentState.setBudget(QString(), false);
        }

        //set moving average days
        if (d->m_tabRowColPivot->ui->m_movingAverageDays->isEnabled()) {
            d->m_currentState.setMovingAverageDays(d->m_tabRowColPivot->ui->m_movingAverageDays->value());
        }
    } else if (d->m_tabRowColQuery) {
        eMyMoney::Report::RowType rtq[8] = { eMyMoney::Report::RowType::Category, eMyMoney::Report::RowType::TopCategory, eMyMoney::Report::RowType::Tag, eMyMoney::Report::RowType::Payee, eMyMoney::Report::RowType::Account, eMyMoney::Report::RowType::TopAccount, eMyMoney::Report::RowType::Month, eMyMoney::Report::RowType::Week, };
        d->m_currentState.setRowType(rtq[d->m_tabRowColQuery->ui->m_comboOrganizeBy->currentIndex()]);

        unsigned qc = eMyMoney::Report::QueryColumn::None;

        if (d->m_currentState.queryColumns() & eMyMoney::Report::QueryColumn::Loan)
            // once a loan report, always a loan report
            qc = eMyMoney::Report::QueryColumn::Loan;

        if (d->m_tabRowColQuery->ui->m_checkNumber->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Number;
        if (d->m_tabRowColQuery->ui->m_checkPayee->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Payee;
        if (d->m_tabRowColQuery->ui->m_checkTag->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Tag;
        if (d->m_tabRowColQuery->ui->m_checkCategory->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Category;
        if (d->m_tabRowColQuery->ui->m_checkMemo->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Memo;
        if (d->m_tabRowColQuery->ui->m_checkAccount->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Account;
        if (d->m_tabRowColQuery->ui->m_checkReconciled->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Reconciled;
        if (d->m_tabRowColQuery->ui->m_checkAction->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Action;
        if (d->m_tabRowColQuery->ui->m_checkShares->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Shares;
        if (d->m_tabRowColQuery->ui->m_checkPrice->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Price;
        if (d->m_tabRowColQuery->ui->m_checkBalance->isChecked())
            qc |= eMyMoney::Report::QueryColumn::Balance;

        d->m_currentState.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(qc));

        d->m_currentState.setTax(d->m_tabRowColQuery->ui->m_checkTax->isChecked());
        d->m_currentState.setInvestmentsOnly(d->m_tabRowColQuery->ui->m_checkInvestments->isChecked());
        d->m_currentState.setLoansOnly(d->m_tabRowColQuery->ui->m_checkLoans->isChecked());

        d->m_currentState.setDetailLevel(d->m_tabRowColQuery->ui->m_checkHideSplitDetails->isChecked() ?
                                         eMyMoney::Report::DetailLevel::None : eMyMoney::Report::DetailLevel::All);
        d->m_currentState.setHideTransactions(d->m_tabRowColQuery->ui->m_checkHideTransactions->isChecked());
        d->m_currentState.setShowingColumnTotals(!d->m_tabRowColQuery->ui->m_checkHideTotals->isChecked());

        d->m_currentState.setIncludingTransfers(d->m_tabRowColQuery->ui->m_checkTransfers->isChecked());
    }

    if (d->m_tabChart) {
        eMyMoney::Report::ChartType ct[5] = { eMyMoney::Report::ChartType::Line, eMyMoney::Report::ChartType::Bar, eMyMoney::Report::ChartType::StackedBar, eMyMoney::Report::ChartType::Pie, eMyMoney::Report::ChartType::Ring };
        eMyMoney::Report::ChartPalette cp[4] = { eMyMoney::Report::ChartPalette::Application, eMyMoney::Report::ChartPalette::Default, eMyMoney::Report::ChartPalette::Rainbow, eMyMoney::Report::ChartPalette::Subdued };
        d->m_currentState.setChartType(ct[d->m_tabChart->ui->m_comboType->currentIndex()]);
        d->m_currentState.setChartPalette(cp[d->m_tabChart->ui->m_comboPalette->currentIndex()]);
        d->m_currentState.setChartCHGridLines(d->m_tabChart->ui->m_checkCHGridLines->isChecked());
        d->m_currentState.setChartSVGridLines(d->m_tabChart->ui->m_checkSVGridLines->isChecked());
        d->m_currentState.setChartDataLabels(d->m_tabChart->ui->m_checkValues->isChecked());
        d->m_currentState.setChartByDefault(d->m_tabChart->ui->m_checkShowChart->isChecked());
        d->m_currentState.setChartLineWidth(d->m_tabChart->ui->m_lineWidth->value());
        d->m_currentState.setLogYAxis(d->m_tabChart->ui->m_logYaxis->isChecked());
        d->m_currentState.setNegExpenses(d->m_tabChart->ui->m_negExpenses->isChecked());
    }

    if (d->m_tabRange) {
        d->m_currentState.setDataRangeStart(d->m_tabRange->ui->m_dataRangeStart->text());
        d->m_currentState.setDataRangeEnd(d->m_tabRange->ui->m_dataRangeEnd->text());
        d->m_currentState.setDataMajorTick(d->m_tabRange->ui->m_dataMajorTick->text());
        d->m_currentState.setDataMinorTick(d->m_tabRange->ui->m_dataMinorTick->text());
        d->m_currentState.setYLabelsPrecision(d->m_tabRange->ui->m_yLabelsPrecision->value());
        d->m_currentState.setDataFilter((eMyMoney::Report::DataLock)d->m_tabRange->ui->m_dataLock->currentIndex());

        eMyMoney::Report::ColumnType ct[6] = { eMyMoney::Report::ColumnType::Days, eMyMoney::Report::ColumnType::Weeks, eMyMoney::Report::ColumnType::Months, eMyMoney::Report::ColumnType::BiMonths, eMyMoney::Report::ColumnType::Quarters, eMyMoney::Report::ColumnType::Years, };
        bool dy[6] = { true, true, false, false, false, false };
        d->m_currentState.setColumnType(ct[d->m_tabRange->ui->m_comboColumns->currentIndex()]);

        //TODO (Ace) This should be implicit in the call above.  MMReport needs fixin'
        d->m_currentState.setColumnsAreDays(dy[d->m_tabRange->ui->m_comboColumns->currentIndex()]);
        d->m_currentState.setDateFilter(d->m_dateRange->fromDate(), d->m_dateRange->toDate());
    }

    // setup the date lock
    eMyMoney::TransactionFilter::Date range = d->m_dateRange->dateRange();
    d->m_currentState.setDateFilter(range);

    if (d->m_tabCapitalGain) {
        d->m_currentState.setTermSeparator(d->m_tabCapitalGain->ui->m_termSeparator->date());
        d->m_currentState.setShowSTLTCapitalGains(d->m_tabCapitalGain->ui->m_showSTLTCapitalGains->isChecked());
        d->m_currentState.setSettlementPeriod(d->m_tabCapitalGain->ui->m_settlementPeriod->value());
        d->m_currentState.setShowingColumnTotals(!d->m_tabCapitalGain->ui->m_checkHideTotals->isChecked());
        d->m_currentState.setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(d->m_tabCapitalGain->ui->m_investmentSum->currentData().toInt()));
    }

    if (d->m_tabPerformance) {
        d->m_currentState.setShowingColumnTotals(!d->m_tabPerformance->ui->m_checkHideTotals->isChecked());
        d->m_currentState.setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(d->m_tabPerformance->ui->m_investmentSum->currentData().toInt()));
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

void KReportConfigurationFilterDlg::slotNegExpensesChanged(int state)
{
    Q_D(KReportConfigurationFilterDlg);
    d->m_tabChart->setNegExpenses(state == Qt::Checked);
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
        case eMyMoney::Report::DetailLevel::None:
        case eMyMoney::Report::DetailLevel::End:
        case eMyMoney::Report::DetailLevel::All:
            combo->setCurrentItem(i18nc("All accounts", "All"), false);
            break;
        case eMyMoney::Report::DetailLevel::Top:
            combo->setCurrentItem(i18n("Top-Level"), false);
            break;
        case eMyMoney::Report::DetailLevel::Group:
            combo->setCurrentItem(i18n("Groups"), false);
            break;
        case eMyMoney::Report::DetailLevel::Total:
            combo->setCurrentItem(i18n("Totals"), false);
            break;
        }

        combo = d->m_tabRowColPivot->ui->m_comboRows;
        switch (d->m_initialState.rowType()) {
        case eMyMoney::Report::RowType::ExpenseIncome:
        case eMyMoney::Report::RowType::Budget:
        case eMyMoney::Report::RowType::BudgetActual:
            combo->setCurrentItem(i18n("Income & Expenses"), false); // income / expense
            break;
        default:
            combo->setCurrentItem(i18n("Assets & Liabilities"), false); // asset / liability
            break;
        }
        d->m_tabRowColPivot->ui->m_checkTotalColumn->setChecked(d->m_initialState.isShowingRowTotals());
        d->m_tabRowColPivot->ui->m_checkTotalRow->setChecked(d->m_initialState.isShowingColumnTotals());
        d->m_tabRowColPivot->ui->m_propagateRemainder->setEnabled(d->m_initialState.rowType() == eMyMoney::Report::RowType::BudgetActual);
        d->m_tabRowColPivot->ui->m_propagateRemainder->setChecked(d->m_initialState.isPropagateBudgetDifference());
        d->m_tabRowColPivot->ui->m_checkTotalRow->setDisabled(d->m_initialState.isPropagateBudgetDifference());

        connect(d->m_tabRowColPivot->ui->m_propagateRemainder, &QCheckBox::stateChanged, this, [&](int _state) {
            Q_D(KReportConfigurationFilterDlg);
            const auto state = static_cast<Qt::CheckState>(_state);
            d->m_tabRowColPivot->ui->m_checkTotalColumn->setDisabled(state == Qt::Checked);
            switch (state) {
            case Qt::Checked:
                d->m_tabRowColPivot->ui->m_checkTotalColumn->setChecked(false);
                break;
            default:
                break;
            }
        });

        slotRowTypeChanged(combo->currentIndex());

        //load budgets combo
        d->m_tabRowColPivot->ui->m_comboBudget->setDisabled(true);
        if (d->m_initialState.rowType() == eMyMoney::Report::RowType::Budget
                || d->m_initialState.rowType() == eMyMoney::Report::RowType::BudgetActual) {
            d->m_tabRowColPivot->ui->m_comboBudget->setEnabled(true);
            d->m_tabRowColPivot->ui->m_comboRows->setEnabled(false);
            d->m_tabRowColPivot->ui->m_rowsLabel->setEnabled(false);
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
        d->m_tabRowColPivot->ui->m_movingAverageLabel->setEnabled(d->m_initialState.isIncludingMovingAverage());

        if (d->m_initialState.isIncludingMovingAverage()) {
            spinbox->setValue(d->m_initialState.movingAverageDays());
        }

        d->m_tabRowColPivot->ui->m_checkScheduled->setChecked(d->m_initialState.isIncludingSchedules());
        d->m_tabRowColPivot->ui->m_checkTransfers->setChecked(d->m_initialState.isIncludingTransfers());
        d->m_tabRowColPivot->ui->m_checkUnused->setChecked(d->m_initialState.isIncludingUnusedAccounts());
    } else if (d->m_tabRowColQuery) {
        KComboBox *combo = d->m_tabRowColQuery->ui->m_comboOrganizeBy;
        switch (d->m_initialState.rowType()) {
        case eMyMoney::Report::RowType::NoRows:
        case eMyMoney::Report::RowType::Category:
            combo->setCurrentItem(i18n("Categories"), false);
            break;
        case eMyMoney::Report::RowType::TopCategory:
            combo->setCurrentItem(i18n("Top Categories"), false);
            break;
        case eMyMoney::Report::RowType::Tag:
            combo->setCurrentItem(i18n("Tags"), false);
            break;
        case eMyMoney::Report::RowType::Payee:
            combo->setCurrentItem(i18n("Payees"), false);
            break;
        case eMyMoney::Report::RowType::Account:
            combo->setCurrentItem(i18n("Accounts"), false);
            break;
        case eMyMoney::Report::RowType::TopAccount:
            combo->setCurrentItem(i18n("Top Accounts"), false);
            break;
        case eMyMoney::Report::RowType::Month:
            combo->setCurrentItem(i18n("Month"), false);
            break;
        case eMyMoney::Report::RowType::Week:
            combo->setCurrentItem(i18n("Week"), false);
            break;
        default:
            throw MYMONEYEXCEPTION_CSTRING("KReportConfigurationFilterDlg::slotReset(): QueryTable report has invalid rowtype");
        }

        unsigned qc = d->m_initialState.queryColumns();
        d->m_tabRowColQuery->ui->m_checkNumber->setChecked(qc & eMyMoney::Report::QueryColumn::Number);
        d->m_tabRowColQuery->ui->m_checkPayee->setChecked(qc & eMyMoney::Report::QueryColumn::Payee);
        d->m_tabRowColQuery->ui->m_checkTag->setChecked(qc & eMyMoney::Report::QueryColumn::Tag);
        d->m_tabRowColQuery->ui->m_checkCategory->setChecked(qc & eMyMoney::Report::QueryColumn::Category);
        d->m_tabRowColQuery->ui->m_checkMemo->setChecked(qc & eMyMoney::Report::QueryColumn::Memo);
        d->m_tabRowColQuery->ui->m_checkAccount->setChecked(qc & eMyMoney::Report::QueryColumn::Account);
        d->m_tabRowColQuery->ui->m_checkReconciled->setChecked(qc & eMyMoney::Report::QueryColumn::Reconciled);
        d->m_tabRowColQuery->ui->m_checkAction->setChecked(qc & eMyMoney::Report::QueryColumn::Action);
        d->m_tabRowColQuery->ui->m_checkShares->setChecked(qc & eMyMoney::Report::QueryColumn::Shares);
        d->m_tabRowColQuery->ui->m_checkPrice->setChecked(qc & eMyMoney::Report::QueryColumn::Price);
        d->m_tabRowColQuery->ui->m_checkBalance->setChecked(qc & eMyMoney::Report::QueryColumn::Balance);

        d->m_tabRowColQuery->ui->m_checkTax->setChecked(d->m_initialState.isTax());
        d->m_tabRowColQuery->ui->m_checkInvestments->setChecked(d->m_initialState.isInvestmentsOnly());
        d->m_tabRowColQuery->ui->m_checkLoans->setChecked(d->m_initialState.isLoansOnly());

        d->m_tabRowColQuery->ui->m_checkHideTransactions->setChecked(d->m_initialState.isHideTransactions());
        d->m_tabRowColQuery->ui->m_checkHideTotals->setChecked(!d->m_initialState.isShowingColumnTotals());
        d->m_tabRowColQuery->ui->m_checkHideSplitDetails->setEnabled(!d->m_initialState.isHideTransactions());

        d->m_tabRowColQuery->ui->m_checkHideSplitDetails->setChecked
        (d->m_initialState.detailLevel() == eMyMoney::Report::DetailLevel::None || d->m_initialState.isHideTransactions());
        d->m_tabRowColQuery->ui->m_checkTransfers->setChecked(d->m_initialState.isIncludingTransfers());
    }

    if (d->m_tabChart) {
        KMyMoneyGeneralCombo* combo = d->m_tabChart->ui->m_comboType;
        switch (d->m_initialState.chartType()) {
        case eMyMoney::Report::ChartType::None:
            combo->setCurrentItem(static_cast<int>(eMyMoney::Report::ChartType::Line));
            break;
        case eMyMoney::Report::ChartType::Line:
        case eMyMoney::Report::ChartType::Bar:
        case eMyMoney::Report::ChartType::StackedBar:
        case eMyMoney::Report::ChartType::Pie:
        case eMyMoney::Report::ChartType::Ring:
            combo->setCurrentItem(static_cast<int>(d->m_initialState.chartType()));
            break;
        default:
            throw MYMONEYEXCEPTION_CSTRING("KReportConfigurationFilterDlg::slotReset(): Report has invalid charttype");
        }
        combo = d->m_tabChart->ui->m_comboPalette;
        switch (d->m_initialState.chartPalette()) {
        case eMyMoney::Report::ChartPalette::Application:
        case eMyMoney::Report::ChartPalette::Default:
        case eMyMoney::Report::ChartPalette::Rainbow:
        case eMyMoney::Report::ChartPalette::Subdued:
            combo->setCurrentItem(static_cast<int>(d->m_initialState.chartPalette()));
            break;
        default:
            throw MYMONEYEXCEPTION_CSTRING("KReportConfigurationFilterDlg::slotReset(): Report has invalid chartpalette");
        }
        d->m_tabChart->ui->m_checkCHGridLines->setChecked(d->m_initialState.isChartCHGridLines());
        d->m_tabChart->ui->m_checkSVGridLines->setChecked(d->m_initialState.isChartSVGridLines());
        d->m_tabChart->ui->m_checkValues->setChecked(d->m_initialState.isChartDataLabels());
        d->m_tabChart->ui->m_checkShowChart->setChecked(d->m_initialState.isChartByDefault());
        d->m_tabChart->ui->m_lineWidth->setValue(d->m_initialState.chartLineWidth());
        d->m_tabChart->ui->m_logYaxis->setChecked(d->m_initialState.isLogYAxis());
        d->m_tabChart->ui->m_negExpenses->setChecked(d->m_initialState.isNegExpenses());
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
            case eMyMoney::Report::ColumnType::NoColumns:
            case eMyMoney::Report::ColumnType::Days:
                combo->setCurrentItem(i18nc("@item the columns will display daily data", "Daily"), false);
                break;
            case eMyMoney::Report::ColumnType::Weeks:
                combo->setCurrentItem(i18nc("@item the columns will display weekly data", "Weekly"), false);
                break;
            default:
                break;
            }
        } else {
            switch (d->m_initialState.columnType()) {
            case eMyMoney::Report::ColumnType::NoColumns:
            case eMyMoney::Report::ColumnType::Months:
                combo->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
                break;
            case eMyMoney::Report::ColumnType::BiMonths:
                combo->setCurrentItem(i18nc("@item the columns will display bi-monthly data", "Bi-Monthly"), false);
                break;
            case eMyMoney::Report::ColumnType::Quarters:
                combo->setCurrentItem(i18nc("@item the columns will display quarterly data", "Quarterly"), false);
                break;
            case eMyMoney::Report::ColumnType::Years:
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
        d->m_tabCapitalGain->ui->m_investmentSum->addItem(i18n("Only owned"), static_cast<int>(eMyMoney::Report::InvestmentSum::Owned));
        d->m_tabCapitalGain->ui->m_investmentSum->addItem(i18n("Only sold"), static_cast<int>(eMyMoney::Report::InvestmentSum::Sold));
        d->m_tabCapitalGain->ui->m_investmentSum->blockSignals(false);
        d->m_tabCapitalGain->ui->m_investmentSum->setCurrentIndex(d->m_tabCapitalGain->ui->m_investmentSum->findData(static_cast<int>(d->m_initialState.investmentSum())));
    }

    if (d->m_tabPerformance) {
        d->m_tabPerformance->ui->m_checkHideTotals->setChecked(!d->m_initialState.isShowingColumnTotals());
        d->m_tabPerformance->ui->m_investmentSum->blockSignals(true);
        d->m_tabPerformance->ui->m_investmentSum->clear();
        d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("From period"), static_cast<int>(eMyMoney::Report::InvestmentSum::Period));
        d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("Owned and sold"), static_cast<int>(eMyMoney::Report::InvestmentSum::OwnedAndSold));
        d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("Only owned"), static_cast<int>(eMyMoney::Report::InvestmentSum::Owned));
        d->m_tabPerformance->ui->m_investmentSum->addItem(i18n("Only sold"), static_cast<int>(eMyMoney::Report::InvestmentSum::Sold));
        d->m_tabPerformance->ui->m_investmentSum->blockSignals(false);
        d->m_tabPerformance->ui->m_investmentSum->setCurrentIndex(d->m_tabPerformance->ui->m_investmentSum->findData(static_cast<int>(d->m_initialState.investmentSum())));
    }

    d->m_tabFilters->resetFilter(d->m_initialState);

    if (d->m_dateRange) {
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
    }
}

void KReportConfigurationFilterDlg::slotShowHelp()
{
    Q_D(KReportConfigurationFilterDlg);
    if (d->ui->m_tabWidget->currentIndex() == 1)
        d->m_tabFilters->slotShowHelp();
    else
        KHelpClient::invokeHelp("details.reports.config");
}

//TODO Fix the reports and engine to include transfers even if categories are filtered - bug #1523508
void KReportConfigurationFilterDlg::slotUpdateCheckTransfers()
{
    Q_D(KReportConfigurationFilterDlg);
    auto cb = d->m_tabRowColPivot->ui->m_checkTransfers;
    if (!d->m_tabFilters->categoriesView()->allItemsSelected()) {
        cb->setChecked(false);
        cb->setDisabled(true);
    } else {
        cb->setEnabled(true);
    }
}
