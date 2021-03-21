/*

    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reportsview.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"
#include "kreportsview.h"
#include "kreportchartview.h"
#include "kmymoneysettings.h"
#include "pivottable.h"
#include "pivotgrid.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyenums.h"
#include "reportsviewenums.h"

#define VIEW_LEDGER         "ledger"

ReportsView::ReportsView(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "reportsview"/*must be the same as X-KDE-PluginInfo-Name*/),
    m_view(nullptr)
{
    Q_UNUSED(args)
    setComponentName("reportsview", i18n("Reports view"));
    // For information, announce that we have been loaded.
    qDebug("Plugins: reportsview loaded");
}

ReportsView::~ReportsView()
{
    qDebug("Plugins: reportsview unloaded");
}

void ReportsView::plug(KXMLGUIFactory* guiFactory)
{
    Q_UNUSED(guiFactory)
    m_view = new KReportsView;
    viewInterface()->addView(m_view, i18n("Reports"), View::Reports, Icons::Icon::Reports);
}

void ReportsView::unplug()
{
    viewInterface()->removeView(View::Reports);
}

QVariant ReportsView::requestData(const QString &arg, uint type)
{
    switch(type) {
    case eWidgetPlugin::WidgetType::NetWorthForecast:
        return QVariant::fromValue(netWorthForecast());
    case eWidgetPlugin::WidgetType::NetWorthForecastWithArgs:
        return QVariant::fromValue(netWorthForecast(arg));
    case eWidgetPlugin::WidgetType::Budget:
        return QVariant(budget());
    default:
        return QVariant();
    }

}

QWidget *ReportsView::netWorthForecast() const
{
    MyMoneyReport reportCfg = MyMoneyReport(
                                  eMyMoney::Report::RowType::AssetLiability,
                                  static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                  eMyMoney::TransactionFilter::Date::UserDefined, // overridden by the setDateFilter() call below
                                  eMyMoney::Report::DetailLevel::Total,
                                  i18n("Net Worth Forecast"),
                                  i18n("Generated Report"));

    reportCfg.setChartByDefault(true);
    reportCfg.setChartCHGridLines(false);
    reportCfg.setChartSVGridLines(false);
    reportCfg.setChartDataLabels(false);
    reportCfg.setChartType(eMyMoney::Report::ChartType::Line);
    reportCfg.setIncludingSchedules(false);
    reportCfg.addAccountGroup(eMyMoney::Account::Type::Asset);
    reportCfg.addAccountGroup(eMyMoney::Account::Type::Liability);
    reportCfg.setColumnsAreDays(true);
    reportCfg.setConvertCurrency(true);
    reportCfg.setIncludingForecast(true);
    reportCfg.setDateFilter(QDate::currentDate(), QDate::currentDate().addDays(+ 90));
    reports::PivotTable table(reportCfg);

    auto chartWidget = new reports::KReportChartView(nullptr);

    table.drawChart(*chartWidget);
    return chartWidget;
}

QWidget *ReportsView::netWorthForecast(const QString &arg) const
{
    const QStringList liArgs = arg.split(';');
    if (liArgs.count() != 4)
        return new QWidget();

    eMyMoney::Report::DetailLevel detailLevel[4] = { eMyMoney::Report::DetailLevel::All, eMyMoney::Report::DetailLevel::Top, eMyMoney::Report::DetailLevel::Group, eMyMoney::Report::DetailLevel::Total, };

    MyMoneyReport reportCfg = MyMoneyReport(
                                  eMyMoney::Report::RowType::AssetLiability,
                                  static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                  eMyMoney::TransactionFilter::Date::UserDefined, // overridden by the setDateFilter() call below
                                  detailLevel[liArgs.at(0).toInt()],
                                  i18n("Net Worth Forecast"),
                                  i18n("Generated Report"));

    reportCfg.setChartByDefault(true);
    reportCfg.setChartCHGridLines(false);
    reportCfg.setChartSVGridLines(false);
    reportCfg.setChartType(eMyMoney::Report::ChartType::Line);
    reportCfg.setIncludingSchedules(false);
    reportCfg.setColumnsAreDays( true );
    reportCfg.setChartDataLabels(false);
    reportCfg.setConvertCurrency(true);
    reportCfg.setIncludingForecast(true);
    reportCfg.setDateFilter(QDate::currentDate(), QDate::currentDate().addDays(liArgs.at(2).toLongLong()));
    reports::PivotTable table(reportCfg);

    auto forecastChart = new reports::KReportChartView(nullptr);
    forecastChart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table.drawChart(*forecastChart);

    // Adjust the size
    forecastChart->resize(liArgs.at(2).toInt() - 10, liArgs.at(3).toInt());
    forecastChart->show();
    forecastChart->update();
    return forecastChart;
}

