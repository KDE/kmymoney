/*
    SPDX-FileCopyrightText: 2007-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kbalancechartdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialogButtonBox>
#include <QLocale>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KChartCartesianCoordinatePlane.h>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KXmlGuiWindow>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "kmymoneyutils.h"
#include "kreportchartview.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#include "mymoneyutils.h"
#include "pivottable.h"
#include "reporttabimpl.h"

using namespace reports;

class BalanceChartView : public reports::KReportChartView
{
public:
    BalanceChartView(const MyMoneyAccount& account, bool showLegend, QWidget* parent = nullptr);
    void addMarker();

    MyMoneyAccount m_account;
};

BalanceChartView::BalanceChartView(const MyMoneyAccount& account, bool showLegend, QWidget* parent)
    : reports::KReportChartView(parent)
    , m_account(account)
{
    if (!showLegend) {
        removeLegend();
    }
}

void BalanceChartView::addMarker()
{
    // prevent crashing if called too early
    if (!coordinatePlane()->diagram()) {
        return;
    }

    // add another row for limit
    bool haveMinBalance = false;
    bool haveMaxCredit = false;
    MyMoneyMoney minBalance, maxCredit;
    MyMoneyMoney factor(1, 1);
    if (m_account.accountGroup() == eMyMoney::Account::Type::Asset)
        factor = -factor;

    if (!m_account.value("maxCreditEarly").isEmpty()) {
        haveMaxCredit = true;
        maxCredit = MyMoneyMoney(m_account.value("maxCreditEarly")) * -factor;
    }
    if (!m_account.value("maxCreditAbsolute").isEmpty()) {
        haveMaxCredit = true;
        maxCredit = MyMoneyMoney(m_account.value("maxCreditAbsolute")) * -factor;
    }

    if (!m_account.value("minBalanceEarly").isEmpty()) {
        haveMinBalance = true;
        minBalance = MyMoneyMoney(m_account.value("minBalanceEarly"));
    }
    if (!m_account.value("minBalanceAbsolute").isEmpty()) {
        haveMinBalance = true;
        minBalance = MyMoneyMoney(m_account.value("minBalanceAbsolute"));
    }

    bool paintZeroLine(true);
    if (haveMinBalance) {
        paintZeroLine &= !minBalance.isZero();
        drawLimitLine(minBalance.toDouble());
    }
    if (haveMaxCredit) {
        paintZeroLine &= !maxCredit.isZero();
        drawLimitLine(maxCredit.toDouble());
    }

    // draw the zero value line if needed
    KChart::CartesianCoordinatePlane* cartesianPlane = qobject_cast<CartesianCoordinatePlane*>(coordinatePlane());
    if (cartesianPlane) {
        auto verticalRange = cartesianPlane->verticalRange();

        // check if we have a horizontal data line (see KReportChartView::adjustVerticalRange())
        // and revert the adjustment performed
        if ((verticalRange.second - verticalRange.first) == 4) {
            verticalRange.first += 2;
            verticalRange.second -= 2;
        }

        auto adjustToLimit = [&](const double value) {
            if (verticalRange.first > value) {
                verticalRange.first = value;
            }
            if (verticalRange.second < value) {
                verticalRange.second = value;
            }
        };
        if (haveMinBalance) {
            adjustToLimit(minBalance.toDouble());
        }
        if (haveMaxCredit) {
            adjustToLimit(maxCredit.toDouble());
        }

        // check if we still have a horizontal data line
        // if so, we add the adjustment again (see KReportChartView::adjustVerticalRange())
        if (verticalRange.second == verticalRange.first) {
            verticalRange.first -= 2;
            verticalRange.second += 2;
        }
        cartesianPlane->setVerticalRange(verticalRange);

        // check if vertical range does not cross the abscissa and reset the flag
        if ((verticalRange.first >= 0.0) || (verticalRange.second <= 0.0)) {
            paintZeroLine = false;
        }
    }

    if (paintZeroLine) {
        drawLimitLine(0);
    }
}

KBalanceChartDlg::KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent)
    : QDialog(parent)
    , m_reportCfg(new MyMoneyReport(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                                  static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                                  eMyMoney::TransactionFilter::Date::Last3ToNext3Months,
                                                  eMyMoney::Report::DetailLevel::Total,
                                                  eMyMoney::Report::Origin::Generated,
                                                  QString(),
                                                  QString())))
{
    // setup report
    QLocale locale;
    m_reportCfg->setDataRangeEnd(locale.toString(1.1));
    m_reportCfg->setDataMajorTick(locale.toString(0.1));
    m_reportCfg->setDataMinorTick(locale.toString(0.02));

    m_reportCfg->setChartByDefault(true);
    m_reportCfg->setChartCHGridLines(false);
    m_reportCfg->setChartSVGridLines(false);
    m_reportCfg->setChartDataLabels(false);
    m_reportCfg->setChartType(eMyMoney::Report::ChartType::Line);
    m_reportCfg->setChartPalette(eMyMoney::Report::ChartPalette::Application);
    m_reportCfg->setColumnsAreDays(true);
    m_reportCfg->setConvertCurrency(false);
    m_reportCfg->setMixedTime(true);
    m_reportCfg->setNegExpenses(MyMoneyAccount::balanceFactor(account.accountType()).isNegative());

    bool showLegend(false);
    if (account.accountType() == eMyMoney::Account::Type::Investment || account.accountType() == eMyMoney::Account::Type::Stock) {
        m_reportCfg->setName(i18nc("@title:window Value chart for investments", "%1 Value History", account.name()));
        m_reportCfg->setDateFilter(eMyMoney::TransactionFilter::Date::Last6Months);
        m_reportCfg->setDetailLevel(eMyMoney::Report::DetailLevel::All);
        m_reportCfg->setIncludingForecast(false);
        m_reportCfg->setIncludingBudgetActuals(true);
        m_reportCfg->setInvestmentsOnly(true);
        setWindowTitle(i18n("Value of %1", account.name()));

    } else {
        m_reportCfg->setName(i18nc("@title:window Balance chart for account", "%1 Balance History", account.name()));
        m_reportCfg->setDetailLevel(eMyMoney::Report::DetailLevel::Total);
        m_reportCfg->setIncludingForecast(true);
        m_reportCfg->setIncludingBudgetActuals(true);
        m_reportCfg->setInvestmentsOnly(false);
        m_reportCfg->setDataRangeEnd(QLatin1String("0"));
        m_reportCfg->setDataMajorTick(QLatin1String("0"));
        m_reportCfg->setDataMinorTick(QLatin1String("0"));
        setWindowTitle(i18n("Balance of %1", account.name()));
    }

    // setup accounts
    if (account.accountType() == eMyMoney::Account::Type::Investment) {
        const auto subAccountList = account.accountList();
        for (const auto& accountID : qAsConst(subAccountList)) {
            m_reportCfg->addAccount(accountID);
        }
        // show the legend when more than one graph is displayed
        showLegend = (subAccountList.count() > 1);

    } else {
        m_reportCfg->addAccount(account.id());
    }

    setSizeGripEnabled(true);
    setModal(true);

    // restore the last used dialog size
    winId(); // needs to be called to create the QWindow
    KConfigGroup grp = KSharedConfig::openConfig()->group("KBalanceChartDlg");
    if (grp.isValid()) {
        KWindowConfig::restoreWindowSize(windowHandle(), grp);
    }
    // let the minimum size be 700x500
    resize(QSize(700, 500).expandedTo(windowHandle() ? windowHandle()->size() : QSize()));

    QVBoxLayout* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    // add chart to the main layout
    m_chartView = new BalanceChartView(account, showLegend);
    mainLayout->addWidget(m_chartView);

    // draw the chart
    reports::PivotTable(*m_reportCfg, KMyMoneyUtils::forecastConfig()).drawChart(*m_chartView);

    // add the min/max marker lines but only, if we don't show
    // a legend because the markers create too many entries in
    // the legend. The markers usually only make sense in regular
    // accounts (which don't show a legend) and not in investment
    // accounts
    if (!showLegend) {
        m_chartView->addMarker();
    }

    // add the buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Reset | QDialogButtonBox::Retry);
    buttonBox->button(QDialogButtonBox::Reset)->setText(i18n("Configure report"));
    buttonBox->button(QDialogButtonBox::Reset)->setIcon(Icons::get(Icons::Icon::DocumentProperties));
    buttonBox->button(QDialogButtonBox::Retry)->setText(i18n("New report"));
    buttonBox->button(QDialogButtonBox::Retry)->setIcon(Icons::get(Icons::Icon::DocumentNew));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Reset), &QPushButton::pressed, this, &KBalanceChartDlg::configureReport);
    connect(buttonBox->button(QDialogButtonBox::Retry), &QPushButton::pressed, this, &KBalanceChartDlg::newReport);
    mainLayout->addWidget(buttonBox);

    if (!showLegend) {
        m_chartView->removeLegend();
    }
}

KBalanceChartDlg::~KBalanceChartDlg()
{
    // store the last used dialog size
    KConfigGroup grp = KSharedConfig::openConfig()->group("KBalanceChartDlg");
    if (grp.isValid()) {
        KWindowConfig::saveWindowSize(windowHandle(), grp);
    }
}

void KBalanceChartDlg::configureReport()
{
    QDialog dialog;
    QVBoxLayout* layout = new QVBoxLayout;
    dialog.setLayout(layout);
    ReportTabChart* chartWidget = new ReportTabChart(&dialog);
    chartWidget->setPlotExpensesDownwardVisible(false);
    chartWidget->removeChartType(eMyMoney::Report::ChartType::StackedBar);
    chartWidget->removeChartType(eMyMoney::Report::ChartType::Ring);
    layout->addWidget(chartWidget);
    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    layout->addWidget(box);
    box->addButton(box->button(QDialogButtonBox::Ok), QDialogButtonBox::AcceptRole);
    connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    chartWidget->load(m_reportCfg);
    if (dialog.exec() == QDialog::Accepted) {
        chartWidget->apply(m_reportCfg);
        reports::PivotTable(*m_reportCfg).drawChart(*m_chartView);
    }
}

void KBalanceChartDlg::newReport()
{
    MyMoneyReport& newReport = *m_reportCfg;
    newReport.setGroup("Value/Balance History");
    QDate today = QDate::currentDate();
    newReport.setName(newReport.name() + QString(" (%1)").arg(MyMoneyUtils::formatDate(today)));
    newReport.setDateFilter(eMyMoney::TransactionFilter::Date::UserDefined);
    newReport.setEvaluationDate(today);
    newReport.setIsGenerated();

    MyMoneyFileTransaction ft;
    try {
        if (!newReport.id().isEmpty()) {
            MyMoneyFile::instance()->modifyReport(newReport);
            ft.commit();
        } else {
            MyMoneyFile::instance()->addReport(newReport);
            ft.commit();
        }

        // open report
        KXmlGuiWindow* mw = KMyMoneyUtils::mainWindow();
        QAction* action = mw->actionCollection()->action("report_open");
        MyMoneyUtils::triggerAction(action);
        Q_EMIT openReport(newReport.id());
    } catch (const MyMoneyException&) {
    }
}
