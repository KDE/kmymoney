/***************************************************************************
                          pivotgridtest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PIVOTGRIDTEST_H
#define PIVOTGRIDTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "mymoneyfile.h"
#include "mymoneyseqaccessmgr.h"

class PivotGridTest : public CppUnit::TestFixture  {
  CPPUNIT_TEST_SUITE(PivotGridTest);
  CPPUNIT_TEST(testCellAddValue);
  CPPUNIT_TEST(testCellAddCell);
  CPPUNIT_TEST(testCellRunningSum);
  CPPUNIT_TEST_SUITE_END();

private:
  MyMoneyAccount  *m;

  MyMoneySeqAccessMgr* storage;
  MyMoneyFile* file;

public:
  PivotGridTest();
  void setUp ();
  void tearDown ();
  void testCellAddValue();
  void testCellAddCell();
  void testCellRunningSum();
};

#endif // PIVOTGRIDTEST_H
