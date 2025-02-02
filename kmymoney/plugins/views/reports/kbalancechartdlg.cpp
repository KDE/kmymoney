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
#include <QVBoxLayout>
#include <QWindow>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "kreportchartview.h"
#include "mymoneyenums.h"
#include "mymoneyreport.h"
#include "pivottable.h"

using namespace reports;

class BalanceChartView : public reports::KReportChartView
{
public:
    BalanceChartView(MyMoneyReport* report, const MyMoneyAccount& account, QWidget* parent = nullptr);
};

BalanceChartView::BalanceChartView(MyMoneyReport* report, const MyMoneyAccount& account, QWidget* parent)
    : reports::KReportChartView(parent)
{
    // add another row for limit
    bool needRow = false;
    bool haveMinBalance = false;
    bool haveMaxCredit = false;
    MyMoneyMoney minBalance, maxCredit;
    MyMoneyMoney factor(1, 1);
    if (account.accountGroup() == eMyMoney::Account::Type::Asset)
        factor = -factor;

    if (!account.value("maxCreditEarly").isEmpty()) {
        needRow = true;
        haveMaxCredit = true;
        maxCredit = MyMoneyMoney(account.value("maxCreditEarly")) * factor;
    }
    if (!account.value("maxCreditAbsolute").isEmpty()) {
        needRow = true;
        haveMaxCredit = true;
        maxCredit = MyMoneyMoney(account.value("maxCreditAbsolute")) * factor;
    }

    if (!account.value("minBalanceEarly").isEmpty()) {
        needRow = true;
        haveMinBalance = true;
        minBalance = MyMoneyMoney(account.value("minBalanceEarly"));
    }
    if (!account.value("minBalanceAbsolute").isEmpty()) {
        needRow = true;
        haveMinBalance = true;
        minBalance = MyMoneyMoney(account.value("minBalanceAbsolute"));
    }

    if (needRow) {
        if (haveMinBalance) {
            drawLimitLine(minBalance.toDouble());
        }
        if (haveMaxCredit) {
            drawLimitLine(maxCredit.toDouble());
        }
    }

    // always draw the y axis zero value line
    // TODO: port to KF5 - this crashes KChart
    // drawLimitLine(0);

    // remove the legend if only one graph displayed
    if (report->accounts().size() > 1) {
        removeLegend();
    }
}

KBalanceChartDlg::KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent)
    : QDialog(parent)
    , m_reportCfg(new MyMoneyReport(MyMoneyReport(eMyMoney::Report::RowType::AssetLiability,
                                                  static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                                  eMyMoney::TransactionFilter::Date::Last3ToNext3Months,
                                                  eMyMoney::Report::DetailLevel::Total,
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
        m_reportCfg->setDateFilter(QDate::currentDate().addMonths(-2), QDate::currentDate().addMonths(2));
        m_reportCfg->setDetailLevel(eMyMoney::Report::DetailLevel::Total);
        m_reportCfg->setIncludingForecast(true);
        m_reportCfg->setIncludingBudgetActuals(true);
        m_reportCfg->setInvestmentsOnly(false);
        setWindowTitle(i18n("Balance of %1", account.name()));
    }

    // setup accounts
    if (account.accountType() == eMyMoney::Account::Type::Investment) {
        const auto subAccountList = account.accountList();
        for (const auto& accountID : qAsConst(subAccountList)) {
            m_reportCfg->addAccount(accountID);
        }
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
    m_chartView = new BalanceChartView(m_reportCfg, account);
    mainLayout->addWidget(m_chartView);

    // draw the chart
    reports::PivotTable(*m_reportCfg).drawChart(*m_chartView);

    // add the buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

KBalanceChartDlg::~KBalanceChartDlg()
{
    // store the last used dialog size
    KConfigGroup grp = KSharedConfig::openConfig()->group("KBalanceChartDlg");
    if (grp.isValid()) {
        KWindowConfig::saveWindowSize(windowHandle(), grp);
    }
}
