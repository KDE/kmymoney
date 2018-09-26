/*
 * Copyright 2005-2007  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
