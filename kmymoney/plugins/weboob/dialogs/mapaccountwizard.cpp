/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#include "mapaccountwizard.h"

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_mapaccountwizard.h"

#include "../interface/weboobinterface.h"
#include "../weboobexc.h"

enum {
  BACKENDS_PAGE = 0,
  ACCOUNTS_PAGE
};

class MapAccountWizardPrivate
{
public:
  MapAccountWizardPrivate(WeboobInterface* weboob) :
    ui(new Ui::MapAccountWizard),
    m_weboob(*weboob)
  {
  }

  ~MapAccountWizardPrivate()
  {
    delete ui;
  }

  Ui::MapAccountWizard *ui;
  WeboobInterface &m_weboob;
  QFutureWatcher<QList<WeboobInterface::Account> > accountsWatcher;
  QFutureWatcher<QList<WeboobInterface::Backend> > backendsWatcher;
  std::unique_ptr<QProgressDialog> progress;
};

MapAccountWizard::MapAccountWizard(QWidget *parent, WeboobInterface* weboob) :
  QWizard(parent),
  d_ptr(new MapAccountWizardPrivate(weboob))
{
  Q_D(MapAccountWizard);
  d->ui->setupUi(this);
  d->ui->addBackendButton->setVisible(false); // the button isn't connected to anything

  slotCheckNextButton();
  connect(this, &QWizard::currentIdChanged, this, &MapAccountWizard::slotCheckNextButton);
  connect(this, &QWizard::currentIdChanged, this, &MapAccountWizard::slotNewPage);
  connect(d->ui->backendsList, &QTreeWidget::itemSelectionChanged, this, &MapAccountWizard::slotCheckNextButton);
  connect(d->ui->accountsList, &QTreeWidget::itemSelectionChanged, this, &MapAccountWizard::slotCheckNextButton);
auto abc = QString();
  connect(&d->accountsWatcher, &QFutureWatcherBase::finished, this, &MapAccountWizard::slotGotAccounts);
  connect(&d->backendsWatcher, &QFutureWatcherBase::finished, this, &MapAccountWizard::slotGotBackends);
}

/**
 * @internal Deconstructer stub needed to delete unique_ptrs with type Private
 */
MapAccountWizard::~MapAccountWizard()
{
  Q_D(MapAccountWizard);
  delete d;
}

QString MapAccountWizard::currentBackend() const
{
  Q_D(const MapAccountWizard);
  return d->ui->backendsList->currentItem()->text(0);
}

QString MapAccountWizard::currentAccount() const
{
  Q_D(const MapAccountWizard);
  return d->ui->accountsList->currentItem()->text(0);
}

void MapAccountWizard::slotCheckNextButton(void)
{
  Q_D(MapAccountWizard);
  auto enableButton = false;
  switch (currentId()) {
    case BACKENDS_PAGE:
      enableButton = d->ui->backendsList->currentItem() != 0 && d->ui->backendsList->currentItem()->isSelected();
      button(QWizard::NextButton)->setEnabled(enableButton);
      break;
    case ACCOUNTS_PAGE:
      enableButton = d->ui->accountsList->currentItem() != 0 && d->ui->accountsList->currentItem()->isSelected();
      button(QWizard::FinishButton)->setEnabled(enableButton);
      break;
  }

}

void MapAccountWizard::slotNewPage(int id)
{
  Q_D(MapAccountWizard);
  d->progress = std::make_unique<QProgressDialog>(this);
  d->progress->setModal(true);
  d->progress->setCancelButton(nullptr);
  d->progress->setMinimum(0);
  d->progress->setMaximum(0);
  d->progress->setMinimumDuration(0);

  switch (id) {
    case BACKENDS_PAGE: {
        d->ui->backendsList->clear();
        d->progress->setWindowTitle(i18n("Loading Weboob backend..."));
        d->progress->setLabelText(i18n("Getting list of backends."));

        QCoreApplication::processEvents();
        d->backendsWatcher.setFuture(QtConcurrent::run(&d->m_weboob, &WeboobInterface::getBackends));

        break;
      }
    case ACCOUNTS_PAGE: {
        d->ui->accountsList->clear();
        d->progress->setWindowTitle(i18n("Connecting to bank..."));
        d->progress->setLabelText(i18n("Getting list of accounts from your bank."));

        QCoreApplication::processEvents();
        d->accountsWatcher.setFuture(QtConcurrent::run(&d->m_weboob, &WeboobInterface::getAccounts, d->ui->backendsList->currentItem()->text(0)));

        button(QWizard::BackButton)->setEnabled(false);
        d->ui->accountsList->setEnabled(false);

        break;
      }

    default:
      // I do not know if this can actually happen. But to be safe:
      d->progress.reset();
  }
}

void MapAccountWizard::slotGotBackends()
{
  Q_D(MapAccountWizard);
  const auto backends = d->backendsWatcher.result();
  for (const auto& backend : backends)
    d->ui->backendsList->addTopLevelItem(new QTreeWidgetItem(QStringList{backend.name,
                                                                  backend.module}));
  d->progress.reset();

  if (backends.isEmpty())
    KMessageBox::information(this, i18n("No backends available.\nAdd one using weboob-config-qt."));
}

void MapAccountWizard::slotGotAccounts()
{
  Q_D(MapAccountWizard);
  try {
    const auto accounts = d->accountsWatcher.result();
    for (const auto& account : accounts)
      d->ui->accountsList->addTopLevelItem(new QTreeWidgetItem(QStringList{account.id,
                                                                    account.name,
                                                                    account.balance.formatMoney(QString(), 2)}));
    d->progress.reset();

  if (accounts.isEmpty())
    KMessageBox::information(this, i18n("No accounts available.\nCheck your backend configuration in weboob-config-qt."));
  else
    button(QWizard::FinishButton)->setEnabled(true);

  } catch (const WeboobException &e) {
    d->progress.reset();
    QString msg;
    switch (e.msg()) {
      case ExceptionCode::BrowserIncorrectPassword:
        msg = i18n("Incorrect password.");
        break;
      default:
        break;
    }
    if (!msg.isEmpty())
      KMessageBox::error(this, msg);
  }

  button(QWizard::BackButton)->setEnabled(true);
  d->ui->accountsList->setEnabled(true);
}
