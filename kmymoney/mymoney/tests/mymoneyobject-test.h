/*
    SPDX-FileCopyrightText: 2005-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYOBJECTTEST_H
#define MYMONEYOBJECTTEST_H

#include <QObject>

#include "mymoneyobject.h"

class MyMoneyObjectTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEmptyConstructor();
    void testConstructor();
    void testClearId();
    void testCopyConstructor();
    void testAssignmentConstructor();
    void testEquality();
};

#endif
