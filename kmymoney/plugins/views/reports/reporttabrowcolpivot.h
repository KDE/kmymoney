/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REPORTTABROWCOLPIVOT_H
#define REPORTTABROWCOLPIVOT_H

#include "mymoneybudget.h"
#include "mymoneyreport.h"

#include <QWidget>

namespace Ui {
class ReportTabRowColPivot;
}

class ReportTabRowColPivot : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ReportTabRowColPivot)

public:
    explicit ReportTabRowColPivot(QWidget* parent);
    ~ReportTabRowColPivot();

    bool apply(MyMoneyReport* report, bool budgetActual, const QVector<MyMoneyBudget>& budgets);
    bool load(MyMoneyReport* report, const QVector<MyMoneyBudget>& budgets);

    bool comboBudgetEnabled();
    bool comboRowsIsIncomeExpense();
    void setCheckTransfersEnabled(bool state);
    void setTotalColumnEnabled(bool state);

Q_SIGNALS:
    void comboRowsActivated(int index);

private:
    Ui::ReportTabRowColPivot* ui;
};

#endif // REPORTTABROWCOLPIVOT_H
