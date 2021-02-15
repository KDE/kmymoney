/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