QString ReportsView::budget() const
{
    const auto file = MyMoneyFile::instance();

    QString html;
    //div header
    html += "<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + i18n("Budget") + "</div>\n<div class=\"gap\">&nbsp;</div>\n";

    if (file->countBudgets() == 0) {
        html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
        html += QString("<tr>");
        html += QString("<td><center>%1</center></td>").arg(i18n("You have no budgets to display."));
        html += QString("</tr>");
        html += "</table></div></div>";
        return html;
    }

    auto prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
    bool isOverrun = false;
    int i = 0;

    //config report just like "Monthly Budgeted vs Actual
    MyMoneyReport reportCfg = MyMoneyReport(
                                  eMyMoney::Report::RowType::BudgetActual,
                                  static_cast<unsigned>(eMyMoney::Report::ColumnType::Months),
                                  eMyMoney::TransactionFilter::Date::CurrentMonth,
                                  eMyMoney::Report::DetailLevel::All,
                                  i18n("Monthly Budgeted vs. Actual"),
                                  i18n("Generated Report"));

    reportCfg.setBudget("Any", true);

    reports::PivotTable table(reportCfg);

    reports::PivotGrid grid = table.grid();

    //display budget summary
    html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
    html += "<tr class=\"itemtitle\">";
    html += "<td class=\"left\" colspan=\"3\">";
    html += i18n("Current Month Summary");
    html += "</td></tr>";
    html += "<tr class=\"item\">";
    html += "<td class=\"right\" width=\"50%\">";
    html += i18n("Budgeted");
    html += "</td>";
    html += "<td class=\"right\" width=\"20%\">";
    html += i18n("Actual");
    html += "</td>";
    html += "<td class=\"right\" width=\"20%\">";
    html += i18n("Difference");
    html += "</td></tr>";

    html += QString("<tr class=\"row-odd\">");

    MyMoneyMoney totalBudgetValue = grid.m_total[reports::eBudget].m_total;
    MyMoneyMoney totalActualValue = grid.m_total[reports::eActual].m_total;
    MyMoneyMoney totalBudgetDiffValue = grid.m_total[reports::eBudgetDiff].m_total;

    QString totalBudgetAmount = totalBudgetValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString totalActualAmount = totalActualValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString totalBudgetDiffAmount = totalBudgetDiffValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);

    html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalBudgetAmount, totalBudgetValue.isNegative()));
    html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalActualAmount, totalActualValue.isNegative()));
    html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalBudgetDiffAmount, totalBudgetDiffValue.isNegative()));
    html += "</tr>";
    html += "</table>";

    //budget overrun
    html += "<div class=\"gap\">&nbsp;</div>\n";
    html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
    html += "<tr class=\"itemtitle\">";
    html += "<td class=\"left\" colspan=\"4\">";
    html += i18n("Budget Overruns");
    html += "</td></tr>";
    html += "<tr class=\"item\">";
    html += "<td class=\"left\" width=\"30%\">";
    html += i18n("Account");
    html += "</td>";
    html += "<td class=\"right\" width=\"20%\">";
    html += i18n("Budgeted");
    html += "</td>";
    html += "<td class=\"right\" width=\"20%\">";
    html += i18n("Actual");
    html += "</td>";
    html += "<td class=\"right\" width=\"20%\">";
    html += i18n("Difference");
    html += "</td></tr>";

    reports::PivotGrid::iterator it_outergroup = grid.begin();
    const int column = 0;
    while (it_outergroup != grid.end()) {
        i = 0;
        reports::PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
        while (it_innergroup != (*it_outergroup).end()) {
            reports::PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
            while (it_row != (*it_innergroup).end()) {
                //column number is 1 because the report includes only current month
                if (it_row.value()[reports::eBudgetDiff].value(column).isNegative()) {
                    //get report account to get the name later
                    reports::ReportAccount rowname = it_row.key();

                    //write the outergroup if it is the first row of outergroup being shown
                    if (i == 0) {
                        html += "<tr style=\"font-weight:bold;\">";
                        html += QString("<td class=\"left\" colspan=\"4\">%1</td>").arg(MyMoneyAccount::accountTypeToString(rowname.accountType()));
                        html += "</tr>";
                    }
                    html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");

                    //get values from grid
                    MyMoneyMoney actualValue = it_row.value()[reports::eActual][column];
                    MyMoneyMoney budgetValue = it_row.value()[reports::eBudget][column];
                    MyMoneyMoney budgetDiffValue = it_row.value()[reports::eBudgetDiff][column];

                    //format amounts
                    QString actualAmount = actualValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
                    QString budgetAmount = budgetValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
                    QString budgetDiffAmount = budgetDiffValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);

                    //account name
                    html += QString("<td>") + link(VIEW_LEDGER, QString("?id=%1").arg(rowname.id()), QString()) + rowname.name() + linkend() + "</td>";

                    //show amounts
                    html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetAmount, budgetValue.isNegative()));
                    html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(actualAmount, actualValue.isNegative()));
                    html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetDiffAmount, budgetDiffValue.isNegative()));
                    html += "</tr>";

                    //set the flag that there are overruns
                    isOverrun = true;
                }
                ++it_row;
            }
            ++it_innergroup;
        }
        ++it_outergroup;
    }

    //if no negative differences are found, then inform that
    if (isOverrun == false) {
        html += QString::fromLatin1("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(((i++ & 1) == 1) ? QLatin1String("even") : QLatin1String("odd"));
        html += QString::fromLatin1("<td class=\"center\" colspan=\"4\">%1</td>").arg(i18n("No Budget Categories have been overrun"));
        html += "</tr>";
    }
    html += "</table></div></div>";
    return html;
}

QString ReportsView::showColoredAmount(const QString &amount, bool isNegative) const
{
    if (isNegative) {
        //if negative, get the settings for negative numbers
        return QString("<font color=\"%1\">%2</font>").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name(), amount);
    }

    //if positive, return the same string
    return amount;
}

QString ReportsView::link(const QString& view, const QString& query, const QString& _title) const
{
    QString titlePart;
    QString title(_title);
    if (!title.isEmpty())
        titlePart = QString(" title=\"%1\"").arg(title.replace(QLatin1Char(' '), "&nbsp;"));

    return QString("<a href=\"/%1%2\"%3>").arg(view, query, titlePart);
}

QString ReportsView::linkend() const
{
    return QStringLiteral("</a>");
}

K_PLUGIN_FACTORY_WITH_JSON(ReportsViewFactory, "reportsview.json", registerPlugin<ReportsView>();)

#include "reportsview.moc"
