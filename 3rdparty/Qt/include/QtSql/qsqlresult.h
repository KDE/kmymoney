/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtSql/qsql.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sql)

class QString;
class QSqlRecord;
template <typename T> class QVector;
class QVariant;
class QSqlDriver;
class QSqlError;
class QSqlResultPrivate;

class Q_SQL_EXPORT QSqlResult
{
    friend class QSqlQuery;
    friend class QSqlTableModelPrivate;
    friend class QSqlResultPrivate;

public:
    virtual ~QSqlResult();
    virtual QVariant handle() const;

protected:
    enum BindingSyntax {
        PositionalBinding,
        NamedBinding
#ifdef QT3_SUPPORT
        , BindByPosition = PositionalBinding,
        BindByName = NamedBinding
#endif
    };

    explicit QSqlResult(const QSqlDriver * db);
    int at() const;
    QString lastQuery() const;
    QSqlError lastError() const;
    bool isValid() const;
    bool isActive() const;
    bool isSelect() const;
    bool isForwardOnly() const;
    const QSqlDriver* driver() const;
    virtual void setAt(int at);
    virtual void setActive(bool a);
    virtual void setLastError(const QSqlError& e);
    virtual void setQuery(const QString& query);
    virtual void setSelect(bool s);
    virtual void setForwardOnly(bool forward);

    // prepared query support
    virtual bool exec();
    virtual bool prepare(const QString& query);
    virtual bool savePrepare(const QString& sqlquery);
    virtual void bindValue(int pos, const QVariant& val, QSql::ParamType type);
    virtual void bindValue(const QString& placeholder, const QVariant& val,
                           QSql::ParamType type);
    void addBindValue(const QVariant& val, QSql::ParamType type);
    QVariant boundValue(const QString& placeholder) const;
    QVariant boundValue(int pos) const;
    QSql::ParamType bindValueType(const QString& placeholder) const;
    QSql::ParamType bindValueType(int pos) const;
    int boundValueCount() const;
    QVector<QVariant>& boundValues() const;
    QString executedQuery() const;
    QString boundValueName(int pos) const;
    void clear();
    bool hasOutValues() const;

    BindingSyntax bindingSyntax() const;

    virtual QVariant data(int i) = 0;
    virtual bool isNull(int i) = 0;
    virtual bool reset(const QString& sqlquery) = 0;
    virtual bool fetch(int i) = 0;
    virtual bool fetchNext();
    virtual bool fetchPrevious();
    virtual bool fetchFirst() = 0;
    virtual bool fetchLast() = 0;
    virtual int size() = 0;
    virtual int numRowsAffected() = 0;
    virtual QSqlRecord record() const;
    virtual QVariant lastInsertId() const;

    enum VirtualHookOperation { BatchOperation, DetachFromResultSet, SetNumericalPrecision, NextResult };
    virtual void virtual_hook(int id, void *data);
    bool execBatch(bool arrayBind = false);
    void detachFromResultSet();
    void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy);
    QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const;
    bool nextResult();

private:
    QSqlResultPrivate* d;
    void resetBindCount(); // HACK

private:
    Q_DISABLE_COPY(QSqlResult)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSQLRESULT_H
