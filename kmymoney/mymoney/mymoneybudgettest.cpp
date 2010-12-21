/***************************************************************************
                          mymoneybudgettest.cpp
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

#include "mymoneybudgettest.h"

#include <QtTest/QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyMoneyTest;

QTEST_MAIN(MyMoneyBudgetTest)

void MyMoneyBudgetTest::init()
{
}

void MyMoneyBudgetTest::cleanup()
{
}

#include "mymoneybudgettest.moc"
