/*
 * Copyright 2009-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2016       Christian Dávid <christian-david@web.de>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef MYMONEYPAYEETEST_H
#define MYMONEYPAYEETEST_H

#include <QObject>

class MyMoneyPayeeTest : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void testXml();
  void testDefaultAccount();
  void testEmptyMatchKeyBegin();
  void testEmptyMatchKeyEnd();
  void testEmptyMatchKeyMiddle();
  void testEmptyMatchKeyMix();
  void testMatchKeyDisallowSingleSpace();
  void testMatchKeyDisallowMultipleSpace();
  void testMatchKeyAllowSpaceAtStart();
  void testMatchKeyAllowSpaceAtEnd();
  void testMatchNameExact();
  void testElementNames();
  void testAttributeNames();
};

#endif
