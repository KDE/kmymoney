/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 20267 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REPORTTABROWCOLPERFORMANCE_H
#define REPORTTABROWCOLPERFORMANCE_H

#include "mymoneyreport.h"

#include <QWidget>

namespace Ui {
class ReportTabPerformanceColumns;
}

class ReportTabRowColPerformance : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ReportTabRowColPerformance)

public:
    explicit ReportTabRowColPerformance(QWidget* parent);
    ~ReportTabRowColPerformance();

    bool apply(MyMoneyReport* report);
    bool load(MyMoneyReport* report);
    bool isModified() const;

public Q_SLOTS:
    void slotUpdatePerformanceColumnVisibility(eMyMoney::Report::InvestmentSum sumType);

private:
    Ui::ReportTabPerformanceColumns* ui;
    bool m_isModified;
    bool m_isLoading;
};

#endif // REPORTTABROWCOLQUERY_H
