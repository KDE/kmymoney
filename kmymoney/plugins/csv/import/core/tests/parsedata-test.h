/***************************************************************************
                           parsedatatest.h
                         ------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
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
