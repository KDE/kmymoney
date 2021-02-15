/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WEBOOBINTERFACE_H
#define WEBOOBINTERFACE_H

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

class WeboobInterface
{
  PyObject  *m_weboobInterface;

public:

  struct Backend {
    QString name;
    QString module;
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
      TYPE_BANK
    } type;
    QString raw;
    QString category;
    QString label;
    MyMoneyMoney amount;
  };

  struct Account {
    QString id;
    QString name;
    enum type_t {
      TYPE_UNKNOWN = 0,
      TYPE_CHECKING,
      TYPE_SAVINGS,
      TYPE_DEPOSIT,
      TYPE_LOAN,
      TYPE_MARKET,
      TYPE_JOINT
    } type;
    MyMoneyMoney balance;

    QList<Transaction> transactions;
  };

  explicit WeboobInterface();

  ~WeboobInterface();

  QStringList getProtocols();

  QList<Backend> getBackends();

  QList<Account> getAccounts(QString backend);

  Account getAccount(QString backend, QString account, QString max);

private:

  PyObject* execute(QString method, QVariantList args);

  QString extractDictStringValue(PyObject* pyContainer, const char *szKey);
  long extractDictLongValue(PyObject* pyContainer, const char* szKey);
};

#endif
