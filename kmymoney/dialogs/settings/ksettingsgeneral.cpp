/*
 * Copyright 2005-2008  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ksettingsgeneral.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsgeneral.h"

#include "kmymoneydateinput.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"

class KSettingsGeneralPrivate
{
  Q_DISABLE_COPY(KSettingsGeneralPrivate)

public:
  KSettingsGeneralPrivate() :
    ui(new Ui::KSettingsGeneral),
    initialHideZeroBalanceEquities(false)
  {
  }

  ~KSettingsGeneralPrivate()
  {
    delete ui;
  }

  Ui::KSettingsGeneral *ui;
  bool initialHideZeroBalanceEquities;
};

KSettingsGeneral::KSettingsGeneral(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KSettingsGeneralPrivate)
{
  Q_D(KSettingsGeneral);
  d->ui->setupUi(this);
  // hide the internally used date field
  d->ui->kcfg_StartDate->hide();

  // setup connections, so that the sort options get loaded once the edit fields are filled
  connect(d->ui->kcfg_StartDate, &QDateTimeEdit::dateChanged, this, &KSettingsGeneral::slotLoadStartDate);

  // setup connections, so that changes by the user are forwarded to the (hidden) edit fields
  connect(d->ui->m_startDateEdit, &KMyMoneyDateInput::dateChanged, d->ui->kcfg_StartDate, &QDateTimeEdit::setDate);

  connect(d->ui->choosePath, &QAbstractButton::pressed, this, &KSettingsGeneral::slotChooseLogPath);
  d->initialHideZeroBalanceEquities = d->ui->kcfg_HideZeroBalanceEquities->isChecked();
}

KSettingsGeneral::~KSettingsGeneral()
{
  Q_D(KSettingsGeneral);
  delete d;
}

void KSettingsGeneral::slotChooseLogPath()
{
  Q_D(KSettingsGeneral);
  QString filePath = QFileDialog::getExistingDirectory(this, i18n("Choose file path"), QDir::homePath());
  d->ui->kcfg_logPath->setText(filePath);
  slotUpdateLogTypes();
}

void KSettingsGeneral::slotLoadStartDate(const QDate&)
{
  Q_D(KSettingsGeneral);
  // only need this once
  disconnect(d->ui->kcfg_StartDate, &QDateTimeEdit::dateChanged, this, &KSettingsGeneral::slotLoadStartDate);
  d->ui->m_startDateEdit->setDate(d->ui->kcfg_StartDate->date());
}

void KSettingsGeneral::slotUpdateLogTypes()
{
  Q_D(KSettingsGeneral);
  bool enable = d->ui->kcfg_logPath->text().isEmpty() ? false : true;
  d->ui->kcfg_logImportedStatements->setEnabled(enable);
  d->ui->kcfg_logOfxTransactions->setEnabled(enable);
  if (!enable)
  {
    d->ui->kcfg_logImportedStatements->setChecked(enable);
    d->ui->kcfg_logOfxTransactions->setChecked(enable);
  }
}

void KSettingsGeneral::showEvent(QShowEvent *event)
{
  Q_UNUSED(event)
  QWidget::showEvent(event);
  slotUpdateLogTypes();
}

void KSettingsGeneral::slotUpdateEquitiesVisibility()
{
/// @todo cleanup
/// @todo port to new model code
//   Q_D(KSettingsGeneral);
//   if (d->initialHideZeroBalanceEquities == d->ui->kcfg_HideZeroBalanceEquities->isChecked())      // setting hasn't been changed, so return
//     return;
//   d->initialHideZeroBalanceEquities = d->ui->kcfg_HideZeroBalanceEquities->isChecked();
//   AccountsModel* accountsModel = Models::instance()->accountsModel();                   // items' model for accounts' page
//   InstitutionsModel* institutionsModel = Models::instance()->institutionsModel();       // items' model for institutions' page
//   auto file = MyMoneyFile::instance();
//   QList<MyMoneyAccount> accountsList;
//   file->accountList(accountsList);
//
//   foreach (const auto account, accountsList) {
//     if (account.isInvest() && account.balance().isZero()) {                             // search only for zero balance stocks
//       if (d->initialHideZeroBalanceEquities) {
//         accountsModel->slotObjectRemoved(eMyMoney::File::Object::Account, account.id());     // remove item from accounts' page
//         institutionsModel->slotObjectRemoved(eMyMoney::File::Object::Account, account.id()); // remove item from institutions' page
//       } else {
//         accountsModel->slotObjectAdded(eMyMoney::File::Object::Account, account.id());     // add item to accounts' page
//         institutionsModel->slotObjectAdded(eMyMoney::File::Object::Account, account.id()); // add item to institutions' page
//       }
//     }
//   }
}
