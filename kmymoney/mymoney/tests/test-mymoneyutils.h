/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYUTILSTEST_H
#define MYMONEYUTILSTEST_H

#include <QLocale>
#include <QObject>

#include "mymoneytestutils.h"

class MyMoneyUtilsTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT

private:
    QString firstShort;

private Q_SLOTS:
    void init();
    void cleanup();
    void commonTestData();
    void testConvertWildcardToRegularExpression_data();
    void testConvertWildcardToRegularExpression();
    void testConvertRegularExpressionToWildcard_data();
    void testConvertRegularExpressionToWildcard();
    void testExtractId_data();
    void testExtractId();
    void testIsoStringToDateTime_data();
    void testIsoStringToDateTime();
    void testDateTimeToIsoString_data();
    void testDateTimeToIsoString();
    void testDateToString_data();
    void testDateToString();
    void testStringToDate_data();
    void testStringToDate();
};

#endif // MYMONEYUTILSTEST_H
