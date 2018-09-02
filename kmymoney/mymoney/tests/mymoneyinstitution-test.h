/*
 * Copyright 2002-2010  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
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

#ifndef MYMONEYINSTITUTIONTEST_H
#define MYMONEYINSTITUTIONTEST_H

#include <QObject>

class MyMoneyInstitution;

class MyMoneyInstitutionTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyInstitution *m, *n;

private Q_SLOTS:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testSetFunctions();
  void testNonemptyConstructor();
  void testCopyConstructor();
  void testMyMoneyFileConstructor();
  void testEquality();
  void testInequality();
  void testAccountIDList();
};

#endif
