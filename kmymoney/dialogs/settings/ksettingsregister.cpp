/*
 * SPDX-FileCopyrightText: 2005-2007 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ksettingsregister.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsregister.h"

KSettingsRegister::KSettingsRegister(QWidget* parent) :
  QWidget(parent),
  ui(new Ui::KSettingsRegister)
{
  ui->setupUi(this);
  // hide the internally used text fields
  ui->kcfg_sortNormalView->hide();
  ui->kcfg_sortReconcileView->hide();
  ui->kcfg_sortSearchView->hide();

  // setup connections, so that the sort options get loaded once the edit fields are filled
  connect(ui->kcfg_sortNormalView, &QLineEdit::textChanged, this, &KSettingsRegister::slotLoadNormal);
  connect(ui->kcfg_sortReconcileView, &QLineEdit::textChanged, this, &KSettingsRegister::slotLoadReconcile);
  connect(ui->kcfg_sortSearchView, &QLineEdit::textChanged, this, &KSettingsRegister::slotLoadSearch);

  // setup connections, so that changes by the user are forwarded to the (hidden) edit fields
  connect(ui->m_sortNormalView, &TransactionSortOption::settingsChanged, ui->kcfg_sortNormalView, &KLineEdit::setText);
  connect(ui->m_sortReconcileView, &TransactionSortOption::settingsChanged, ui->kcfg_sortReconcileView, &KLineEdit::setText);
  connect(ui->m_sortSearchView, &TransactionSortOption::settingsChanged, ui->kcfg_sortSearchView, &KLineEdit::setText);
}

KSettingsRegister::~KSettingsRegister()
{
  delete ui;
}

void KSettingsRegister::slotLoadNormal(const QString& text)
{
  // only need this once
  disconnect(ui->kcfg_sortNormalView, &QLineEdit::textChanged, this, &KSettingsRegister::slotLoadNormal);
  ui->m_sortNormalView->setSettings(text);
}

void KSettingsRegister::slotLoadReconcile(const QString& text)
{
  // only need this once
  disconnect(ui->kcfg_sortReconcileView, &QLineEdit::textChanged, this, &KSettingsRegister::slotLoadReconcile);
  ui->m_sortReconcileView->setSettings(text);
}

void KSettingsRegister::slotLoadSearch(const QString& text)
{
  // only need this once
  disconnect(ui->kcfg_sortSearchView, &QLineEdit::textChanged, this, &KSettingsRegister::slotLoadSearch);
  ui->m_sortSearchView->setSettings(text);
}
