/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2012 Thomas Baumgart <ipwizard@users.sourceforge.net>
 * Copyright (C) 2015 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KMYMONEYUTILSTEST_H
#define KMYMONEYUTILSTEST_H

#include <QObject>
#include <QtCore/QList>

#define KMM_MYMONEY_UNIT_TESTABLE friend class KMyMoneyUtilsTest;

#include "kmymoneyutils.h"

class KMyMoneyUtilsTest : public QObject
{
  Q_OBJECT
protected:

private Q_SLOTS:
  void initTestCase();
  void init();
  void cleanup();
  void testNextCheckNumber();
};

#endif
