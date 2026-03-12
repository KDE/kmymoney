/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Ace Jones <ace.j@hotpop.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KREPORTSVIEW_P_H
#define KREPORTSVIEW_P_H

#include "kreportsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QList>
#include <QLocale>
#include <QMenu>
#include <QMimeData>
#include <QPointer>
#include <QPrintPreviewDialog>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWheelEvent>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KChartAbstractCoordinatePlane>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KMessageBox>
#include <QPainter>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kreportsview.h"
#include "ui_reportcontrol.h"

#include "icons.h"
#include "kmmtextbrowser.h"
#include "kmymoneyutils.h"
#include "kmymoneyviewbase_p.h"
#include "kreportconfigurationfilterdlg.h"
#include "kreporttab.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#include "objectinfotable.h"
#include "pivottable.h"
#include "querytable.h"
#include "reportgroup.h"
#include "reportsviewproxymodel.h"

using namespace reports;
using namespace eMyMoney;
using namespace Icons;

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

class KReportsViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KReportsView)

public:
    explicit KReportsViewPrivate(KReportsView *qq)
        : KMyMoneyViewBasePrivate(qq)
        , m_needLoad(true)
        , m_reportListView(nullptr)
        , m_columnsAlreadyAdjusted(false)
    {
    }

    ~KReportsViewPrivate()
    {
        saveState();
    }

    void setupView(QTreeView* view, ReportsModel* model)
    {
        Q_Q(KReportsView);
        auto proxyModel = new ReportsViewProxyModel(view);
        proxyModel->setSourceModel(model);
        proxyModel->setDynamicSortFilter(true);
        view->setModel(proxyModel);

        view->setAllColumnsShowFocus(true);
        view->setAlternatingRowColors(true);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        view->setUniformRowHeights(true);

        // sorting
        view->setSortingEnabled(true);
        view->sortByColumn(0, Qt::AscendingOrder);
        view->header()->setSectionsClickable(true);
        view->header()->setSortIndicatorShown(true);
        view->header()->setSectionResizeMode(ReportsModel::Columns::ReportName, QHeaderView::ResizeToContents);
        view->header()->setStretchLastSection(true);

        q->connect(view, &QTreeView::doubleClicked, q, &KReportsView::slotDoubleClicked);
        q->connect(view, &QWidget::customContextMenuRequested, q, &KReportsView::slotContextMenu);

        view->installEventFilter(q);
        view->viewport()->installEventFilter(q);
    }

    QModelIndexList selectedSourceIndexes(const QTreeView* view) const
    {
        QModelIndexList viewIndexes = view->selectionModel()->selectedRows();

        QAbstractItemModel* viewModel = view->model();

        if (auto* proxy = qobject_cast<QSortFilterProxyModel*>(viewModel)) {
            QModelIndexList sourceIndexes;
            sourceIndexes.reserve(viewIndexes.size());

            for (const QModelIndex& idx : viewIndexes) {
                QModelIndex src = proxy->mapToSource(idx);
                if (src.isValid())
                    sourceIndexes << src;
            }
            return sourceIndexes;
        }

        return viewIndexes; // already source indexes
    }

    void selectReportInViewById(QTreeView* view, const QString& id)
    {
        if (!view)
            return;

        auto* proxyModel = qobject_cast<QSortFilterProxyModel*>(view->model());
        if (!proxyModel)
            return;

        auto* sourceModel = qobject_cast<ReportsModel*>(proxyModel->sourceModel());
        if (!sourceModel)
            return;

        // 1. Obtain source index
        const QModelIndex sourceIndex = sourceModel->indexById(id);
        if (!sourceIndex.isValid())
            return;

        // 2. Map to proxy index
        const QModelIndex proxyIndex = proxyModel->mapFromSource(sourceIndex);
        if (!proxyIndex.isValid())
            return; // filtered out by proxy

        // 3. Expand all parents
        QModelIndex parent = proxyIndex.parent();
        while (parent.isValid()) {
            view->expand(parent);
            parent = parent.parent();
        }

        // 4. Select + activate
        auto* selectionModel = view->selectionModel();
        if (!selectionModel)
            return;

        selectionModel->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

        view->setCurrentIndex(proxyIndex);
        view->scrollTo(proxyIndex);
    }

    void init()
    {
        Q_Q(KReportsView);
        m_needLoad = false;
        m_needsRefresh = true;

        setColumnsAlreadyAdjusted(false);
        ui.setupUi(q);
        restoreState();

        setupView(ui.m_tocTreeViewCustom, MyMoneyFile::instance()->reportsModel());

        QList<ReportGroup> defaultGroups;
        defaultReports(defaultGroups);
        m_builtInReports = new ReportsModel();
        m_builtInReports->load(defaultGroups);
        setupView(ui.m_tocTreeViewDefault, m_builtInReports);

        ui.m_closeButton->setIcon(Icons::get(Icon::DialogClose));
        ui.m_filterContainer->hide();
        ui.m_searchWidget->installEventFilter(q);

        q->connect(ui.m_reportTabWidget, &QTabWidget::tabCloseRequested, q, &KReportsView::slotClose);

        m_focusWidget = ui.m_tocTreeViewDefault;
    }

    QMap<QString, bool> saveTocExpandState(QTreeView* view) const
    {
        QMap<QString, bool> expandStates;

        auto* model = view->model();
        if (!model)
            return expandStates;

        const int rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const QModelIndex idx = model->index(row, 0);
            const QString label = model->data(idx, Qt::DisplayRole).toString();
            expandStates.insert(label, view->isExpanded(idx));
        }

        return expandStates;
    }

    void restoreTocExpandState(const QMap<QString, bool>& expandStates, QTreeView* view)
    {
        auto* model = view->model();
        if (!model)
            return;

        const int rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const QModelIndex idx = model->index(row, 0);
            const QString label = model->data(idx, Qt::DisplayRole).toString();

            const bool expanded = expandStates.value(label, false);
            view->setExpanded(idx, expanded);
        }
    }

    void saveState()
    {
        if (!m_needLoad) {
            KSharedConfigPtr config = KSharedConfig::openConfig();
            KConfigGroup grp = config->group("Last Use Settings");

            grp.writeEntry("KReportsViewSplitterSize", ui.m_splitter->saveState());
            grp.sync();
        }
    }

    void restoreState()
    {
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group("Last Use Settings");

        QByteArray state = grp.readEntry("KReportsViewSplitterSize", QByteArray());
        if (!state.isEmpty())
            ui.m_splitter->restoreState(state);
        else
            ui.m_splitter->setSizes({300, 700}); // fallback
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

    void addReportTab(const MyMoneyReport& report, KReportsView::OpenOption openOption)
    {
        Q_Q(KReportsView);
        auto reportTab = new KReportTab(ui.m_reportTabWidget, report, q, openOption);
        reportTab->installEventFilter(q);
    }

    void defaultReports(QList<ReportGroup>& groups)
    {
        {
            ReportGroup list("Income and Expenses", i18n("Income and Expenses"));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::ExpenseIncome,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::CurrentMonth,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Income and Expenses This Month"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::ExpenseIncome,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Income and Expenses This Year"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::ExpenseIncome,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Years),
                                         TransactionFilter::Date::All,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Income and Expenses By Year"),
                                         i18n("Default Report")));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::ExpenseIncome,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last12Months,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Income and Expenses Graph"),
                                         i18n("Default Report")));
            list.back().setChartByDefault(true);
            list.back().setChartType(eMyMoney::Report::ChartType::Line);
            list.back().setChartDataLabels(false);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::ExpenseIncome,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last12Months,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Income and Expenses Bar Graph"),
                                         i18n("Default Report")));
            list.back().setChartByDefault(true);
            list.back().setChartType(eMyMoney::Report::ChartType::StackedBar);
            list.back().setChartDataLabels(false);
            list.back().setNegExpenses(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::ExpenseIncome,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::Group,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Income and Expenses Pie Chart"),
                                         i18n("Default Report")));
            list.back().setChartByDefault(true);
            list.back().setChartType(eMyMoney::Report::ChartType::Pie);
            list.back().setShowingRowTotals(false);

            groups.push_back(list);
        }
        {
            ReportGroup list("Net Worth", i18n("Net Worth"));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Net Worth By Month"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Today,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Net Worth Today"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Years),
                                         TransactionFilter::Date::All,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Net Worth By Year"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Next7Days,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("7-day Cash Flow Forecast"),
                                         i18n("Default Report")));
            list.back().setIncludingSchedules(true);
            list.back().setColumnsAreDays(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last12Months,
                                         eMyMoney::Report::DetailLevel::Total,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Net Worth Graph"),
                                         i18n("Default Report")));
            list.back().setChartByDefault(true);
            list.back().setChartCHGridLines(false);
            list.back().setChartSVGridLines(false);
            list.back().setChartType(eMyMoney::Report::ChartType::Line);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Institution,
                                         eMyMoney::Report::QueryColumn::None,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Account Balances by Institution"),
                                         i18n("Default Report")));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AccountType,
                                         eMyMoney::Report::QueryColumn::None,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Account Balances by Type"),
                                         i18n("Default Report")));

            groups.push_back(list);
        }
        {
            ReportGroup list("Transactions", i18n("Transactions"));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Account,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category
                                             | eMyMoney::Report::QueryColumn::Tag | eMyMoney::Report::QueryColumn::Balance,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Transactions by Account"),
                                         i18n("Default Report")));
            // list.back().setConvertCurrency(false);
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Category,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account
                                             | eMyMoney::Report::QueryColumn::Tag,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Transactions by Category"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Payee,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Tag,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Transactions by Payee"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Tag,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Transactions by Tag"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Month,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category
                                             | eMyMoney::Report::QueryColumn::Tag,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Transactions by Month"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Week,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category
                                             | eMyMoney::Report::QueryColumn::Tag,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Transactions by Week"),
                                         i18n("Default Report")));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Account,
                                         eMyMoney::Report::QueryColumn::Loan,
                                         TransactionFilter::Date::All,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Loan Transactions"),
                                         i18n("Default Report")));
            list.back().setLoansOnly(true);
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AccountReconcile,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category
                                             | eMyMoney::Report::QueryColumn::Balance,
                                         TransactionFilter::Date::Last3Months,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Transactions by Reconciliation Status"),
                                         i18n("Default Report")));
            groups.push_back(list);
        }
        {
            ReportGroup list("CashFlow", i18n("Cash Flow"));
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::CashFlow,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account,
                                         TransactionFilter::Date::CurrentMonth,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Cash Flow Transactions This Month"),
                                         i18n("Default Report")));
            groups.push_back(list);
        }
        {
            ReportGroup list("Investments", i18n("Investments"));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::TopAccount,
                                         eMyMoney::Report::QueryColumn::Action | eMyMoney::Report::QueryColumn::Shares | eMyMoney::Report::QueryColumn::Price,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Transactions"),
                                         i18n("Default Report")));
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AccountByTopAccount,
                                         eMyMoney::Report::QueryColumn::Shares | eMyMoney::Report::QueryColumn::Price,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Holdings by Account"),
                                         i18n("Default Report")));
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::EquityType,
                                         eMyMoney::Report::QueryColumn::Shares | eMyMoney::Report::QueryColumn::Price,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Holdings by Type"),
                                         i18n("Default Report")));
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AccountByTopAccount,
                                         eMyMoney::Report::QueryColumn::Performance,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Performance by Account"),
                                         i18n("Default Report")));
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::EquityType,
                                         eMyMoney::Report::QueryColumn::Performance,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Performance by Type"),
                                         i18n("Default Report")));
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AccountByTopAccount,
                                         eMyMoney::Report::QueryColumn::CapitalGain,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Capital Gains by Account"),
                                         i18n("Default Report")));
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::EquityType,
                                         eMyMoney::Report::QueryColumn::CapitalGain,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Capital Gains by Type"),
                                         i18n("Default Report")));
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Today,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Holdings Pie"),
                                         i18n("Default Report")));
            list.back().setChartByDefault(true);
            list.back().setChartCHGridLines(false);
            list.back().setChartSVGridLines(false);
            list.back().setChartType(eMyMoney::Report::ChartType::Pie);
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last12Months,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Worth Graph"),
                                         i18n("Default Report")));
            list.back().setChartByDefault(true);
            list.back().setChartCHGridLines(false);
            list.back().setChartSVGridLines(false);
            list.back().setChartType(eMyMoney::Report::ChartType::Line);
            list.back().setColumnsAreDays(true);
            list.back().setInvestmentsOnly(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last12Months,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Price Graph"),
                                         i18n("Default Report")));
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

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last12Months,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Moving Average Price Graph"),
                                         i18n("Default Report")));
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

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last30Days,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Moving Average"),
                                         i18n("Default Report")));
            list.back().setChartCHGridLines(false);
            list.back().setChartSVGridLines(false);
            list.back().setChartType(eMyMoney::Report::ChartType::Line);
            list.back().setColumnsAreDays(true);
            list.back().setInvestmentsOnly(true);
            list.back().setIncludingBudgetActuals(false);
            list.back().setIncludingMovingAverage(true);
            list.back().setMovingAverageDays(10);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Last30Days,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Investment Moving Average vs Actual"),
                                         i18n("Default Report")));
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

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Category,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account,
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Tax Transactions by Category"),
                                         i18n("Default Report")));
            list.back().setTax(true);
            list.push_back(
                MyMoneyReport(eMyMoney::Report::RowType::Payee,
                              eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Account,
                              TransactionFilter::Date::YearToDate,
                              eMyMoney::Report::DetailLevel::All,
                              eMyMoney::Report::Origin::BuiltIn,
                              i18n("Tax Transactions by Payee"),
                              i18n("Default Report")));
            list.back().setTax(true);
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Category,
                                         eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account,
                                         TransactionFilter::Date::LastFiscalYear,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Tax Transactions by Category Last Fiscal Year"),
                                         i18n("Default Report")));
            list.back().setTax(true);
            list.push_back(
                MyMoneyReport(eMyMoney::Report::RowType::Payee,
                              eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Account,
                              TransactionFilter::Date::LastFiscalYear,
                              eMyMoney::Report::DetailLevel::All,
                              eMyMoney::Report::Origin::BuiltIn,
                              i18n("Tax Transactions by Payee Last Fiscal Year"),
                              i18n("Default Report")));
            list.back().setTax(true);
            groups.push_back(list);
        }
        {
            ReportGroup list("Budgeting", i18n("Budgeting"));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::BudgetActual,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::YearToDate,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Budgeted vs. Actual This Year"),
                                         i18n("Default Report")));
            list.back().setShowingRowTotals(true);
            list.back().setBudget("Any", true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::BudgetActual,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::YearToMonth,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Budgeted vs. Actual This Year (YTM)"),
                                         i18n("Default Report")));
            list.back().setShowingRowTotals(true);
            list.back().setBudget("Any", true);
            // in case we're in January, we show the last year
            if (QDate::currentDate().month() == 1) {
                list.back().setDateFilter(TransactionFilter::Date::LastYear);
            }

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::BudgetActual,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::CurrentMonth,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Monthly Budgeted vs. Actual"),
                                         i18n("Default Report")));
            list.back().setBudget("Any", true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::BudgetActual,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::CurrentFiscalYear,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Yearly Budgeted vs. Actual"),
                                         i18n("Default Report")));
            list.back().setBudget("Any", true);
            list.back().setShowingRowTotals(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Budget,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::CurrentMonth,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Monthly Budget"),
                                         i18n("Default Report")));
            list.back().setBudget("Any", false);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Budget,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::CurrentFiscalYear,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Yearly Budget"),
                                         i18n("Default Report")));
            list.back().setBudget("Any", false);
            list.back().setShowingRowTotals(true);
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::BudgetActual,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::CurrentFiscalYear,
                                         eMyMoney::Report::DetailLevel::Group,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Yearly Budgeted vs Actual Graph"),
                                         i18n("Default Report")));
            list.back().setChartByDefault(true);
            list.back().setChartCHGridLines(false);
            list.back().setChartSVGridLines(false);
            list.back().setBudget("Any", true);
            list.back().setChartType(eMyMoney::Report::ChartType::Line);

            groups.push_back(list);
        }
        {
            ReportGroup list("Forecast", i18n("Forecast"));

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Next12Months,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Forecast By Month"),
                                         i18n("Default Report")));
            list.back().setIncludingForecast(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::NextQuarter,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Forecast Next Quarter"),
                                         i18n("Default Report")));
            list.back().setColumnsAreDays(true);
            list.back().setIncludingForecast(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::ExpenseIncome,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::CurrentYear,
                                         eMyMoney::Report::DetailLevel::Top,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Income and Expenses Forecast This Year"),
                                         i18n("Default Report")));
            list.back().setIncludingForecast(true);

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Next3Months,
                                         eMyMoney::Report::DetailLevel::Total,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Net Worth Forecast Graph"),
                                         i18n("Default Report")));
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

            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Schedule,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Next12Months,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Schedule Information"),
                                         i18n("Default Report")));
            list.back().setDetailLevel(eMyMoney::Report::DetailLevel::All);
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::Schedule,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Next12Months,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Schedule Summary Information"),
                                         i18n("Default Report")));
            list.back().setDetailLevel(eMyMoney::Report::DetailLevel::Top);
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AccountInfo,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Today,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Account Information"),
                                         i18n("Default Report")));
            list.back().setConvertCurrency(false);
            list.push_back(MyMoneyReport(eMyMoney::Report::RowType::AccountLoanInfo,
                                         static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                         TransactionFilter::Date::Today,
                                         eMyMoney::Report::DetailLevel::All,
                                         eMyMoney::Report::Origin::BuiltIn,
                                         i18n("Loan Information"),
                                         i18n("Default Report")));
            list.back().setConvertCurrency(false);
            groups.push_back(list);
        }
        {
            ReportGroup list("Charts", i18n("Charts"));
            for (const auto& group : groups) {
                for (const auto& report : group) {
                    if (report.isChartByDefault()) {
                        list.append(report);
                    }
                }
            }
            groups.push_back(list);
        }

        // In each report setup associated group, which is used in report configuration post steps
        for (auto& group : groups) {
            for (auto& report : group) {
                report.setGroup(group.name());
            }
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

    void setFilter(const QString& text, QTreeView* view)
    {
        auto* proxy = qobject_cast<QSortFilterProxyModel*>(view->model());
        if (text.isEmpty()) {
            proxy->setFilterRegularExpression(QRegularExpression());
            restoreTocExpandState(expandStatesBeforeSearch, view);
            expandStatesBeforeSearch.clear();
        } else {
            if (expandStatesBeforeSearch.isEmpty())
                expandStatesBeforeSearch = saveTocExpandState(view);

            proxy->setFilterRegularExpression(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));

            // Expand visible parents (like your old code)
            for (int row = 0; row < proxy->rowCount(); ++row) {
                view->setExpanded(proxy->index(row, 0), true);
            }
        }
    }

    void setFilter(const QString& text)
    {
        setFilter(text, ui.m_tocTreeViewDefault);
        setFilter(text, ui.m_tocTreeViewCustom);
    }

    // Generate a transaction report that contains transactions for only the
    // currently selected account.
    void showTransactionReport()
    {
        Q_Q(KReportsView);
        if (!m_currentAccount.id().isEmpty()) {
            MyMoneyReport report(eMyMoney::Report::RowType::Account,
                                 eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category,
                                 eMyMoney::TransactionFilter::Date::YearToDate,
                                 eMyMoney::Report::DetailLevel::All,
                                 eMyMoney::Report::Origin::Generated,
                                 i18n("%1 YTD Account Transactions", m_currentAccount.name()),
                                 i18n("Generated Report"));
            report.setGroup(i18n("Transactions"));
            report.addAccount(m_currentAccount.id());
            q->slotOpenReport(report);
        }
    }

    /**
      * This member holds the load state of page
      */
    bool m_needLoad;

    Ui::KReportsView ui;
    QListWidget* m_reportListView;
    QString m_selectedExportFilter;

    bool m_columnsAlreadyAdjusted;
    MyMoneyAccount m_currentAccount;
    QMap<QString, bool> expandStatesBeforeSearch;
    ReportsModel* m_builtInReports;
};

#endif
