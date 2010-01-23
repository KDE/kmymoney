/***************************************************************************
                          mymoneymaptest.cpp
                          -------------------
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneymaptest.h"
#include <iostream>

MyMoneyMapTest::MyMoneyMapTest()
{
}


void MyMoneyMapTest::setUp()
{
  m = new MyMoneyMap<QString, QString>;
}

void MyMoneyMapTest::tearDown()
{
  delete m;
}

void MyMoneyMapTest::testArrayOperator()
{
}

