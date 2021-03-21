/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYREPORTTEST_H
#define MYMONEYREPORTTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyReportTest;

#include "mymoneyreport.h"

class MyMoneyReportTest : public QObject
{
    Q_OBJECT

protected:
    MyMoneyReport *m;

private Q_SLOTS:
    void init();
    void cleanup();
};
#endif
