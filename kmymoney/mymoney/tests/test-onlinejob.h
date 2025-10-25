/*
    SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEJOBTEST_H
#define ONLINEJOBTEST_H

#include <QObject>
#include <QString>

#include "mymoneytestutils.h"

class onlineJobTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT

private Q_SLOTS:
//    void initTestCase();
//    void cleanupTestCase();

    void testDefaultConstructor();
    void testCopyConstructor();
    void testCopyAssignment();
    void testCopyConstructorWithNewId();
};

#endif // ONLINEJOBTEST_H
