/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "accountsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QDate>
#include <QRadioButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_accountsettings.h"

#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"

class AccountSettingsPrivate
{
    Q_DISABLE_COPY_MOVE(AccountSettingsPrivate)

public:
    AccountSettingsPrivate(const MyMoneyAccount& acc)
        : ui(new Ui::AccountSettings)
        , m_account(acc)
    {
    }

    ~AccountSettingsPrivate()
    {
        delete ui;
    }
    Ui::AccountSettings* ui;
    const MyMoneyAccount& m_account;
    QButtonGroup buttonGroup;
};

AccountSettings::AccountSettings(const MyMoneyAccount& acc, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new AccountSettingsPrivate(acc))
{
    Q_D(AccountSettings);
    d->ui->setupUi(this);

    d->buttonGroup.addButton(d->ui->m_todayRB, 0);
    d->buttonGroup.addButton(d->ui->m_lastUpdateRB, 1);
    d->buttonGroup.addButton(d->ui->m_pickDateRB, 2);
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

    int numDays = 60;
    QString snumDays = kvp.value("wb-numRequestDays");
    if (!snumDays.isEmpty())
        numDays = snumDays.toInt();
    d->ui->m_numdaysSpin->setValue(numDays);
    d->ui->m_todayRB->setChecked(kvp.value("wb-todayMinus").isEmpty() || kvp.value("wb-todayMinus").toInt() != 0);
    d->ui->m_lastUpdateRB->setChecked(!kvp.value("wb-lastUpdate").isEmpty() && kvp.value("wb-lastUpdate").toInt() != 0);
    d->ui->m_lastUpdateTXT->setText(d->m_account.value("lastImportedTransactionDate"));
    d->ui->m_pickDateRB->setChecked(!kvp.value("wb-pickDate").isEmpty() && kvp.value("wb-pickDate").toInt() != 0);
    QString specificDate = kvp.value("wb-specificDate");
    if (!specificDate.isEmpty())
        d->ui->m_specificDate->setDate(QDate::fromString(specificDate));
    else
        d->ui->m_specificDate->setDate(QDate::currentDate());
    d->ui->m_specificDate->setMaximumDate(QDate::currentDate());
}

void AccountSettings::loadKvp(MyMoneyKeyValueContainer& kvp)
{
    Q_D(AccountSettings);
    kvp.setValue("wb-id", d->ui->id->text());
    kvp.setValue("wb-backend", d->ui->backend->text());

    kvp.setValue("wb-numRequestDays", QString::number(d->ui->m_numdaysSpin->value()));
    kvp.setValue("wb-todayMinus", QString::number(d->ui->m_todayRB->isChecked()));
    kvp.setValue("wb-lastUpdate", QString::number(d->ui->m_lastUpdateRB->isChecked()));
    kvp.setValue("wb-pickDate", QString::number(d->ui->m_pickDateRB->isChecked()));
    kvp.setValue("wb-specificDate", d->ui->m_specificDate->date().toString());
}
