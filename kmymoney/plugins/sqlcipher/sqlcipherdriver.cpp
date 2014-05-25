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

#include "sqlcipherdriver.h"
#include <QSqlQuery>
#include <QSqlError>
#include <sqlcipher/sqlite3.h>

bool SQLCipherDriver::open(const QString& db, const QString& user, const QString& password, const QString& host, int port, const QString& connOpts)
{
  if (QSQLiteDriver::open(db, user, password, host, port, connOpts)) {
    // SQLCipher does not accept empty passwords
    if (password.isEmpty())
      return true;

    // This is ugly but needed as the handle is stored in the private section of QSQLiteDriver
    sqlite3* handle = *static_cast<sqlite3**>(QSQLiteDriver::handle().data());
    if (sqlite3_key(handle, password.toUtf8().constData(), password.length()) == SQLITE_OK) {
      return true;
    } else {
      setLastError(QSqlError(QLatin1String("Error while setting passphrase for database."), QString(), QSqlError::ConnectionError, -1));
      QSQLiteDriver::setOpen(false);
      setOpenError(true);
    }
  }

  return false;
}
