/*
    SPDX-FileCopyrightText: 2012 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYUTILSTEST_H
#define KMYMONEYUTILSTEST_H

#include <QObject>
#include <QList>

#define KMM_MYMONEY_UNIT_TESTABLE friend class KMyMoneyUtilsTest;

#include "kmymoneyutils.h"

class KMyMoneyUtilsTest : public QObject
{
    Q_OBJECT
protected:

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testNextCheckNumber();
};

#endif
