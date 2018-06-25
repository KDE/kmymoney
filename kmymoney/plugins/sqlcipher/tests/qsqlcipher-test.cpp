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

#include "qsqlcipher-test.h"

#include <QtTest>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlResult>
#include <QSqlRecord>

QTEST_GUILESS_MAIN(qsqlciphertest)

static const auto passphrase(QStringLiteral("blue valentines"));
static const auto sqlDriverName(QStringLiteral("QSQLCIPHER"));

void qsqlciphertest::initTestCase()
{
  // Called before the first testfunction is executed
  auto drivers = QSqlDatabase::drivers();
  QVERIFY(drivers.contains(sqlDriverName));
  QVERIFY(m_file.open()); // open new temporary file just to get it's filename
  m_file.resize(0);       // it's important for the file to be empty during creation of encrypted database
  m_file.close();         // only filename was needed, so close it
  m_db = QSqlDatabase::addDatabase(sqlDriverName);
  QVERIFY(m_db.isValid());
  m_db.setDatabaseName(m_file.fileName());
  m_db.setPassword(passphrase);
  QVERIFY(m_db.open());
}

void qsqlciphertest::cleanupTestCase()
{
  m_db.close();
  m_db = QSqlDatabase();  // otherwise QSqlDatabase::removeDatabase says that m_db is still in use
  m_db.removeDatabase(QSqlDatabase::defaultConnection);
}

void qsqlciphertest::isSQLCipherUsed()
{
  QSqlQuery query(m_db);
  QVERIFY(query.exec("PRAGMA cipher_version"));
  QVERIFY(query.next());  // if that fails, then probably libsqlite3 instead libsqlcipher is in use
  qInfo() << "Cipher version: " << query.value(0).toString();
}

/**
 * taken from @url http://www.keenlogics.com/qt-sqlite-driver-plugin-for-encrypted-databases-using-sqlcipher-windows/
 * thank you!
 */
void qsqlciphertest::createEncryptedDatabase()
{
  QSqlQuery query(m_db);
  QVERIFY(query.exec(QString::fromLatin1("PRAGMA key = '%1'").arg(passphrase))); // this should happen immediately after opening the database, otherwise it cannot be encrypted
  m_file.open();
  // http://www.sqlite.org/fileformat.html#database_header
  auto header = QString(m_file.read(16));
  QVERIFY(header !=  QLatin1String("SQLite format 3\000")); // encrypted database has scrambled content in contrast to its regular SQLite counterpart
  m_file.close();
}

void qsqlciphertest::createTable()
{
  QSqlQuery query(m_db);
  QVERIFY(query.exec("CREATE TABLE test ("
                     "id int PRIMARY_KEY,"
                     "text varchar(20)"
                     ");"
                    ));
  QVERIFY(!query.lastError().isValid());
}

void qsqlciphertest::writeData_data()
{
  data();
}

void qsqlciphertest::writeData()
{
  QFETCH(int, id);
  QFETCH(QString, text);

  QSqlQuery query(m_db);
  query.prepare("INSERT INTO test (id, text) "
                "VALUES (:id, :text);"
               );
  query.bindValue(":id", id);
  query.bindValue(":text", text);
  QVERIFY(query.exec());
  QVERIFY(!query.lastError().isValid());
  QCOMPARE(query.numRowsAffected(), 1);
}

void qsqlciphertest::reopenDatabase()
{
  // close the encrypted database...
  m_db.close();
  m_db = QSqlDatabase();
  m_db.removeDatabase(QSqlDatabase::defaultConnection);

  // ... and open it anew
  m_db = QSqlDatabase::addDatabase(sqlDriverName);
  QVERIFY(m_db.isValid());
  m_db.setDatabaseName(m_file.fileName());
  m_db.setPassword(passphrase);
  QVERIFY(m_db.open());

  QSqlQuery query(m_db);
  QVERIFY(query.exec(QString::fromLatin1("PRAGMA key = '%1'").arg(passphrase))); // this should happen immediately after opening the database, to decrypt it properly

  QVERIFY(query.exec("SELECT count(*) FROM sqlite_master;")); // this is a check if the password correctly decrypts the database
  QVERIFY(!query.lastError().isValid());
}

void qsqlciphertest::countData()
{
  QSqlQuery query(m_db);
  QVERIFY(query.exec("SELECT count(id) FROM test;"));
  QVERIFY(query.next());
  QCOMPARE(query.value(0).toInt(), data());
}

void qsqlciphertest::readData_data()
{
  data();
}

void qsqlciphertest::readData()
{
  QFETCH(int, id);
  QFETCH(QString, text);

  QSqlQuery query(m_db);
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

int qsqlciphertest::data()
{
  QTest::addColumn<int>("id");
  QTest::addColumn<QString>("text");

  int i = 0;
  QTest::newRow("string Hello World") << ++i << "Hello World";
  QTest::newRow("string 20 characters") << ++i << "ABCDEFGHIJKLMNOPQRST";
  QTest::newRow("simple string") << ++i << "simple";

  return i; // return number of rows!
}
