/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyenums.h"

#include <QHash>

namespace eMyMoney { namespace Report {

eMyMoney::Report::ReportType rowTypeToReportType(eMyMoney::Report::RowType rowType)
{
    // clang-format off
    static const QHash<eMyMoney::Report::RowType, eMyMoney::Report::ReportType> reportTypes{
        {eMyMoney::Report::RowType::NoRows, eMyMoney::Report::ReportType::NoReport},
        {eMyMoney::Report::RowType::AssetLiability, eMyMoney::Report::ReportType::PivotTable},
        {eMyMoney::Report::RowType::ExpenseIncome, eMyMoney::Report::ReportType::PivotTable},
        {eMyMoney::Report::RowType::Category, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::TopCategory, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::Account, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::Tag, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::Payee, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::Month, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::Week, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::TopAccount, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::AccountByTopAccount, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::EquityType, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::AccountType, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::Institution, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::Budget, eMyMoney::Report::ReportType::PivotTable},
        {eMyMoney::Report::RowType::BudgetActual, eMyMoney::Report::ReportType::PivotTable},
        {eMyMoney::Report::RowType::Schedule, eMyMoney::Report::ReportType::InfoTable},
        {eMyMoney::Report::RowType::AccountInfo, eMyMoney::Report::ReportType::InfoTable},
        {eMyMoney::Report::RowType::AccountLoanInfo, eMyMoney::Report::ReportType::InfoTable},
        {eMyMoney::Report::RowType::AccountReconcile, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::CashFlow, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::CapitalGainByTopAccount, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::CapitalGainByType, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::PerformanceByTopAccount, eMyMoney::Report::ReportType::QueryTable},
        {eMyMoney::Report::RowType::PerformanceByType, eMyMoney::Report::ReportType::QueryTable},
    };
    // clang-format on
    return reportTypes.value(rowType, eMyMoney::Report::ReportType::Invalid);
}

unsigned int performanceColumns()
{
    return static_cast<unsigned int>(eMyMoney::Report::QueryColumn::StartingMarketValue | eMyMoney::Report::QueryColumn::Buys
                                     | eMyMoney::Report::QueryColumn::Sells | eMyMoney::Report::QueryColumn::ReinvestIncome
                                     | eMyMoney::Report::QueryColumn::CashIncome | eMyMoney::Report::QueryColumn::EndingMarketValue
                                     | eMyMoney::Report::QueryColumn::Return | eMyMoney::Report::QueryColumn::ReturnInvestment
                                     | eMyMoney::Report::QueryColumn::AnnualizedReturn | eMyMoney::Report::QueryColumn::ExtendedInternalRateOfReturn);
}

unsigned int expandPerformanceColumns(unsigned int qc)
{
    // when the default column set is used, expand to the single columns
    if ((qc & eMyMoney::Report::performanceColumns()) == 0 || qc & eMyMoney::Report::QueryColumn::Performance) {
        qc &= ~eMyMoney::Report::QueryColumn::Performance;
        qc |= eMyMoney::Report::performanceColumns();
    }
    return qc;
}

unsigned int collapsePerformanceColumns(unsigned int qc)
{
    // when the default column set is used, replace them by the default value
    if ((qc & eMyMoney::Report::performanceColumns()) == eMyMoney::Report::performanceColumns()) {
        qc &= ~eMyMoney::Report::performanceColumns();
        qc |= eMyMoney::Report::QueryColumn::Performance;
    }
    return qc;
}

}}
