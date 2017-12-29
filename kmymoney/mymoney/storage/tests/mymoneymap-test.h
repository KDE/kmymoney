/***************************************************************************
                          mymoneymaptest.h
                          -------------------
    copyright            : (C) 2007 by Thomas Baumgart
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

#ifndef MYMONEYMAPTEST_H
#define MYMONEYMAPTEST_H

#include <QObject>

#include "mymoneytestutils.h"

#include "mymoneyseqaccessmgr.h"
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
