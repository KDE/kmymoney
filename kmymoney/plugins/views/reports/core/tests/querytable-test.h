/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QUERYTABLETEST_H
#define QUERYTABLETEST_H

#include <QObject>
#include "mymoneyfile.h"

class QueryTableTest : public QObject
{
    Q_OBJECT
private:
    MyMoneyFile* file;

private Q_SLOTS:
    void setup();
    void init();
    void cleanup();
    void testQueryBasics();
    void testCashFlowAnalysis();
    void testAccountQuery();
    void testInvestment();
    void testSplitShares();
    void testConversionRate();
    void testBalanceColumn();
    void testBalanceColumnWithMultipleCurrencies();
    void testTaxReport();
    void testProtectedMethods();
};

#endif
