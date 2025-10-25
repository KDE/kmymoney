/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMSETTEST_H
#define KMMSETTEST_H

#include <QObject>

#include "mymoneytestutils.h"
#include <kmmset.h>

class KMMSetTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT

protected:
private Q_SLOTS:
    void init();
    void cleanup();
    void testEmptyConstructor();
    void testConstructor();
    void testCopyConstructor();
    void testAssignmentConstructor();
    void testDuplicateItems();
    void testContains();
    void testIntersect();
    void testUnite();
    void testSubtract();
    void testValues();
};

#endif // KMMSETTEST_H
