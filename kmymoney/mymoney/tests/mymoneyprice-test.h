/*
    SPDX-FileCopyrightText: 2005-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYPRICETEST_H
#define MYMONEYPRICETEST_H

#include <QObject>

#include "mymoneyprice.h"

class MyMoneyPriceTest : public QObject
{
    Q_OBJECT

protected:
    MyMoneyPrice* m;

private Q_SLOTS:
    void init();
    void cleanup();

    void testDefaultConstructor();
    void testConstructor();
    void testValidity();
    void testRate();

};
#endif
