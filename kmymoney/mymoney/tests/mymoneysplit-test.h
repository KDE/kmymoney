/*
 * Copyright 2002-2016  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
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

#ifndef MYMONEYSPLITTEST_H
#define MYMONEYSPLITTEST_H

#include <QObject>

class MyMoneySplit;

class MyMoneySplitTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneySplit *m;

private Q_SLOTS:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testSetFunctions();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testEquality();
  void testInequality();
  void testAmortization();
  void testValue();
  void testSetValue();
  void testSetAction();
  void testIsAutoCalc();
  void testUnaryMinus();
};

#endif
