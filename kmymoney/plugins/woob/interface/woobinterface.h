/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021-2026 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WOOBINTERFACE_H
#define WOOBINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

struct _object;
typedef _object PyObject;
struct _ts;
typedef _ts PyThreadState;

class WoobInterface
{
    PyObject* m_pythonSysModule = nullptr;
    PyObject* m_pythonSysPathVariable = nullptr;
    PyObject* m_pythonWoobModuleLocation = nullptr;
    PyObject* m_pythonWoobModule = nullptr;

    PyThreadState* m_pythonThreadState = nullptr;

public:
    struct Backend {
        QString name;
        QString module;
        QString login;
    };

    struct Transaction {
        QString id;
        QDate date;
        QDate rdate;
        enum type_t {
            TYPE_UNKNOWN = 0,
            TYPE_TRANSFER,
            TYPE_ORDER,
            TYPE_CHECK,
            TYPE_DEPOSIT,
            TYPE_PAYBACK,
            TYPE_WITHDRAWAL,
            TYPE_CARD,
            TYPE_LOAN_PAYMENT,
            TYPE_BANK,
        } type = TYPE_UNKNOWN;
        QString raw;
        QString category;
        QString label;
        QString payee;
        MyMoneyMoney amount;
    };

    struct Account {
        QString id;
        QString name;
        QString number;
        QString currency;
        enum type_t {
            TYPE_UNKNOWN = 0,
            TYPE_CHECKING,
            TYPE_SAVINGS,
            TYPE_DEPOSIT,
            TYPE_LOAN,
            TYPE_MARKET,
            TYPE_JOINT,
        } type = TYPE_UNKNOWN;
        MyMoneyMoney balance;

        QList<Transaction> transactions;
    };

    explicit WoobInterface();

    ~WoobInterface();

    bool isPythonInitialized() const;

    bool isWoobInitialized() const;

    QStringList getProtocols();

    QList<Backend> getBackends();

    QList<Account> getAccounts(QString backend);

    Account getAccount(QString backend, QString account, QString endDate);

private:
    PyObject* execute(QString method, QVariantList args);

    QString extractDictStringValue(PyObject* pyContainer, const char* szKey);
    long extractDictLongValue(PyObject* pyContainer, const char* szKey);
};

#endif
