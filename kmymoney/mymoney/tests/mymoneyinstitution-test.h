/*
    SPDX-FileCopyrightText: 2002-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYINSTITUTIONTEST_H
#define MYMONEYINSTITUTIONTEST_H

#include <QObject>

class MyMoneyInstitution;

class MyMoneyInstitutionTest : public QObject
{
    Q_OBJECT

protected:
    MyMoneyInstitution *m, *n;

private Q_SLOTS:
    void init();
    void cleanup();
    void testEmptyConstructor();
    void testSetFunctions();
    void testNonemptyConstructor();
    void testCopyConstructor();
    void testMyMoneyFileConstructor();
    void testEquality();
    void testInequality();
    void testAccountIDList();
};

#endif
