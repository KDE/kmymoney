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

#include <QFrame>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "daterangedlg.h"
#include "kmymoneyaccountselector.h"
#include "ktransactionfilter.h"
#include "mymoneybudget.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#include "pricemodel.h"
#include "reporttabimpl.h"
#include "reporttabrowcolpivot.h"
#include "reporttabrowcolquery.h"

#include <ui_kreportconfigurationfilterdlg.h>
#include <ui_reporttabcapitalgain.h>
#include <ui_reporttabchart.h>
#include <ui_reporttabgeneral.h>
#include <ui_reporttabperformance.h>
#include <ui_reporttabrange.h>
#include <ui_reporttabrowcolpivot.h>

class KReportConfigurationFilterDlgPrivate
{
    Q_DISABLE_COPY(KReportConfigurationFilterDlgPrivate)

public:
    KReportConfigurationFilterDlgPrivate(KReportConfigurationFilterDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::KReportConfigurationFilterDlg)
        , m_tabRowColPivot(nullptr)
        , m_tabRowColQuery(nullptr)
        , m_tabChart(nullptr)
        , m_tabRange(nullptr)
        , m_dateRange(nullptr)
        , m_type(KReportConfigurationFilterDlg::Type::Default)

    {
    }

    ~KReportConfigurationFilterDlgPrivate()
    {
        delete ui;
    }

    KReportConfigurationFilterDlg* q_ptr;
    Ui::KReportConfigurationFilterDlg* ui;

    QPointer<ReportTabGeneral> m_tabGeneral;
    QPointer<ReportTabRowColPivot> m_tabRowColPivot;
    QPointer<ReportTabRowColQuery> m_tabRowColQuery;
    QPointer<ReportTabChart> m_tabChart;
    QPointer<ReportTabRange> m_tabRange;
    QPointer<ReportTabCapitalGain> m_tabCapitalGain;
    QPointer<ReportTabPerformance> m_tabPerformance;
    QPointer<KTransactionFilter> m_tabFilters;

    MyMoneyReport m_initialState;
    MyMoneyReport m_currentState;
    QVector<MyMoneyBudget> m_budgets;
    DateRangeDlg* m_dateRange;
    KReportConfigurationFilterDlg::Type m_type;
};

