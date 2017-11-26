/***************************************************************************
                          mymoneyfinancialcalculatortest.h
                          -------------------
    copyright            : (C) 2003 by Thomas Baumgart
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

#ifndef MYMONEYFINANCIALCALCULATORTEST_H
#define MYMONEYFINANCIALCALCULATORTEST_H

#include <QObject>

class MyMoneyFinancialCalculator;

class MyMoneyFinancialCalculatorTest : public QObject
{
  Q_OBJECT
protected:
  MyMoneyFinancialCalculator *m;

private Q_SLOTS:

  void init();
  void cleanup();
  void testEmptyConstructor();
  void testSetPrec();
  void testSetNpp();
  void testSetPF();
  void testSetCF();
  void testSetBep();
  void testSetDisc();
  void testSetIr();
  void testSetPv();
  void testSetPmt();
  void testSetFv();
  void testCombinedSet();
  void testNumPayments();
  void testUseCase1();
  void testUseCase2();
};

#endif
