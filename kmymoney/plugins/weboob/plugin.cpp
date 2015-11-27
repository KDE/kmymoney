/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <KGenericFactory>
#include <kprogressdialog.h>

#include "dialogs/mapaccount.h"
#include "dialogs/webaccount.h"
#include "plugin.h"
#include "weboob.h"

class WeboobPlugin::Private
{
public:
  Private() : progress(0) {}
  ~Private() {
    delete progress;
  }

  QFutureWatcher<Weboob::Account> watcher;
  KProgressDialog* progress;
  WebAccountSettings* accountSettings;
};

K_PLUGIN_FACTORY(WeboobFactory, registerPlugin<WeboobPlugin>();)
K_EXPORT_PLUGIN(WeboobFactory("kmm_weboob"))

WeboobPlugin::WeboobPlugin(QObject *parent, const QVariantList&) :
    KMyMoneyPlugin::Plugin(parent, "Weboob"/*must be the same as X-KDE-PluginInfo-Name*/),
    KMyMoneyPlugin::OnlinePlugin(),
    d(new Private())
{
  setComponentData(WeboobFactory::componentData());
  setXMLFile("kmm_weboob.rc");

  connect(&d->watcher, SIGNAL(finished()), this, SLOT(gotAccount()));
}

WeboobPlugin::~WeboobPlugin()
{
  delete d;
}

void WeboobPlugin::protocols(QStringList& protocolList) const
{
  protocolList << "weboob";
}

QWidget* WeboobPlugin::accountConfigTab(const MyMoneyAccount& account, QString& tabName)
{
  const MyMoneyKeyValueContainer& kvp = account.onlineBankingSettings();
  tabName = i18n("Weboob configuration");

  d->accountSettings = new WebAccountSettings(account, 0);
  d->accountSettings->loadUi(kvp);

  return d->accountSettings;
}

MyMoneyKeyValueContainer WeboobPlugin::onlineBankingSettings(const MyMoneyKeyValueContainer& current)
{
  MyMoneyKeyValueContainer kvp(current);
  kvp["provider"] = objectName();
  if (d->accountSettings) {
    d->accountSettings->loadKvp(kvp);
  }
  return kvp;
}

bool WeboobPlugin::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings)
{
  Q_UNUSED(acc);

  WbMapAccountDialog w;
  w.weboob = &weboob;
  if (w.exec() == QDialog::Accepted) {
    onlineBankingSettings.setValue("wb-backend", w.backendsList->currentItem()->text(0));
    onlineBankingSettings.setValue("wb-id", w.accountsList->currentItem()->text(0));
	onlineBankingSettings.setValue("wb-max", "0");
    return true;
  }
  return false;
}

bool WeboobPlugin::updateAccount(const MyMoneyAccount& kacc, bool moreAccounts)
{
  Q_UNUSED(moreAccounts);

  QString bname = kacc.onlineBankingSettings().value("wb-backend");
  QString id = kacc.onlineBankingSettings().value("wb-id");
  QString max = kacc.onlineBankingSettings().value("wb-max");

  d->progress = new KProgressDialog(0, i18n("Connecting to bank..."), i18n("Retrieving transactions..."));
  d->progress->setModal(true);
  d->progress->setAllowCancel(false);
  d->progress->progressBar()->setMinimum(0);
  d->progress->progressBar()->setMaximum(0);
  d->progress->setMinimumDuration(0);

  QFuture<Weboob::Account> future = QtConcurrent::run(&weboob, &Weboob::getAccount, bname, id, max);
  d->watcher.setFuture(future);

  d->progress->exec();

  delete d->progress;
  d->progress = 0;

  return true;
}

void WeboobPlugin::gotAccount()
{
  Weboob::Account acc = d->watcher.result();

  MyMoneyAccount kacc = statementInterface()->account("wb-id", acc.id);
  MyMoneyStatement ks;

  ks.m_accountId = kacc.id();
  ks.m_strAccountName = acc.name;
  ks.m_closingBalance = acc.balance;
  if (acc.transactions.length() > 0)
    ks.m_dateEnd = acc.transactions.front().date;

#if 0
  switch(acc.type)
  {
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

  for (QListIterator<Weboob::Transaction> it(acc.transactions); it.hasNext(); )
  {
    Weboob::Transaction tr = it.next();
    MyMoneyStatement::Transaction kt;

    kt.m_strBankID = QString("ID ") + tr.id;
    kt.m_datePosted = tr.rdate;
    kt.m_amount = tr.amount;
    kt.m_strMemo = tr.raw;
    kt.m_strPayee = tr.label;

    ks.m_listTransactions += kt;
  }

  statementInterface()->import(ks);

  d->progress->hide();
}

#include "plugin.moc"
