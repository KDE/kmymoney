/*
    SPDX-FileCopyrightText: 2007-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYMAPTEST_H
#define MYMONEYMAPTEST_H

#include <QObject>

#include "mymoneytestutils.h"

#include "mymoneystoragemgr.h"
#include "mymoneymap.h"

class MyMoneyMapTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyMap<QString, QString> *m;
private Q_SLOTS:
  void init();
  void cleanup();
  void testArrayOperator();
  void testModifyKey();
  void testModifyKeyTwice();
};

#endif
