/*
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004       Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#ifndef MYMONEYACCOUNTTEST_H
#define MYMONEYACCOUNTTEST_H

#include <QObject>

class MyMoneyAccount;

class MyMoneyAccountTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyAccount *m;

private Q_SLOTS:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testConstructor();
  void testSetFunctions();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testSubAccounts();
  void testEquality();
  void testHasReferenceTo();
  void testAdjustBalance();
  void testSetClosed();
  void specialAccountTypes();
  void specialAccountTypes_data();
  void addReconciliation();
  void reconciliationHistory();
  void testHasOnlineMapping();
};

#endif
