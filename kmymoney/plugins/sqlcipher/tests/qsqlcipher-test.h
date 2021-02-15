/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QSQLCIPHER_TEST_H
#define QSQLCIPHER_TEST_H

#include <QObject>
#include <QSqlDatabase>
#include <QTemporaryFile>

class qsqlciphertest : public QObject
{
  Q_OBJECT
private:
  int data();

private Q_SLOTS:
  void initTestCase();
  void cleanupTestCase();

  void isSQLCipherUsed();
  void createEncryptedDatabase();
  void createTable();
  void writeData_data();
  void writeData();
  void reopenDatabase();
  void countData();
  void readData_data();
  void readData();

private:
  QTemporaryFile m_file;
  QSqlDatabase m_db;
};

#endif // QSQLCIPHER_TEST_H
