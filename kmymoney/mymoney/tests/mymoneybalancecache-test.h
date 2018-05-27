/*
 * Copyright 2009-2011  Fernando Vilas <fvilas@iname.com>
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
