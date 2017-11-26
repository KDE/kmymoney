/***************************************************************************
                          mymoneybudgettest.h
                          -------------------
    copyright            : (C) 2010 by Thomas Baumgart
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

#ifndef MYMONEYBUDGETTEST_H
#define MYMONEYBUDGETTEST_H

#include <QObject>

class MyMoneyBudgetTest : public QObject
{
  Q_OBJECT

protected:
private Q_SLOTS:
  void init();
  void addMonthlyToMonthly();
  void addMonthlyToYearly();
  void addMonthlyToMonthByMonth();
  void addYearlyToMonthly();
  void addYearlyToYearly();
  void addYearlyToMonthByMonth();
  void addMonthByMonthToMonthly();
  void addMonthByMonthToYearly();
  void addMonthByMonthToMonthByMonth();
  void cleanup();
  void testElementNames();
  void testAttributeNames();
};

#endif
