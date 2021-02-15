/*
    SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEJOBTYPEDTEST_H
#define ONLINEJOBTYPEDTEST_H

#include "QObject"

#define KMM_MYMONEY_UNIT_TESTABLE friend class onlineJobTypedTest;

class onlineJobTypedTest : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void initTestCase();
  void cleanupTestCase();

  void copyContructor();
  void constructWithIncompatibleType();
  void constructWithNull();
  void copyByAssignment();
  void constructWithManadtoryDynamicCast();
};

#endif // ONLINEJOBTYPEDTEST_H
