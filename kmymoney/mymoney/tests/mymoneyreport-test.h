/***************************************************************************
                          mymoneyreporttest.h
                          -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYREPORTTEST_H
#define MYMONEYREPORTTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyReportTest;

#include "mymoneyreport.h"

class MyMoneyReportTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyReport *m;

private Q_SLOTS:
  void init();
  void cleanup();
  void testElementNames();
  void testAttributeNames();
};
#endif
