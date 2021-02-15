/*
 * SPDX-FileCopyrightText: 2005-2008 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
  if (!enable) {
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
