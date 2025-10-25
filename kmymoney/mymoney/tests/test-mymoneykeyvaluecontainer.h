/*
    SPDX-FileCopyrightText: 2002-2013 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYKEYVALUECONTAINERTEST_H
#define MYMONEYKEYVALUECONTAINERTEST_H

#include <QObject>

#include "mymoneytestutils.h"

class MyMoneyKeyValueContainer;

class MyMoneyKeyValueContainerTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT
protected:
    MyMoneyKeyValueContainer* m;

private Q_SLOTS:
    void init();
    void cleanup();
    void testEmptyConstructor();
    void testRetrieveValue();
    void testRetrieveDefaultValue();
    void testSetValue();
    void testDeletePair();
    void testClear();
    void testRetrieveList();
    void testLoadList();
    void testArrayRead();
    void testArrayWrite();
};

#endif
