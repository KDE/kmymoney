/*
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2003       Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2004       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2009-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
 * Copyright 2016       Christian Dávid <christian-david@web.de>
 * Copyright 2018       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef MYMONEYXMLCONTENTHANDLERTEST_H
#define MYMONEYXMLCONTENTHANDLERTEST_H

#include <QObject>

class MyMoneyXmlContentHandlerTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void readKeyValueContainer();
  void writeKeyValueContainer();
  void readTransaction();
  void readTransactionEx();
  void writeTransaction();
  void readAccount();
  void writeAccount();
  void readWritePayee();
  void readWriteTag();
  void readInstitution();
  void writeInstitution();
  void readSchedule();
  void writeSchedule();
  void testOverdue();
  void testNextPayment();
  void testNextPaymentOnLastDayOfMonth();
  void testPaymentDates();
  void testHasReferenceTo();
  void testPaidEarlyOneTime();
  void testReplaceId();

};

#endif
