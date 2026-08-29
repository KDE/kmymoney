/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reporttabcapitalgain.h"
#include "mymoneyenums.h"

#include "ui_reporttabcapitalgain.h"

ReportTabCapitalGain::ReportTabCapitalGain(QWidget* parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabCapitalGain;
    ui->setupUi(this);
    connect(ui->m_investmentSum, &QComboBox::currentIndexChanged, this, &ReportTabCapitalGain::slotInvestmentSumChanged);
}

ReportTabCapitalGain::~ReportTabCapitalGain()
{
    delete ui;
}

bool ReportTabCapitalGain::apply(MyMoneyReport* report)
{
    report->setTermSeparator(ui->m_termSeparator->date());
    report->setShowSTLTCapitalGains(ui->m_showSTLTCapitalGains->isChecked());
    report->setSettlementPeriod(ui->m_settlementPeriod->value());
    report->setShowingColumnTotals(!ui->m_checkHideTotals->isChecked());
    report->setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(ui->m_investmentSum->currentData().toInt()));
    return true;
}

bool ReportTabCapitalGain::load(MyMoneyReport* report)
{
    ui->m_termSeparator->setDate(report->termSeparator());
    ui->m_showSTLTCapitalGains->setChecked(report->isShowingSTLTCapitalGains());
    ui->m_settlementPeriod->setValue(report->settlementPeriod());
    ui->m_checkHideTotals->setChecked(!report->isShowingColumnTotals());
    ui->m_investmentSum->blockSignals(true);
    ui->m_investmentSum->clear();
    ui->m_investmentSum->addItem(i18n("Only owned"), static_cast<int>(eMyMoney::Report::InvestmentSum::Owned));
    ui->m_investmentSum->addItem(i18n("Only sold"), static_cast<int>(eMyMoney::Report::InvestmentSum::Sold));
    ui->m_investmentSum->blockSignals(false);
    ui->m_investmentSum->setCurrentIndex(ui->m_investmentSum->findData(static_cast<int>(report->investmentSum())));
    return true;
}

void ReportTabCapitalGain::slotInvestmentSumChanged(int index)
{
    Q_UNUSED(index);
    if (ui->m_investmentSum->currentData().value<eMyMoney::Report::InvestmentSum>() == eMyMoney::Report::InvestmentSum::Owned) {
        ui->m_settlementPeriod->setValue(0);
        ui->m_settlementPeriod->setEnabled(false);
        ui->m_showSTLTCapitalGains->setChecked(false);
        ui->m_showSTLTCapitalGains->setEnabled(false);
        ui->m_termSeparator->setEnabled(false);
    } else {
        ui->m_settlementPeriod->setEnabled(true);
        ui->m_showSTLTCapitalGains->setEnabled(true);
        ui->m_termSeparator->setEnabled(true);
    }
}
