/*
    SPDX-FileCopyrightText: 2016-2018 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHARTTEST_H
#define CHARTTEST_H

#include <QObject>

class ChartTest: public QObject
{
    Q_OBJECT

private slots:
    void createChart();

};

#endif // CHARTTEST_H
