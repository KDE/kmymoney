/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
