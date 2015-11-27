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

#include <kprogressdialog.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "mapaccount.h"
#include "weboob.h"

class WbMapAccountDialog::Private
{
public:
  Private() : progress(0) {}
  ~Private() {
    delete progress;
  }

  QFutureWatcher<QList<Weboob::Account> > watcher;
  QFutureWatcher<QList<Weboob::Backend> > watcher2;
  KProgressDialog* progress;
};

WbMapAccountDialog::WbMapAccountDialog(QWidget *parent):
    QWizard(parent),
    d(new Private),
    d2(new Private)
{
  setupUi(this);

  checkNextButton();
  connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(checkNextButton()));
  connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(newPage(int)));
  connect(backendsList, SIGNAL(itemSelectionChanged()), this, SLOT(checkNextButton()));
  connect(accountsList, SIGNAL(itemSelectionChanged()), this, SLOT(checkNextButton()));

  connect(&d->watcher, SIGNAL(finished()), this, SLOT(gotAccounts()));
  connect(&d2->watcher2, SIGNAL(finished()), this, SLOT(gotBackends()));

  // setup icons
  button(QWizard::FinishButton)->setIcon(KStandardGuiItem::ok().icon());
  button(QWizard::CancelButton)->setIcon(KStandardGuiItem::cancel().icon());
  button(QWizard::NextButton)->setIcon(KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon());
  button(QWizard::BackButton)->setIcon(KStandardGuiItem::back(KStandardGuiItem::UseRTL).icon());
}

WbMapAccountDialog::~WbMapAccountDialog()
{
  delete d;
}

void WbMapAccountDialog::checkNextButton(void)
{
  bool enableButton = false;
  switch (currentId()) {
    case BACKENDS_PAGE:
      enableButton = backendsList->currentItem() != 0 && backendsList->currentItem()->isSelected();
      break;
    case ACCOUNTS_PAGE:
      enableButton = accountsList->currentItem() != 0 && accountsList->currentItem()->isSelected();
      break;
  }
  button(QWizard::NextButton)->setEnabled(enableButton);
}

void WbMapAccountDialog::newPage(int id)
{
  switch (id) {
    case BACKENDS_PAGE: {
        backendsList->clear();
        d2->progress = new KProgressDialog(this, i18n("Load Weboob backend..."), i18n("Getting list of backends."));
        d2->progress->setModal(true);
        d2->progress->setAllowCancel(false);
        d2->progress->progressBar()->setMinimum(0);
        d2->progress->progressBar()->setMaximum(0);
        d2->progress->setMinimumDuration(0);
        kapp->processEvents();

        QFuture<QList<Weboob::Backend> > future = QtConcurrent::run(weboob, &Weboob::getBackends);
        d2->watcher2.setFuture(future);

        break;
      }
    case ACCOUNTS_PAGE: {
        accountsList->clear();
        d->progress = new KProgressDialog(this, i18n("Connecting to bank..."), i18n("Getting list of accounts list from your bank."));
        d->progress->setModal(true);
        d->progress->setAllowCancel(false);
        d->progress->progressBar()->setMinimum(0);
        d->progress->progressBar()->setMaximum(0);
        d->progress->setMinimumDuration(0);
        kapp->processEvents();

        QFuture<QList<Weboob::Account> > future = QtConcurrent::run(weboob, &Weboob::getAccounts, backendsList->currentItem()->text(0));
        d->watcher.setFuture(future);
        button(QWizard::BackButton)->setEnabled(false);
        accountsList->setEnabled(false);

        break;
      }
  }
}

void WbMapAccountDialog::gotBackends()
{
  QList<Weboob::Backend> backends = d2->watcher2.result();

  for (QListIterator<Weboob::Backend> it(backends); it.hasNext();) {
    Weboob::Backend backend = it.next();
    QStringList headers;
    headers << backend.name << backend.module;
    backendsList->addTopLevelItem(new QTreeWidgetItem(headers));
  }

  delete d2->progress;
  d2->progress = 0;
}

void WbMapAccountDialog::gotAccounts()
{
  QList<Weboob::Account> accounts = d->watcher.result();

  for (QListIterator<Weboob::Account> it(accounts); it.hasNext();) {
    Weboob::Account account = it.next();
    QStringList headers;
    headers << account.id << account.name << account.balance.formatMoney(QString(), 2);
    accountsList->addTopLevelItem(new QTreeWidgetItem(headers));
  }

  delete d->progress;
  d->progress = 0;

  button(QWizard::BackButton)->setEnabled(true);
  accountsList->setEnabled(true);
}
