/***************************************************************************
                         mymoneybalancecachetest  -  description
                            -------------------
   begin                : Tue Sep 21 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
