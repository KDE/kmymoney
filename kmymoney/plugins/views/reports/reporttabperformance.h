/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mymoneyreport.h"

#include <QWidget>

namespace Ui {
class ReportTabPerformance;
}

class ReportTabPerformance : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ReportTabPerformance)

public:
    explicit ReportTabPerformance(QWidget* parent);
    ~ReportTabPerformance();

    bool apply(MyMoneyReport* report);
    bool load(MyMoneyReport* report);

Q_SIGNALS:
    void investmentSumChanged(eMyMoney::Report::InvestmentSum sumType);

protected:
    Ui::ReportTabPerformance* ui;
};
