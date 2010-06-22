/***************************************************************************
                          mymoneyobjecttest.h
                          -------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#ifndef MYMONEYOBJECTTEST_H
#define MYMONEYOBJECTTEST_H

#include <QtCore/QObject>

#define private public
#include "mymoneyobject.h"
#undef private

class MyMoneyObjectTest : public QObject
{
  Q_OBJECT
private slots:
  void testEmptyConstructor();
  void testConstructor();
  void testClearId();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testEquality();
};

#endif
