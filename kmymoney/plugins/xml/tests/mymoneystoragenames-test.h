/*
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

#ifndef MYMONEYSTORAGENAMESTEST_H
#define MYMONEYSTORAGENAMESTEST_H

#include <QObject>

class MyMoneyStorageNamesTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void keyValuePairElementNames();
  void keyValuePairAttributeNames();
  void transactionElementNames();
  void transactionAttributeNames();
  void splitElementNames();
  void splitAttributeNames();
  void accountElementNames();
  void accountAttributeNames();
  void payeeElementNames();
  void payeeAttributeNames();
  void tagAttributeNames();
  void securityAttributeNames();
  void institutionElementNames();
  void institutionAttributeNames();
  void reportElementNames();
  void reportAttributeNames();
  void budgetElementNames();
  void budgetAttributeNames();
  void scheduleElementNames();
  void scheduleAttributeNames();
  void onlineJobElementNames();
  void onlineJobAttributeNames();

};

#endif
