/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYUTILSTEST_H
#define MYMONEYUTILSTEST_H

#include <QObject>

class MyMoneyUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void commonTestData();
    void testConvertWildcardToRegularExpression_data();
    void testConvertWildcardToRegularExpression();
    void testConvertRegularExpressionToWildcard_data();
    void testConvertRegularExpressionToWildcard();
};

#endif // MYMONEYUTILSTEST_H
