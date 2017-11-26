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

#include "sqlcipherdrivertest.h"

#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlResult>
#include <QSqlRecord>

QTEST_GUILESS_MAIN(sqlcipherdrivertest);

static const QString passphrase = QLatin1String("blue valentines");

void sqlcipherdrivertest::initTestCase()
{
  // Called before the first testfunction is executed
  QVERIFY(file.open());
  file.close();
}

void sqlcipherdrivertest::cleanupTestCase()
{
  // Called after the last testfunction was executed
  db.close();
}

void sqlcipherdrivertest::init()
{
  // Called before each testfunction is executed
}

void sqlcipherdrivertest::cleanup()
{
  // Called after every testfunction
}

void sqlcipherdrivertest::createUnencryptedDatabase()
{
  QTemporaryFile file;
  file.open();
  file.close();

  QSqlDatabase db = QSqlDatabase::addDatabase("SQLCIPHER");
  QVERIFY(db.isValid());
  db.setDatabaseName(file.fileName());
  QVERIFY(db.open());

  QSqlQuery query;
  QVERIFY(query.exec("SELECT count(*) FROM sqlite_master;"));
  QVERIFY(!query.lastError().isValid());
  QVERIFY(query.next());
  QCOMPARE(query.value(0), QVariant(0));
  db.close();
}

void sqlcipherdrivertest::createDatabase()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("SQLCIPHER");
  QVERIFY(db.isValid());
  db.setDatabaseName(file.fileName());
  db.setPassword(passphrase);
  QVERIFY(db.open());
}

void sqlcipherdrivertest::checkReadAccess()
{
  QSqlQuery query;
  QVERIFY(query.exec("SELECT count(*) FROM sqlite_master;"));
  QVERIFY(!query.lastError().isValid());
}

void sqlcipherdrivertest::createTable()
{
  QSqlQuery query;
  QVERIFY(query.exec("CREATE TABLE test ("
                     "id int PRIMARY_KEY,"
                     "text varchar(20)"
                     ");"
                    ));
  QVERIFY(!query.lastError().isValid());
}

int sqlcipherdrivertest::data()
{
  QTest::addColumn<int>("id");
  QTest::addColumn<QString>("text");

  QTest::newRow("string Hello World") << 1 << "Hello World";
  QTest::newRow("string 20 characters") << 2 << "ABCDEFGHIJKLMNOPQRST";
  QTest::newRow("simple string") << 3 << "simple";

  return 3; // return number of rows!
}

void sqlcipherdrivertest::write_data()
{
  data();
}

void sqlcipherdrivertest::write()
{
  QFETCH(int, id);
  QFETCH(QString, text);

  QSqlQuery query;
  query.prepare("INSERT INTO test (id, text) "
                "VALUES (:id, :text);"
               );
  query.bindValue(":id", id);
  query.bindValue(":text", text);
  QVERIFY(query.exec());
  QVERIFY(!query.lastError().isValid());
  QCOMPARE(query.numRowsAffected(), 1);
}

void sqlcipherdrivertest::count()
{
  QSqlQuery query;
  QVERIFY(query.exec("SELECT count(id) FROM test;"));
  QVERIFY(query.next());
  QCOMPARE(query.value(0).toInt(), data());
}

void sqlcipherdrivertest::readData_data()
{
  data();
}

void sqlcipherdrivertest::readData()
{
  QFETCH(int, id);
  QFETCH(QString, text);

  QSqlQuery query;
  query.prepare("SELECT id, text FROM test WHERE id = :id;");
  query.bindValue(":id", QVariant::fromValue(id));
  QVERIFY(query.exec());

  QVERIFY(!query.lastError().isValid());
  QVERIFY(query.next());

  QSqlRecord record = query.record();
  int idIndex = record.indexOf("id");
  QVERIFY(idIndex != -1);
  int textIndex = record.indexOf("text");
  QVERIFY(textIndex != -1);

  QCOMPARE(query.value(0).toInt(), id);
  QCOMPARE(query.value(1).toString(), text);
  QVERIFY(!query.next());
}

/**
 * taken from @url http://www.keenlogics.com/qt-sqlite-driver-plugin-for-encrypted-databases-using-sqlcipher-windows/
 * thank you!
 */
void sqlcipherdrivertest::isFileEncrpyted()
{
  file.open();
  // http://www.sqlite.org/fileformat.html#database_header
  QString header = file.read(16);
  QVERIFY(header !=  "SQLite format 3\000");
  file.close();
}

void sqlcipherdrivertest::checkForEncryption_data()
{
  data();
}

void sqlcipherdrivertest::checkForEncryption()
{
  QFETCH(QString, text);

  QVERIFY(file.open());

  QString line;
  QTextStream in(&file);

  while (!in.atEnd()) {
    line = in.readLine();
    if (line.contains(text)) {
      QFAIL("Found unencrypted text in database file");
    }
  }

  file.close();
}

void sqlcipherdrivertest::reopenDatabase()
{
  db.close();

  db = QSqlDatabase::addDatabase("SQLCIPHER");
  QVERIFY(db.isValid());
  db.setDatabaseName(file.fileName());
  QVERIFY(db.open());

  QSqlQuery query;
  query.prepare("PRAGMA key = '" + passphrase + '\'');
  QVERIFY(query.exec());

  QVERIFY(!query.lastError().isValid());

  checkReadAccess();
}

void sqlcipherdrivertest::readData2_data()
{
  data();
}

void sqlcipherdrivertest::readData2()
{
  readData();
}
