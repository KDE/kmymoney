/*
    SPDX-FileCopyrightText: 2009 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kbalancewarning.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"

class KBalanceWarning::Private
{
public:
    QString dontShowAgain() const {
        return "BalanceWarning";
    }
    QMap<QString, bool> m_deselectedAccounts;
};

KBalanceWarning::KBalanceWarning(QObject* parent) :
    QObject(parent),
    d(new Private)
{
    KMessageBox::enableMessage(d->dontShowAgain());
}

KBalanceWarning::~KBalanceWarning()
{
    delete d;
}

void KBalanceWarning::slotShowMessage(QWidget* parent, const MyMoneyAccount& account, const QString& msg)
{
    if (d->m_deselectedAccounts.find(account.id()) == d->m_deselectedAccounts.end()) {
        KMessageBox::information(parent, msg, QString(), d->dontShowAgain());
        if (!KMessageBox::shouldBeShownContinue(d->dontShowAgain())) {
            d->m_deselectedAccounts[account.id()] = true;
            KMessageBox::enableMessage(d->dontShowAgain());
        }
    }
}
