/***************************************************************************
                          kbalancewarning.cpp
                             -------------------
    begin                : Mon Feb  9 2009
    copyright            : (C) 2009 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kbalancewarning.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

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
