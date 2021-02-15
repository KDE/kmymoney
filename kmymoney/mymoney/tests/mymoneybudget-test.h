/*
 * SPDX-FileCopyrightText: 2010-2011 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
};

#endif
