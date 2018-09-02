/*
 * Copyright 2009       Thomas Baumgart <tbaumgart@kde.org>
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
