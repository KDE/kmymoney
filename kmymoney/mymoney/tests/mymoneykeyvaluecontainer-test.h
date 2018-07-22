/*
 * Copyright 2002-2013  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef MYMONEYKEYVALUECONTAINERTEST_H
#define MYMONEYKEYVALUECONTAINERTEST_H

#include <QObject>

class MyMoneyKeyValueContainer;

class MyMoneyKeyValueContainerTest : public QObject
{
  Q_OBJECT
protected:
  MyMoneyKeyValueContainer *m;

private Q_SLOTS:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testRetrieveValue();
  void testSetValue();
  void testDeletePair();
  void testClear();
  void testRetrieveList();
  void testLoadList();
  void testArrayRead();
  void testArrayWrite();
};

#endif
