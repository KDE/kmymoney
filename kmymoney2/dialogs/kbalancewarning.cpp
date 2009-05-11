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

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "kbalancewarning.h"

class KBalanceWarning::Private
{
public:
  QString dontShowAgain() const { return "BalanceWarning"; }
  QMap<QString, bool> m_deselectedAccounts;
};

KBalanceWarning::KBalanceWarning(QObject* parent, const char* name) :
  QObject(parent, name),
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
  if(d->m_deselectedAccounts.find(account.id()) == d->m_deselectedAccounts.end()) {
    KMessageBox::information(parent, msg, QString::null, d->dontShowAgain());
    if(!KMessageBox::shouldBeShownContinue(d->dontShowAgain())) {
      d->m_deselectedAccounts[account.id()] = true;
      KMessageBox::enableMessage(d->dontShowAgain());
    }
  }
}


#include "kbalancewarning.moc"
