/***************************************************************************
                          mymoneyreporttest.cpp
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

#include "mymoneyreport-test.h"

#include <QtCore/QDebug>

#include <QtTest/QtTest>

#include <QMetaEnum>

QTEST_GUILESS_MAIN(MyMoneyReportTest)

void MyMoneyReportTest::init()
{
  m = new MyMoneyReport();
}

void MyMoneyReportTest::cleanup()
{
  delete m;
}

void MyMoneyReportTest::testElementNames()
{
  for (auto i = (int)MyMoneyReport::Element::Payee; i <= (int)MyMoneyReport::Element::AccountGroup; ++i) {
    auto isEmpty = MyMoneyReport::getElName(static_cast<MyMoneyReport::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyReportTest::testAttributeNames()
{
  for (auto i = (int)MyMoneyReport::Attribute::ID; i < (int)MyMoneyReport::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyReport::getAttrName(static_cast<MyMoneyReport::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
