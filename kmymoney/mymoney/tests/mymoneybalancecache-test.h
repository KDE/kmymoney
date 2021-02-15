/*
    SPDX-FileCopyrightText: 2009-2011 Fernando Vilas <fvilas@iname.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYBALANCECACHETEST_H
#define MYMONEYBALANCECACHETEST_H

#include <QObject>

#include "mymoneybalancecache.h"

class MyMoneyBalanceCacheTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyBalanceCache* m;

private Q_SLOTS:
  void init();
  void cleanup();
  void testCacheItem();
  void testEmpty();
  void testInsert();
  void testClear();
  void testSize();
  void testRetrieve();
};

#endif
