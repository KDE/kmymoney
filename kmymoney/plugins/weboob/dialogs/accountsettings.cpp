/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
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

#include "accountsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_accountsettings.h"

#include "mymoneykeyvaluecontainer.h"

class AccountSettingsPrivate
{
public:
  AccountSettingsPrivate() :
    ui(new Ui::AccountSettings)
  {
  }

  ~AccountSettingsPrivate()
  {
    delete ui;
  }
  Ui::AccountSettings *ui;
};

AccountSettings::AccountSettings(const MyMoneyAccount& /*acc*/, QWidget* parent) :
    QWidget(parent),
    d_ptr(new AccountSettingsPrivate)
{
  Q_D(AccountSettings);
  d->ui->setupUi(this);
}

AccountSettings::~AccountSettings()
{
  Q_D(AccountSettings);
  delete d;
}

void AccountSettings::loadUi(const MyMoneyKeyValueContainer& kvp)
{
  Q_D(AccountSettings);
  d->ui->id->setText(kvp.value("wb-id"));
  d->ui->backend->setText(kvp.value("wb-backend"));
  d->ui->max_history->setText(kvp.value("wb-max"));
}

void AccountSettings::loadKvp(MyMoneyKeyValueContainer& kvp)
{
  Q_D(AccountSettings);
  kvp.setValue("wb-id", d->ui->id->text());
  kvp.setValue("wb-backend", d->ui->backend->text());
  kvp.setValue("wb-max", d->ui->max_history->text());
}
