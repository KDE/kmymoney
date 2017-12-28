/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
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

#include "weboobext.h"

#include <QDebug>
#include <QMutex>
#include <QStandardPaths>

WeboobExt::WeboobExt(QObject* parent)
    : QObject(parent)
{
  mutex = new QMutex();
  path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kmm_weboob/weboob.py");
  action = new Kross::Action(0, path);
  action->setFile(path);
}

WeboobExt::~WeboobExt()
{
  delete mutex;
  action->finalize();
  delete action;
}

QVariant WeboobExt::execute(QString method, QVariantList args)
{
  QVariant result;

  mutex->lock();
  result = action->callFunction(method, args);
  mutex->unlock();

  return result;
}

QList<WeboobExt::Backend> WeboobExt::getBackends()
{
  QList<WeboobExt::Backend> backendsList;

  QVariantList args;

  QVariant result = this->execute("get_backends", args);

  QMap<QString, QVariant> list = result.toMap();
  QMapIterator<QString, QVariant> it(list);

  while (it.hasNext()) {
    it.next();
    QMap<QString, QVariant> params = it.value().toMap();

    WeboobExt::Backend backend;
    backend.name = it.key();
    backend.module = params["module"].toString();

    backendsList.append(backend);
  }

  return backendsList;
}


QList<WeboobExt::Account> WeboobExt::getAccounts(QString backend)
{
  QList<WeboobExt::Account> accountsList;

  QVariantList args;
  args << backend;
  QVariant result = this->execute("get_accounts", args);

  QMap<QString, QVariant> list = result.toMap();
  for (QMapIterator<QString, QVariant> it(list); it.hasNext();) {
    it.next();
    QMap<QString, QVariant> params = it.value().toMap();

    WeboobExt::Account account;
    account.id = it.key();
    account.name = params["name"].toString();
    account.balance = MyMoneyMoney(params["balance"].toInt(), 100);
    account.type = (WeboobExt::Account::type_t)params["type"].toInt();

    accountsList.append(account);
  }

  return accountsList;
}

WeboobExt::Account WeboobExt::getAccount(QString backend, QString accid, QString max)
{
  WeboobExt::Account acc;

  QVariantList args;
  args << backend;
  args << accid;
  args << max;
  QMap<QString, QVariant> result = this->execute("get_transactions", args).toMap();

  acc.id = result["id"].toString();
  acc.name = result["name"].toString();
  acc.balance = MyMoneyMoney(result["balance"].toInt(), 100);
  acc.type = (WeboobExt::Account::type_t)result["type"].toInt();

  QList<QVariant> list = result["transactions"].toList();
  for (QListIterator<QVariant> it(list); it.hasNext();) {
    QMap<QString, QVariant> params = it.next().toMap();
    WeboobExt::Transaction tr;

    tr.id = params["id"].toString();
    tr.date = QDate::fromString(params["date"].toString(), "yyyy-MM-dd");
    tr.rdate = QDate::fromString(params["rdate"].toString(), "yyyy-MM-dd");
    tr.type = (WeboobExt::Transaction::type_t)params["type"].toInt();
    tr.raw = params["raw"].toString();
    tr.category = params["category"].toString();
    tr.label = params["label"].toString();
    tr.amount = MyMoneyMoney(params["amount"].toInt(), 100);

    acc.transactions.append(tr);
  }
  return acc;
}
