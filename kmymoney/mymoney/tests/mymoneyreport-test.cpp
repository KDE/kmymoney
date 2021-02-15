/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyreport-test.h"

#include <QDebug>

#include <QtTest>

#include "mymoneyreport_p.h"

QTEST_GUILESS_MAIN(MyMoneyReportTest)

void MyMoneyReportTest::init()
{
  m = new MyMoneyReport();
}

void MyMoneyReportTest::cleanup()
{
  delete m;
}
