/*
    SPDX-FileCopyrightText: 2004-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSECURITYTEST_H
#define MYMONEYSECURITYTEST_H

#include <memory>
#include <QObject>

#include "mymoneytestutils.h"

class MyMoneySecurity;

class MyMoneySecurityTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT

protected:
    MyMoneySecurity *m;

private Q_SLOTS:
    void init();
    void cleanup();
    void testEmptyConstructor();
    void testNonemptyConstructor();
    void testCopyConstructor();
    void testSetFunctions();
    void testEquality();
    void testInequality();
    // void testMyMoneyFileConstructor();
    // void testAccountIDList ();
};

#endif
