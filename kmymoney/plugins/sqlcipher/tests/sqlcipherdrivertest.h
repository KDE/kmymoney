/*
 * QSqlDriver for SQLCipher
 * Copyright 2014  Christian David <christian-david@web.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SQLCIPHERDRIVERTEST_H
#define SQLCIPHERDRIVERTEST_H

#include <QObject>
#include <QSqlDatabase>
#include <QTemporaryFile>

class sqlcipherdrivertest : public QObject
{
  Q_OBJECT
private:
  int data();

private Q_SLOTS:
  void initTestCase();
  void cleanupTestCase();

  void init();
  void cleanup();

  void createUnencryptedDatabase();

  void createDatabase();
  void checkReadAccess();
  void createTable();
  void write_data();
  void write();
  void count();
  void readData_data();
  void readData();

  void isFileEncrpyted();

  void checkForEncryption_data();
  void checkForEncryption();

  void reopenDatabase();
  void readData2_data();
  void readData2();

private:
  QTemporaryFile file;
  QSqlDatabase db;
};

#endif // SQLCIPHERDRIVERTEST_H
