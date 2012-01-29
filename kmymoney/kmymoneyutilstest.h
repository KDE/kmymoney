/***************************************************************************
 *   Copyright 2012  Thomas Baumgart  ipwizard@users.sourceforge.net       *
 *                                                                         *
 *   This file is part of KMyMoney.                                        *
 *                                                                         *
 *   KMyMoney is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU General Public License           *
 *   as published by the Free Software Foundation; either version 2 of     *
 *   the License or (at your option) version 3 or any later version.       *
 *                                                                         *
 *   KMyMoney is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#ifndef KMYMONEYUTILSTEST_H
#define KMYMONEYUTILSTEST_H

#include <QtCore/QObject>
#include <QtCore/QList>

#define KMM_MYMONEY_UNIT_TESTABLE friend class KMyMoneyUtilsTest;

#include "kmymoneyutils.h"

class KMyMoneyUtilsTest : public QObject
{
  Q_OBJECT
protected:

private slots:
  void initTestCase();
  void init();
  void cleanup();
  void testNextCheckNumber();
};

#endif
