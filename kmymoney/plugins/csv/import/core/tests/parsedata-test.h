/*
    SPDX-FileCopyrightText: 2010-2012 Allan Anderson <agander93@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSEDATATEST_H
#define PARSEDATATEST_H

#include "../csvutil.h"

#include <QObject>

class Parse;

class ParseDataTest : public QObject
{
    Q_OBJECT

public:
    ParseDataTest();

    Parse* m_parse;

private Q_SLOTS:
    void init();
    void cleanup();
    void cleanupTestCase();
    void testDefaultConstructor();
    void testDefaultConstructor_data();
    void testConstructor();
    void testConstructor_data();
    void initTestCase();
    void initTestCase_data();

    /**
    * This method is used to test that a quoted string containing
    * a comma, which would get split by QString::split() when comma
    * is the field separator, is detected and rebuilt.  If the field
    * separator is not a comma, the split does not occur.
    */
    void parseSplitString();
    void parse_data();

};
#endif
