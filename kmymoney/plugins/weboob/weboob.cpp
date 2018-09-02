/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 * Copyright (C) 2016 Christian David <christian-david@web.de>
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

#include "weboob.h"

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mapaccountwizard.h"
#include "accountsettings.h"
#include "weboobinterface.h"

#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneystatement.h"
#include "statementinterface.h"

class WeboobPrivate
{
public:
  WeboobPrivate()
  {
  }

  ~WeboobPrivate()
  {
  }

  WeboobInterface weboob;
  QFutureWatcher<WeboobInterface::Account> watcher;
  std::unique_ptr<QProgressDialog> progress;
  AccountSettings* accountSettings;
};

Weboob::Weboob(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "weboob"),
  d_ptr(new WeboobPrivate)
{
  Q_UNUSED(args)
  setComponentName("weboob", i18n("Weboob"));
  setXMLFile("weboob.rc");

  qDebug("Plugins: weboob loaded");
}

Weboob::~Weboob()
{
  Q_D(Weboob);
  delete d;
  qDebug("Plugins: weboob unloaded");
}

void Weboob::plug()
{
  Q_D(Weboob);
  connect(&d->watcher, &QFutureWatcher<WeboobInterface::Account>::finished, this, &Weboob::gotAccount);
}

void Weboob::unplug()
{
  Q_D(Weboob);
  disconnect(&d->watcher, &QFutureWatcher<WeboobInterface::Account>::finished, this, &Weboob::gotAccount);
}

void Weboob::protocols(QStringList& protocolList) const
{
  protocolList << "weboob";
}

QWidget* Weboob::accountConfigTab(const MyMoneyAccount& account, QString& tabName)
{
  Q_D(Weboob);
  const MyMoneyKeyValueContainer& kvp = account.onlineBankingSettings();
  tabName = i18n("Weboob configuration");

  d->accountSettings = new AccountSettings(account, 0);
  d->accountSettings->loadUi(kvp);

  return d->accountSettings;
}

MyMoneyKeyValueContainer Weboob::onlineBankingSettings(const MyMoneyKeyValueContainer& current)
{
  Q_D(Weboob);
  MyMoneyKeyValueContainer kvp(current);
  kvp["provider"] = objectName().toLower();
  if (d->accountSettings) {
    d->accountSettings->loadKvp(kvp);
  }
  return kvp;
}

bool Weboob::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings)
{
  Q_D(Weboob);
  Q_UNUSED(acc);

  bool rc = false;
  QPointer<MapAccountWizard> w = new MapAccountWizard(nullptr, &d->weboob);
  if (w->exec() == QDialog::Accepted && w != nullptr) {
    onlineBankingSettings.setValue("wb-backend", w->currentBackend());
    onlineBankingSettings.setValue("wb-id", w->currentAccount());
    onlineBankingSettings.setValue("wb-max", "0");
    rc = true;
  }
  delete w;
  return rc;
}

bool Weboob::updateAccount(const MyMoneyAccount& kacc, bool moreAccounts)
{
  Q_D(Weboob);
  Q_UNUSED(moreAccounts);

  QString bname = kacc.onlineBankingSettings().value("wb-backend");
  QString id = kacc.onlineBankingSettings().value("wb-id");
  QString max = kacc.onlineBankingSettings().value("wb-max");

  d->progress = std::make_unique<QProgressDialog>(nullptr);
  d->progress->setWindowTitle(i18n("Connecting to bank..."));
  d->progress->setLabelText(i18n("Retrieving transactions..."));
  d->progress->setModal(true);
  d->progress->setCancelButton(nullptr);
  d->progress->setMinimum(0);
  d->progress->setMaximum(0);
  d->progress->setMinimumDuration(0);

  QFuture<WeboobInterface::Account> future = QtConcurrent::run(&d->weboob, &WeboobInterface::getAccount, bname, id, max);
  d->watcher.setFuture(future);

  d->progress->exec();
  d->progress.reset();

  return true;
}

void Weboob::gotAccount()
{
  Q_D(Weboob);
  WeboobInterface::Account acc = d->watcher.result();

  MyMoneyAccount kacc = statementInterface()->account("wb-id", acc.id);
  MyMoneyStatement ks;

  ks.m_accountId = kacc.id();
  ks.m_strAccountName = acc.name;
  ks.m_closingBalance = acc.balance;
  if (acc.transactions.length() > 0)
    ks.m_dateEnd = acc.transactions.front().date;

#if 0
  switch (acc.type) {
    case Weboob::Account::TYPE_CHECKING:
      ks.m_eType = MyMoneyStatement::etCheckings;
      break;
    case Weboob::Account::TYPE_SAVINGS:
      ks.m_eType = MyMoneyStatement::etSavings;
      break;
    case Weboob::Account::TYPE_MARKET:
      ks.m_eType = MyMoneyStatement::etInvestment;
      break;
    case Weboob::Account::TYPE_DEPOSIT:
    case Weboob::Account::TYPE_LOAN:
    case Weboob::Account::TYPE_JOINT:
    case Weboob::Account::TYPE_UNKNOWN:
      break;
  }
#endif

  for (QListIterator<WeboobInterface::Transaction> it(acc.transactions); it.hasNext();) {
    WeboobInterface::Transaction tr = it.next();
    MyMoneyStatement::Transaction kt;

    kt.m_strBankID = QLatin1String("ID ") + tr.id;
    kt.m_datePosted = tr.rdate;
    kt.m_amount = tr.amount;
    kt.m_strMemo = tr.raw;
    kt.m_strPayee = tr.label;

    ks.m_listTransactions += kt;
  }

  statementInterface()->import(ks);

  d->progress->hide();
}

K_PLUGIN_FACTORY_WITH_JSON(WeboobFactory, "weboob.json", registerPlugin<Weboob>();)

#include "weboob.moc"
