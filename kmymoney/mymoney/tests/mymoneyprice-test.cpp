/*
 * Copyright 2005-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneyprice-test.h"

#include <QtTest>

#include "mymoneyexception.h"
#include "mymoneymoney.h"

QTEST_GUILESS_MAIN(MyMoneyPriceTest)

void MyMoneyPriceTest::init()
{
  m = new MyMoneyPrice();
}

void MyMoneyPriceTest::cleanup()
{
  delete m;
}

void MyMoneyPriceTest::testDefaultConstructor()
{
  QVERIFY(m->isValid() == false);
}

void MyMoneyPriceTest::testConstructor()
{
  MyMoneyPrice n(QString("from"), QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));

  QVERIFY(n.isValid() == true);
  QVERIFY(n.from() == QString("from"));
  QVERIFY(n.to() == QString("to"));
  QVERIFY(n.date() == QDate(2005, 9, 23));
  QVERIFY(n.source() == QString("MySource"));
  QVERIFY(n.rate("to") == MyMoneyMoney(1, 3));
}

void MyMoneyPriceTest::testValidity()
{
  QString emptyId;
  MyMoneyPrice n1(emptyId, QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n2(QString("from"), emptyId, QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n3(QString("from"), QString("to"), QDate(), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n4(QString("from"), QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));

  QVERIFY(n1.isValid() == false);
  QVERIFY(n2.isValid() == false);
  QVERIFY(n3.isValid() == false);
  QVERIFY(n4.isValid() == true);
}

void MyMoneyPriceTest::testRate()
{
  MyMoneyPrice n1(QString("from"), QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n2(QString("from"), QString("to"), QDate(), MyMoneyMoney(1, 3),  QString("MySource"));

  try {
    QVERIFY(n1.rate("to") == MyMoneyMoney(1, 3));
    QVERIFY(n1.rate("from") == MyMoneyMoney(3, 1));
    QVERIFY(n1.rate(QString()) == MyMoneyMoney(1, 3));

    QVERIFY(n2.isValid() == false);
    QVERIFY(n2.rate("to") == MyMoneyMoney::ONE);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  try {
    n1.rate("unknown");
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }
}
