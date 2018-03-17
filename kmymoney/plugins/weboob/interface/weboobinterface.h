/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
