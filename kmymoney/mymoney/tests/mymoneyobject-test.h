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

#ifndef MYMONEYOBJECTTEST_H
#define MYMONEYOBJECTTEST_H

#include <QObject>

#include "mymoneyobject.h"

class MyMoneyObjectTest : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void testEmptyConstructor();
  void testConstructor();
  void testClearId();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testEquality();
  void testReadXML();
};

#endif
