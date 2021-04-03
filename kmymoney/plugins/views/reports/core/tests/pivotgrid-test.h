/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PIVOTGRIDTEST_H
#define PIVOTGRIDTEST_H

#include <QObject>

namespace reports
{
class PivotGridTest;
}

#define KMM_MYMONEY_UNIT_TESTABLE friend class reports::PivotGridTest;

#include "mymoneyfile.h"

namespace reports
{

class PivotGridTest : public QObject
{
    Q_OBJECT
private:
    MyMoneyFile* file;

private Q_SLOTS:
    void init();
    void cleanup();
    void testCellAddValue();
    void testCellAddCell();
    void testCellRunningSum();
};

}

#endif // PIVOTGRIDTEST_H
