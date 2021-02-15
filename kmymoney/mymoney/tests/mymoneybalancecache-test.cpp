/*
    SPDX-FileCopyrightText: 2009-2011 Fernando Vilas <fvilas@iname.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneybalancecache-test.h"

#include <QDebug>

#include <QtTest>

#include "mymoneytestutils.h"
#include "mymoneyexception.h"
#include "mymoneybalancecache.h"

QTEST_GUILESS_MAIN(MyMoneyBalanceCacheTest)

void MyMoneyBalanceCacheTest::init()
{
  m = new MyMoneyBalanceCache();
}

void MyMoneyBalanceCacheTest::cleanup()
{
  delete m;
}

void MyMoneyBalanceCacheTest::testCacheItem()
{
  MyMoneyBalanceCacheItem item(MyMoneyMoney(10, 100), QDate(2010, 9, 18));
  MyMoneyBalanceCacheItem invalid(MyMoneyMoney::minValue, QDate());
  //MyMoneyBalanceCacheItem noarg;


  QVERIFY(item.balance() == MyMoneyMoney(10, 100));
  QVERIFY(item.date() == QDate(2010, 9, 18));
  QVERIFY(item.isValid());

  QVERIFY(! invalid.isValid());

  //QVERIFY(! noarg.isValid());

}

void MyMoneyBalanceCacheTest::testEmpty()
{
  QVERIFY(m->isEmpty());
  testInsert();
  QVERIFY(! m->isEmpty());
}

void MyMoneyBalanceCacheTest::testInsert()
{
  m->insert("A000001", QDate(2010, 9, 16), MyMoneyMoney(10, 100));

  // The next 2 lines have the same date and account.
  // This is intentional to see that the balance for that
  // account and date gets overwritten.
  m->insert("A000001", QDate(2010, 9, 18), MyMoneyMoney(15, 100));
  m->insert("A000001", QDate(2010, 9, 18), MyMoneyMoney(20, 100));
  m->insert("A000002", QDate(2010, 9, 17), MyMoneyMoney(30, 100));
  m->insert("A000002", QDate(2010, 9, 19), MyMoneyMoney(40, 100));
}

void MyMoneyBalanceCacheTest::testClear()
{
  QVERIFY(m->size() == 0);
  testInsert();
  QVERIFY(m->size() == 4);

  // Delete an item that is not in the cache. The cache should
  // be unaffected.
  m->clear("A000003", QDate(2010, 9, 17));
  QVERIFY(m->size() == 4);

  // Now delete a value before the last one in the account.
  // All values after it should also be gone.
  m->clear("A000001", QDate(2010, 9, 17));
  QVERIFY(m->size() == 3);

  // Verify that the items not deleted still exist
  m->clear("A000001");
  QVERIFY(m->size() == 2);
  m->clear();
  QVERIFY(m->size() == 0);

  // Verify that searching for something not in the list is safe
  try {
    m->clear("A000001");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

}

void MyMoneyBalanceCacheTest::testSize()
{
  QVERIFY(m->size() == 0);
  testInsert();
  QVERIFY(m->size() == 4);
}

void MyMoneyBalanceCacheTest::testRetrieve()
{
  testInsert();

  MyMoneyBalanceCacheItem item = m->balance("A000003", QDate(2010, 9, 17));
  QVERIFY(! item.isValid());

  item = m->balance("A000001", QDate(2010, 9, 16));
  QVERIFY(item.isValid());
  QVERIFY(item.balance() == MyMoneyMoney(10, 100));
  QVERIFY(item.date() == QDate(2010, 9, 16));

  item = m->balance("A000001", QDate(2010, 9, 17));
  QVERIFY(! item.isValid());

  item = m->balance("A000001", QDate(2010, 9, 18));
  QVERIFY(item.isValid());
  QVERIFY(item.balance() == MyMoneyMoney(20, 100));
  QVERIFY(item.date() == QDate(2010, 9, 18));

  // Test bad acct
  item = m->mostRecentBalance("A000003", QDate(2010, 9, 17));
  QVERIFY(! item.isValid());

  // Test date too old
  item = m->mostRecentBalance("A000001", QDate(2010, 9, 15));
  QVERIFY(!item.isValid());

  // Test date found
  item = m->mostRecentBalance("A000001", QDate(2010, 9, 16));
  QVERIFY(item.isValid());
  QVERIFY(item.balance() == MyMoneyMoney(10, 100));
  QVERIFY(item.date() == QDate(2010, 9, 16));

  // Test date in between
  item = m->mostRecentBalance("A000001", QDate(2010, 9, 17));
  QVERIFY(item.isValid());
  QVERIFY(item.balance() == MyMoneyMoney(10, 100));
  QVERIFY(item.date() == QDate(2010, 9, 16));

  // Test date found
  item = m->mostRecentBalance("A000001", QDate(2010, 9, 18));
  QVERIFY(item.isValid());
  QVERIFY(item.balance() == MyMoneyMoney(20, 100));
  QVERIFY(item.date() == QDate(2010, 9, 18));

  // Test date later than all entries
  item = m->mostRecentBalance("A000001", QDate(2010, 9, 19));
  QVERIFY(item.isValid());
  QVERIFY(item.balance() == MyMoneyMoney(20, 100));
  QVERIFY(item.date() == QDate(2010, 9, 18));
}
