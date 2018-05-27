/*
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mymoneyreport-test.h"

#include <QDebug>

#include <QtTest>

#include "mymoneyreport_p.h"

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
  for (auto i = (int)Report::Element::Payee; i <= (int)Report::Element::AccountGroup; ++i) {
    auto isEmpty = MyMoneyReportPrivate::getElName(static_cast<Report::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyReportTest::testAttributeNames()
{
  for (auto i = (int)Report::Attribute::ID; i < (int)Report::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyReportPrivate::getAttrName(static_cast<Report::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
