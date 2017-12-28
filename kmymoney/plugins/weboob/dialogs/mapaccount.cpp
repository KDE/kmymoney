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

#include "mapaccount.h"

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressDialog>

#include <kmessagebox.h>
#include <klocalizedstring.h>

#include "../weboob.h"

struct WbMapAccountDialog::Private
{
  QFutureWatcher<QList<WeboobExt::Account> > watcher;
  QFutureWatcher<QList<WeboobExt::Backend> > watcher2;
  std::unique_ptr<QProgressDialog> progress;
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

/**
 * @internal Deconstructer stub needed to delete unique_ptrs with type Private
 */
WbMapAccountDialog::~WbMapAccountDialog()
{
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
  //! @Todo C++14: this should be make_unique
  d2->progress = std::unique_ptr<QProgressDialog>(new QProgressDialog(this));
  d2->progress->setModal(true);
  d2->progress->setCancelButton(nullptr);
  d2->progress->setMinimum(0);
  d2->progress->setMaximum(0);
  d2->progress->setMinimumDuration(0);

  switch (id) {
    case BACKENDS_PAGE: {
        backendsList->clear();

        d2->progress->setWindowTitle(i18n("Loading Weboob backend..."));
        d2->progress->setLabelText(i18n("Getting list of backends."));

        QCoreApplication::processEvents();

        QFuture<QList<WeboobExt::Backend> > future = QtConcurrent::run(weboob, &WeboobExt::getBackends);
        d2->watcher2.setFuture(future);

        break;
      }
    case ACCOUNTS_PAGE: {
        accountsList->clear();
        d2->progress->setWindowTitle(i18n("Connecting to bank..."));
        d2->progress->setLabelText(i18n("Getting list of accounts from your bank."));

        QCoreApplication::processEvents();

        QFuture<QList<WeboobExt::Account> > future = QtConcurrent::run(weboob, &WeboobExt::getAccounts, backendsList->currentItem()->text(0));
        d->watcher.setFuture(future);
        button(QWizard::BackButton)->setEnabled(false);
        accountsList->setEnabled(false);

        break;
      }

    default:
      // I do not know if this can actually happen. But to be safe:
      d2->progress.reset();
  }
}

void WbMapAccountDialog::gotBackends()
{
  QList<WeboobExt::Backend> backends = d2->watcher2.result();

  for (QListIterator<WeboobExt::Backend> it(backends); it.hasNext();) {
    WeboobExt::Backend backend = it.next();
    QStringList headers;
    headers << backend.name << backend.module;
    backendsList->addTopLevelItem(new QTreeWidgetItem(headers));
  }

  d2->progress.reset();
}

void WbMapAccountDialog::gotAccounts()
{
  QList<WeboobExt::Account> accounts = d->watcher.result();

  for (QListIterator<WeboobExt::Account> it(accounts); it.hasNext();) {
    WeboobExt::Account account = it.next();
    QStringList headers;
    headers << account.id << account.name << account.balance.formatMoney(QString(), 2);
    accountsList->addTopLevelItem(new QTreeWidgetItem(headers));
  }

  d->progress.reset();

  button(QWizard::BackButton)->setEnabled(true);
  accountsList->setEnabled(true);
}
