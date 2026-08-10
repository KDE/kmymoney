/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REPORTTABROWCOLQUERY_H
#define REPORTTABROWCOLQUERY_H

#include "mymoneyreport.h"

#include <QWidget>

namespace Ui {
class ReportTabRowColQuery;
}

class ReportTabRowColQuery : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ReportTabRowColQuery)

public:
    explicit ReportTabRowColQuery(QWidget* parent);
    ~ReportTabRowColQuery();

    bool apply(MyMoneyReport* reportState);
    bool load(MyMoneyReport* report);

    void applyConvertCurrencyChanged(MyMoneyReport* report, int state);

private Q_SLOTS:
    void slotHideTransactionsChanged(bool checked);

private:
    Ui::ReportTabRowColQuery* ui;
};

#endif // REPORTTABROWCOLQUERY_H
