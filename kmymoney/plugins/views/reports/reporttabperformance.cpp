/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reporttabperformance.h"
#include "mymoneyenums.h"

#include "ui_reporttabperformance.h"

ReportTabPerformance::ReportTabPerformance(QWidget* parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabPerformance;
    ui->setupUi(this);
    ui->m_investmentSum->addItem(i18n("From period"), static_cast<int>(eMyMoney::Report::InvestmentSum::Period));
    ui->m_investmentSum->addItem(i18n("Owned and sold"), static_cast<int>(eMyMoney::Report::InvestmentSum::OwnedAndSold));
    ui->m_investmentSum->addItem(i18n("Only owned"), static_cast<int>(eMyMoney::Report::InvestmentSum::Owned));
    ui->m_investmentSum->addItem(i18n("Only sold"), static_cast<int>(eMyMoney::Report::InvestmentSum::Sold));
    connect(ui->m_investmentSum, &QComboBox::currentIndexChanged, this, [this]() {
        Q_EMIT investmentSumChanged(static_cast<eMyMoney::Report::InvestmentSum>(ui->m_investmentSum->currentData().toInt()));
    });
}

ReportTabPerformance::~ReportTabPerformance()
{
    delete ui;
}

bool ReportTabPerformance::apply(MyMoneyReport* report)
{
    report->setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(ui->m_investmentSum->currentData().toInt()));
    report->setShowingColumnTotals(!ui->m_checkHideTotals->isChecked());
    return true;
}

bool ReportTabPerformance::load(MyMoneyReport* report)
{
    ui->m_checkHideTotals->setChecked(!report->isShowingColumnTotals());
    ui->m_investmentSum->setCurrentIndex(ui->m_investmentSum->findData(static_cast<int>(report->investmentSum())));
    return true;
}
