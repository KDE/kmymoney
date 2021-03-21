/*
    SPDX-FileCopyrightText: 2010-2012 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CSVDATETEST_H
#define CSVDATETEST_H

#include <QObject>

class ConvertDate;

class CsvDateTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    /**
    * This method is used to test a series of valid and invalid dates,
    * including alpha month names, and different field separators.
    */
    void testConvertDate();
    /**
     * This test checks that Feb 30th is mapped to the last day in February
     */
    void testLastDayInFebruary();

private:
    ConvertDate* m_convert;
};
#endif
