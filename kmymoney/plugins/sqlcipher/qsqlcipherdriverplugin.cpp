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

#include "qsqlcipherdriverplugin.h"

#include "sqlcipherdriver.h"

QT_BEGIN_NAMESPACE

QSqlDriver* QSqlcipherDriverPlugin::create(const QString& name)
{
  if (name == QLatin1String("SQLCIPHER")) {
    QSQLiteDriver* driver = new SQLCipherDriver();
    return driver;
  }
  return 0;
}

QStringList QSqlcipherDriverPlugin::keys() const
{
  return QStringList("SQLCIPHER");
}

Q_EXPORT_PLUGIN2(qsqlcipherdriverplugin, QSqlcipherDriverPlugin);

QT_END_NAMESPACE
