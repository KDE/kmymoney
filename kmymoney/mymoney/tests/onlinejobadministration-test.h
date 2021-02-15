/*
 * SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ONLINEJOBADMINISTRATIONTEST_H
#define ONLINEJOBADMINISTRATIONTEST_H

#include <QObject>
#include <QString>

class MyMoneyFile;

#define KMM_MYMONEY_UNIT_TESTABLE friend class onlineJobAdministrationTest;

class onlineJobAdministrationTest : public QObject
{
  Q_OBJECT

  MyMoneyFile* file;
  QString accountId;

public:
  onlineJobAdministrationTest();

private Q_SLOTS:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void getSettings();
  void registerOnlineTask();

private:
  void setupBaseCurrency();
};

#endif // ONLINEJOBADMINISTRATIONTEST_H
