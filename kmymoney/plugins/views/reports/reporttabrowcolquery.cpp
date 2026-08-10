/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reporttabrowcolquery.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"

#include <ui_reporttabrowcolquery.h>

ReportTabRowColQuery::ReportTabRowColQuery(QWidget* parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabRowColQuery;
    ui->setupUi(this);
    ui->buttonGroup1->setExclusive(false);
    ui->buttonGroup1->setId(ui->m_checkMemo, 0);
    ui->buttonGroup1->setId(ui->m_checkShares, 1);
    ui->buttonGroup1->setId(ui->m_checkPrice, 2);
    ui->buttonGroup1->setId(ui->m_checkReconciled, 3);
    ui->buttonGroup1->setId(ui->m_checkAccount, 4);
    ui->buttonGroup1->setId(ui->m_checkNumber, 5);
    ui->buttonGroup1->setId(ui->m_checkPayee, 6);
    ui->buttonGroup1->setId(ui->m_checkCategory, 7);
    ui->buttonGroup1->setId(ui->m_checkAction, 8);
    ui->buttonGroup1->setId(ui->m_checkBalance, 9);
    connect(ui->m_checkHideTransactions, &QAbstractButton::toggled, this, &ReportTabRowColQuery::slotHideTransactionsChanged);
}

void ReportTabRowColQuery::slotHideTransactionsChanged(bool checked)
{
    if (checked) // toggle m_checkHideSplitDetails only if it's mandatory
        ui->m_checkHideSplitDetails->setChecked(checked);
    ui->m_checkHideSplitDetails->setEnabled(!checked); // hiding transactions without hiding splits isn't allowed
}

ReportTabRowColQuery::~ReportTabRowColQuery()
{
    delete ui;
}

void ReportTabRowColQuery::applyConvertCurrencyChanged(MyMoneyReport* report, int state)
{
    // TODO: Are these settings really necessary here, or is it enough to use them in apply() ?
    report->setTax(ui->m_checkTax->isChecked());
    report->setInvestmentsOnly(ui->m_checkInvestments->isChecked());
    report->setLoansOnly(ui->m_checkLoans->isChecked());

    QCheckBox* box = nullptr;

    bool condition = false;
    if (report->isInvestmentsOnly()) {
        condition = report->isConvertCurrency() && report->hasConversionRates();
        box = ui->m_checkPrice;
    } else {
        condition = report->isConvertCurrency() && report->hasConversionRates() && report->hasMultipleCurrencies();
        box = ui->m_checkRate;
    }

    // The rate column is mandatory when currency conversion is enabled,
    // see https://bugs.kde.org/show_bug.cgi?id=345550

    // Previous state is saved into the tristate flag
    if (state) {
        box->setTristate(box->checkState());
        if (condition) {
            box->setChecked(true);
            box->setEnabled(false);
        }
    } else {
        box->setChecked(box->isTristate());
        box->setTristate(false);
        box->setEnabled(true);
    }
}

bool ReportTabRowColQuery::apply(MyMoneyReport* reportState)
{
    // from slotSearch()
    eMyMoney::Report::RowType rtq[8] = {
        eMyMoney::Report::RowType::Category,
        eMyMoney::Report::RowType::TopCategory,
        eMyMoney::Report::RowType::Tag,
        eMyMoney::Report::RowType::Payee,
        eMyMoney::Report::RowType::Account,
        eMyMoney::Report::RowType::TopAccount,
        eMyMoney::Report::RowType::Month,
        eMyMoney::Report::RowType::Week,
    };
    reportState->setRowType(rtq[ui->m_comboOrganizeBy->currentIndex()]);

    unsigned qc = eMyMoney::Report::QueryColumn::None;

    if (reportState->queryColumns() & eMyMoney::Report::QueryColumn::Loan)
        // once a loan report, always a loan report
        qc = eMyMoney::Report::QueryColumn::Loan;

    if (ui->m_checkNumber->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Number;
    if (ui->m_checkPayee->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Payee;
    if (ui->m_checkTag->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Tag;
    if (ui->m_checkCategory->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Category;
    if (ui->m_checkMemo->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Memo;
    if (ui->m_checkAccount->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Account;
    if (ui->m_checkReconciled->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Reconciled;
    if (ui->m_checkAction->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Action;
    if (ui->m_checkShares->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Shares;
    if (ui->m_checkPrice->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Price;
    if (ui->m_checkRate->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Rate;
    if (ui->m_checkBalance->isChecked())
        qc |= eMyMoney::Report::QueryColumn::Balance;

    reportState->setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(qc));

    reportState->setTax(ui->m_checkTax->isChecked());
    reportState->setInvestmentsOnly(ui->m_checkInvestments->isChecked());
    reportState->setLoansOnly(ui->m_checkLoans->isChecked());

    reportState->setDetailLevel(ui->m_checkHideSplitDetails->isChecked() ? eMyMoney::Report::DetailLevel::None : eMyMoney::Report::DetailLevel::All);
    reportState->setHideTransactions(ui->m_checkHideTransactions->isChecked());
    reportState->setShowingColumnTotals(!ui->m_checkHideTotals->isChecked());

    reportState->setIncludingTransfers(ui->m_checkTransfers->isChecked());

    return true;
}

bool ReportTabRowColQuery::load(MyMoneyReport* report)
{
    KComboBox* combo = ui->m_comboOrganizeBy;
    switch (report->rowType()) {
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

    unsigned qc = report->queryColumns();
    ui->m_checkNumber->setChecked(qc & eMyMoney::Report::QueryColumn::Number);
    ui->m_checkPayee->setChecked(qc & eMyMoney::Report::QueryColumn::Payee);
    ui->m_checkTag->setChecked(qc & eMyMoney::Report::QueryColumn::Tag);
    ui->m_checkCategory->setChecked(qc & eMyMoney::Report::QueryColumn::Category);
    ui->m_checkMemo->setChecked(qc & eMyMoney::Report::QueryColumn::Memo);
    ui->m_checkAccount->setChecked(qc & eMyMoney::Report::QueryColumn::Account);
    ui->m_checkReconciled->setChecked(qc & eMyMoney::Report::QueryColumn::Reconciled);
    ui->m_checkAction->setChecked(qc & eMyMoney::Report::QueryColumn::Action);
    ui->m_checkShares->setChecked(qc & eMyMoney::Report::QueryColumn::Shares);
    ui->m_checkPrice->setChecked(qc & eMyMoney::Report::QueryColumn::Price);
    ui->m_checkRate->setChecked(qc & eMyMoney::Report::QueryColumn::Rate);
    ui->m_checkBalance->setChecked(qc & eMyMoney::Report::QueryColumn::Balance);

    ui->m_checkTax->setChecked(report->isTax());
    ui->m_checkInvestments->setChecked(report->isInvestmentsOnly());
    ui->m_checkLoans->setChecked(report->isLoansOnly());

    ui->m_checkHideTransactions->setChecked(report->isHideTransactions());
    ui->m_checkHideTotals->setChecked(!report->isShowingColumnTotals());
    ui->m_checkHideSplitDetails->setEnabled(!report->isHideTransactions());

    ui->m_checkHideSplitDetails->setChecked(report->detailLevel() == eMyMoney::Report::DetailLevel::None || report->isHideTransactions());
    ui->m_checkTransfers->setChecked(report->isIncludingTransfers());

    return true;
}
