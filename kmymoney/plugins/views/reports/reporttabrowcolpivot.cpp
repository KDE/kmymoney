/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reporttabrowcolpivot.h"
#include "mymoneybudget.h"
#include "mymoneyenums.h"

#include "ui_reporttabrowcolpivot.h"
#include <qdatetime.h>

ReportTabRowColPivot::ReportTabRowColPivot(QWidget* parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabRowColPivot;
    ui->setupUi(this);
    connect(ui->m_propagateRemainder, &QCheckBox::stateChanged, this, [&](int _state) {
        const auto state = static_cast<Qt::CheckState>(_state);
        ui->m_checkTotalColumn->setDisabled(state == Qt::Checked);
        switch (state) {
        case Qt::Checked:
            ui->m_checkTotalColumn->setChecked(false);
            break;
        default:
            break;
        }
    });

    connect(ui->m_comboRows, &QComboBox::activated, this, [&](int index) {
        ui->m_checkTotalColumn->setEnabled(index == 0);
        Q_EMIT comboRowsActivated(index);
    });
}

ReportTabRowColPivot::~ReportTabRowColPivot()
{
    delete ui;
}

bool ReportTabRowColPivot::apply(MyMoneyReport* report, bool budgetActual, const QVector<MyMoneyBudget>& budgets)
{
    eMyMoney::Report::DetailLevel dl[4] = {eMyMoney::Report::DetailLevel::All,
                                           eMyMoney::Report::DetailLevel::Top,
                                           eMyMoney::Report::DetailLevel::Group,
                                           eMyMoney::Report::DetailLevel::Total};

    report->setDetailLevel(dl[ui->m_comboDetail->currentIndex()]);

    // modify the rowtype only if the widget is enabled
    if (ui->m_comboRows->isEnabled()) {
        eMyMoney::Report::RowType rt[2] = {eMyMoney::Report::RowType::ExpenseIncome, eMyMoney::Report::RowType::AssetLiability};
        report->setRowType(rt[ui->m_comboRows->currentIndex()]);
    }

    report->setShowingRowTotals(false);
    if (ui->m_comboRows->currentIndex() == 0)
        report->setShowingRowTotals(ui->m_checkTotalColumn->isChecked());

    report->setShowingColumnTotals(ui->m_checkTotalRow->isChecked());
    report->setIncludingSchedules(ui->m_checkScheduled->isChecked());
    report->setPropagateBudgetDifference(ui->m_propagateRemainder->isChecked());
    report->setIncludingTransfers(ui->m_checkTransfers->isChecked());

    report->setIncludingUnusedAccounts(ui->m_checkUnused->isChecked());

    if (ui->m_comboBudget->isEnabled() && (budgets.count() > 0)) {
        report->setBudget(budgets[ui->m_comboBudget->currentItem()].id(), budgetActual);
    } else {
        report->setBudget(QString(), false);
    }

    // set moving average days
    if (ui->m_movingAverageDays->isEnabled()) {
        report->setMovingAverageDays(ui->m_movingAverageDays->value());
    }
    return true;
}

bool ReportTabRowColPivot::load(MyMoneyReport* report, const QVector<MyMoneyBudget>& budgets)
{
    KComboBox* combo = ui->m_comboDetail;
    switch (report->detailLevel()) {
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

    combo = ui->m_comboRows;
    switch (report->rowType()) {
    case eMyMoney::Report::RowType::ExpenseIncome:
    case eMyMoney::Report::RowType::Budget:
    case eMyMoney::Report::RowType::BudgetActual:
        combo->setCurrentItem(i18n("Income & Expenses"), false); // income / expense
        break;
    default:
        combo->setCurrentItem(i18n("Assets & Liabilities"), false); // asset / liability
        break;
    }
    ui->m_checkTotalColumn->setChecked(report->isShowingRowTotals());
    ui->m_checkTotalRow->setChecked(report->isShowingColumnTotals());
    ui->m_propagateRemainder->setEnabled(report->rowType() == eMyMoney::Report::RowType::BudgetActual);
    ui->m_propagateRemainder->setChecked(report->isPropagateBudgetDifference());
    ui->m_checkTotalRow->setDisabled(report->isPropagateBudgetDifference());

    setTotalColumnEnabled(combo->currentIndex() == 0);

    // load budgets combo
    ui->m_comboBudget->setDisabled(true);
    if (report->rowType() == eMyMoney::Report::RowType::Budget || report->rowType() == eMyMoney::Report::RowType::BudgetActual) {
        ui->m_comboBudget->setEnabled(true);
        ui->m_comboRows->setEnabled(false);
        ui->m_rowsLabel->setEnabled(false);
        ui->m_budgetFrame->setEnabled(!budgets.empty());
        auto i = 0;
        for (QVector<MyMoneyBudget>::const_iterator it_b = budgets.cbegin(); it_b != budgets.cend(); ++it_b) {
            ui->m_comboBudget->insertItem((*it_b).name(), i);
            // set the current selected item
            if ((report->budget() == "Any" && (*it_b).budgetStart().year() == QDate::currentDate().year()) || report->budget() == (*it_b).id())
                ui->m_comboBudget->setCurrentItem(i);
            i++;
        }
    }

    // set moving average days spinbox
    QSpinBox* spinbox = ui->m_movingAverageDays;
    spinbox->setEnabled(report->isIncludingMovingAverage());
    ui->m_movingAverageLabel->setEnabled(report->isIncludingMovingAverage());

    if (report->isIncludingMovingAverage()) {
        spinbox->setValue(report->movingAverageDays());
    }

    ui->m_checkScheduled->setChecked(report->isIncludingSchedules());
    ui->m_checkTransfers->setChecked(report->isIncludingTransfers());
    ui->m_checkUnused->setChecked(report->isIncludingUnusedAccounts());

    return true;
}

bool ReportTabRowColPivot::comboBudgetEnabled()
{
    return ui->m_comboBudget->isEnabled();
}

bool ReportTabRowColPivot::comboRowsIsIncomeExpense()
{
    return ui->m_comboRows->currentIndex() == 0;
}

void ReportTabRowColPivot::setCheckTransfersEnabled(bool state)
{
    auto cb = ui->m_checkTransfers;
    if (state) {
        cb->setChecked(false);
        cb->setDisabled(true);
    } else {
        cb->setEnabled(true);
    }
}

void ReportTabRowColPivot::setTotalColumnEnabled(bool state)
{
    ui->m_checkTotalColumn->setEnabled(state);
}
