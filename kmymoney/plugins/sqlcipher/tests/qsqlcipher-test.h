/*
 * Copyright 2014       Christian Dávid <christian-david@web.de>
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
