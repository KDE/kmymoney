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

#include <QObject>

namespace reports
{
class PivotGridTest;
}

#define KMM_MYMONEY_UNIT_TESTABLE friend class reports::PivotGridTest;

#include "mymoneyfile.h"
#include "mymoneystoragemgr.h"

namespace reports
{

class PivotGridTest : public QObject
{
  Q_OBJECT
private:
  MyMoneyStorageMgr* storage;
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