KReportConfigurationFilterDlg::KReportConfigurationFilterDlg(const MyMoneyReport& report, Type type, QWidget* parent)
    : QDialog(parent)
    , d_ptr(new KReportConfigurationFilterDlgPrivate(this))
{
    Q_D(KReportConfigurationFilterDlg);

    d->ui->setupUi(this);
    d->m_type = type;
    d->m_initialState = report;
    d->m_currentState = report;

    //
    // Rework labeling
    //

    setWindowTitle(i18n("Report Configuration"));
    d->ui->m_tabWidget->setTabPosition(QTabWidget::East);
    d->ui->m_criteriaTab->setTabPosition(QTabWidget::North);
    d->ui->m_criteriaTab->tabBar()->setUsesScrollButtons(false);
    //
    // Rework the buttons
    //

    // the Apply button is always enabled
    d->ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
    d->ui->buttonBox->button(QDialogButtonBox::Apply)
        ->setToolTip(i18nc("@info:tooltip for report configuration apply button", "Apply the configuration changes to the report"));

    connect(d->ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &KReportConfigurationFilterDlg::slotSearch);
    connect(d->ui->buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked, this, [this]() {
        if (isWindow()) {
            close();
        } else {
            Q_EMIT closeRequested();
        }
    });
    connect(d->ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &KReportConfigurationFilterDlg::slotReset);
    connect(d->ui->buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, this, &KReportConfigurationFilterDlg::slotShowHelp);

    //
    // Add new tabs
    //
    if (d->m_type == KReportConfigurationFilterDlg::Type::StaticEvaluationSafe) {
        d->m_tabChart = new ReportTabChart(d->ui->m_criteriaTab);
        d->ui->m_criteriaTab->insertTab(1, d->m_tabChart, i18n("Chart"));
        slotReset();
        return;
    } else if (d->m_initialState.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        // we will use date range together with data range
        d->m_tabFilters = new KTransactionFilter(this, (report.rowType() == eMyMoney::Report::RowType::Account), true, false);
    } else {
        d->m_tabFilters = new KTransactionFilter(this, (report.rowType() == eMyMoney::Report::RowType::Account), report.isInvestmentsOnly());
        d->m_dateRange = d->m_tabFilters->dateRange();
    }

    d->ui->m_tabWidget->addTab(d->m_tabFilters, i18nc("Filters tab", "Filters"));
    d->m_tabFilters->setCriteriaTabPosition(QTabWidget::North);

    d->m_tabGeneral = new ReportTabGeneral(d->ui->m_criteriaTab);
    d->ui->m_criteriaTab->insertTab(0, d->m_tabGeneral, i18nc("General tab", "General"));

    if (d->m_initialState.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        int tabNr = 1;
        if (!(d->m_initialState.isIncludingPrice() || d->m_initialState.isIncludingAveragePrice())) {
            d->m_tabRowColPivot = new ReportTabRowColPivot(d->ui->m_criteriaTab);
            d->ui->m_criteriaTab->insertTab(tabNr++, d->m_tabRowColPivot, i18n("Rows/Columns"));
            connect(d->m_tabRowColPivot, &ReportTabRowColPivot::comboRowsActivated, this, &KReportConfigurationFilterDlg::slotUpdateColumnsCombo);
            // control the state of the includeTransfer check
            connect(d->m_tabFilters->categoriesView(), &KMyMoneySelector::stateChanged, this, &KReportConfigurationFilterDlg::slotUpdateCheckTransfers);
        }

        d->m_tabChart = new ReportTabChart(d->ui->m_criteriaTab);
        d->ui->m_criteriaTab->insertTab(tabNr++, d->m_tabChart, i18n("Chart"));

        d->m_tabRange = new ReportTabRange(d->ui->m_criteriaTab);
        d->ui->m_criteriaTab->insertTab(tabNr, d->m_tabRange, i18n("Range"));

        d->m_dateRange = d->m_tabRange->m_dateRange;

        if (!(d->m_initialState.isIncludingPrice() || d->m_initialState.isIncludingAveragePrice())) {
            connect(d->m_tabRange->ui->m_comboColumns, &QComboBox::activated, this, &KReportConfigurationFilterDlg::slotColumnTypeChanged);
            connect(d->m_tabRange->ui->m_comboColumns, &QComboBox::activated, this, &KReportConfigurationFilterDlg::slotUpdateColumnsCombo);
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
        if (d->m_initialState.isCapitalGainReport()) {
            d->m_tabCapitalGain = new ReportTabCapitalGain(d->ui->m_criteriaTab);
            d->ui->m_criteriaTab->insertTab(1, d->m_tabCapitalGain, i18n("Report"));
        }
        if (d->m_initialState.isPerformanceReport()) {
            d->m_tabPerformance = new ReportTabPerformance(d->ui->m_criteriaTab);
            d->ui->m_criteriaTab->insertTab(1, d->m_tabPerformance, i18n("Report"));
        }
    }

    connect(d->m_tabGeneral->ui->m_checkCurrency, &QCheckBox::stateChanged, this, &KReportConfigurationFilterDlg::slotConvertCurrencyChanged);

    auto criteriaSections = new QWidget(d->ui->m_reportPage);
    auto criteriaSectionsLayout = new QVBoxLayout(criteriaSections);
    criteriaSectionsLayout->setContentsMargins(0, 0, 0, 0);
    criteriaSectionsLayout->setSpacing(style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    while (d->ui->m_criteriaTab->count() > 0) {
        auto sectionWidget = d->ui->m_criteriaTab->widget(0);
        const auto sectionTitle = d->ui->m_criteriaTab->tabText(0);
        d->ui->m_criteriaTab->removeTab(0);

        auto sectionGroup = new QGroupBox(sectionTitle, criteriaSections);
        auto sectionLayout = new QVBoxLayout(sectionGroup);
        sectionLayout->setContentsMargins(6, 6, 6, 6);
        sectionWidget->setParent(sectionGroup);
        sectionWidget->show();
        sectionLayout->addWidget(sectionWidget);
        criteriaSectionsLayout->addWidget(sectionGroup);
    }
    criteriaSectionsLayout->addStretch();

    auto scrollArea = new QScrollArea(d->ui->m_reportPage);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(criteriaSections);

    auto reportPageLayout = qobject_cast<QVBoxLayout*>(d->ui->m_reportPage->layout());
    if (reportPageLayout) {
        reportPageLayout->replaceWidget(d->ui->m_criteriaTab, scrollArea);
        d->ui->m_criteriaTab->hide();
        for (int i = reportPageLayout->count() - 1; i >= 0; --i) {
            auto item = reportPageLayout->itemAt(i);
            if (item && item->spacerItem()) {
                reportPageLayout->removeItem(item);
                delete item;
            }
        }
        reportPageLayout->setStretchFactor(scrollArea, 1);
    }

    QList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
    QList<MyMoneyBudget>::const_iterator it_b;
    for (it_b = list.cbegin(); it_b != list.cend(); ++it_b) {
        d->m_budgets.push_back(*it_b);
    }

    //
    // Now set up the widgets with proper values
    //
    slotReset();

    restoreState();
}

KReportConfigurationFilterDlg::~KReportConfigurationFilterDlg()
{
}

MyMoneyReport KReportConfigurationFilterDlg::getConfig() const
{
    Q_D(const KReportConfigurationFilterDlg);
    return d->m_currentState;
}

void KReportConfigurationFilterDlg::loadReport(const MyMoneyReport& report)
{
    Q_D(KReportConfigurationFilterDlg);
    d->m_initialState = report;
    d->m_currentState = report;
    d->m_tabFilters->loadWidgets();
    slotReset();
}

void KReportConfigurationFilterDlg::setType(Type newType)
{
    Q_D(KReportConfigurationFilterDlg);
    d->m_type = newType;
}

KReportConfigurationFilterDlg::Type KReportConfigurationFilterDlg::type() const
{
    Q_D(const KReportConfigurationFilterDlg);
    return d->m_type;
}

void KReportConfigurationFilterDlg::slotConvertCurrencyChanged(int state)
{
    Q_D(KReportConfigurationFilterDlg);

    if (!d->m_tabRowColQuery)
        return;

    d->m_currentState.assignFilter(d->m_tabFilters->setupFilter());
    d->m_tabRowColQuery->applyConvertCurrencyChanged(&d->m_currentState, state);
}

void KReportConfigurationFilterDlg::slotSearch()
{
    Q_D(KReportConfigurationFilterDlg);

    if (d->m_type == KReportConfigurationFilterDlg::Type::StaticEvaluationSafe) {
        if (d->m_tabChart) {
            d->m_tabChart->apply(&d->m_currentState);
        }
        if (isWindow()) {
            done(true);
        } else {
            Q_EMIT configurationApplied(d->m_currentState);
        }
        return;
    }

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
        d->m_tabRowColPivot->apply(&d->m_currentState, d->m_initialState.rowType() == eMyMoney::Report::RowType::BudgetActual, d->m_budgets);
    } else if (d->m_tabRowColQuery) {
        d->m_tabRowColQuery->apply(&d->m_currentState);
    }

    if (d->m_tabChart) {
        d->m_tabChart->apply(&d->m_currentState);
    }

    if (d->m_tabRange) {
        d->m_currentState.setDataRangeStart(d->m_tabRange->ui->m_dataRangeStart->text());
        d->m_currentState.setDataRangeEnd(d->m_tabRange->ui->m_dataRangeEnd->text());
        d->m_currentState.setDataMajorTick(d->m_tabRange->ui->m_dataMajorTick->text());
        d->m_currentState.setDataMinorTick(d->m_tabRange->ui->m_dataMinorTick->text());
        d->m_currentState.setYLabelsPrecision(d->m_tabRange->ui->m_yLabelsPrecision->value());
        d->m_currentState.setDataFilter((eMyMoney::Report::DataLock)d->m_tabRange->ui->m_dataLock->currentIndex());

        eMyMoney::Report::ColumnType ct[6] = {
            eMyMoney::Report::ColumnType::Days,
            eMyMoney::Report::ColumnType::Weeks,
            eMyMoney::Report::ColumnType::Months,
            eMyMoney::Report::ColumnType::BiMonths,
            eMyMoney::Report::ColumnType::Quarters,
            eMyMoney::Report::ColumnType::Years,
        };
        bool dy[6] = {true, true, false, false, false, false};
        d->m_currentState.setColumnType(ct[d->m_tabRange->ui->m_comboColumns->currentIndex()]);

        // TODO (Ace) This should be implicit in the call above.  MMReport needs fixin'
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

    if (isWindow()) {
        done(true);
    } else {
        Q_EMIT configurationApplied(d->m_currentState);
    }
}

void KReportConfigurationFilterDlg::slotColumnTypeChanged(int row)
{
    Q_D(KReportConfigurationFilterDlg);
    if ((d->m_tabRowColPivot->comboBudgetEnabled() && row < 2)) {
        d->m_tabRange->ui->m_comboColumns->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
    }
}

void KReportConfigurationFilterDlg::slotUpdateColumnsCombo()
{
    Q_D(KReportConfigurationFilterDlg);
    const int monthlyIndex = 2;
    const bool isIncomeExpenseForecast = d->m_currentState.isIncludingForecast() && d->m_tabRowColPivot->comboRowsIsIncomeExpense();
    if (isIncomeExpenseForecast && d->m_tabRange->ui->m_comboColumns->currentIndex() != monthlyIndex) {
        d->m_tabRange->ui->m_comboColumns->setCurrentItem(i18nc("@item the columns will display monthly data", "Monthly"), false);
    }
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

    if (d->m_type == KReportConfigurationFilterDlg::Type::StaticEvaluationSafe) {
        if (d->m_tabChart) {
            d->m_tabChart->load(&d->m_initialState);
        }
        return;
    }

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
        d->m_tabRowColPivot->load(&d->m_initialState, d->m_budgets);
    } else if (d->m_tabRowColQuery) {
        d->m_tabRowColQuery->load(&d->m_initialState);
    }

    if (d->m_tabChart) {
        d->m_tabChart->load(&d->m_initialState);
    }

    if (d->m_tabRange) {
        d->m_tabRange->ui->m_dataRangeStart->setText(d->m_initialState.dataRangeStart());
        d->m_tabRange->ui->m_dataRangeEnd->setText(d->m_initialState.dataRangeEnd());
        d->m_tabRange->ui->m_dataMajorTick->setText(d->m_initialState.dataMajorTick());
        d->m_tabRange->ui->m_dataMinorTick->setText(d->m_initialState.dataMinorTick());
        d->m_tabRange->ui->m_yLabelsPrecision->setValue(d->m_initialState.yLabelsPrecision());
        d->m_tabRange->ui->m_dataLock->setCurrentIndex((int)d->m_initialState.dataFilter());

        KComboBox* combo = d->m_tabRange->ui->m_comboColumns;
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
        d->m_tabCapitalGain->ui->m_investmentSum->setCurrentIndex(
            d->m_tabCapitalGain->ui->m_investmentSum->findData(static_cast<int>(d->m_initialState.investmentSum())));
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
        d->m_tabPerformance->ui->m_investmentSum->setCurrentIndex(
            d->m_tabPerformance->ui->m_investmentSum->findData(static_cast<int>(d->m_initialState.investmentSum())));
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

// TODO Fix the reports and engine to include transfers even if categories are filtered - bug #1523508
void KReportConfigurationFilterDlg::slotUpdateCheckTransfers()
{
    Q_D(KReportConfigurationFilterDlg);
    d->m_tabRowColPivot->setCheckTransfersEnabled(d->m_tabFilters->categoriesView()->allItemsSelected());
}

void KReportConfigurationFilterDlg::saveState()
{
    const auto config = KSharedConfig::openConfig();
    auto group = config->group("Last Use Settings");
    group.writeEntry("KReportConfigurationFilterDlg", saveGeometry());
    group.sync();
}

void KReportConfigurationFilterDlg::restoreState()
{
    const auto config = KSharedConfig::openConfig();
    const auto group = config->group("Last Use Settings");
    QByteArray geometry = group.readEntry("KReportConfigurationFilterDlg", QByteArray());
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void KReportConfigurationFilterDlg::closeEvent(QCloseEvent* event)
{
    saveState();
    QDialog::closeEvent(event);
}
