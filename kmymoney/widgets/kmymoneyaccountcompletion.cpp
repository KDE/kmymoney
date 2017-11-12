/***************************************************************************
                          kmymoneyaccountcompletion.cpp  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyaccountcompletion.h"
#include "kmymoneycompletion_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QLayout>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyaccountselector.h"

KMyMoneyAccountCompletion::KMyMoneyAccountCompletion(QWidget *parent) :
    KMyMoneyCompletion(parent)
{
  Q_D(KMyMoneyCompletion);
  delete d->m_selector;
  d->m_selector = new KMyMoneyAccountSelector(this, 0, false);
  d->m_selector->listView()->setFocusProxy(parent);
  layout()->addWidget(d->m_selector);

#ifndef KMM_DESIGNER
  // Default is to show all accounts
  // FIXME We should leave this also to the caller
  AccountSet set;
  set.addAccountGroup(eMyMoney::Account::Asset);
  set.addAccountGroup(eMyMoney::Account::Liability);
  set.addAccountGroup(eMyMoney::Account::Income);
  set.addAccountGroup(eMyMoney::Account::Expense);
  set.load(selector());
#endif

  connectSignals(d->m_selector, d->m_selector->listView());
}

KMyMoneyAccountCompletion::~KMyMoneyAccountCompletion()
{
}

QStringList KMyMoneyAccountCompletion::accountList(const QList<eMyMoney::Account>& list = QList<eMyMoney::Account>()) const
{
  return selector()->accountList(list);
}

QStringList KMyMoneyAccountCompletion::accountList() const
{
  return accountList(QList<eMyMoney::Account>());
}

KMyMoneyAccountSelector* KMyMoneyAccountCompletion::selector() const
{
  Q_D(const KMyMoneyCompletion);
//  return nullptr;
      return dynamic_cast<KMyMoneyAccountSelector*>(d->m_selector);
  }

void KMyMoneyAccountCompletion::slotMakeCompletion(const QString& txt)
{
  Q_D(KMyMoneyCompletion);
  // if(txt.isEmpty() || txt.length() == 0)
  //  return;

  auto cnt = 0;
  if (txt.contains(MyMoneyFile::AccountSeperator) == 0) {
    d->m_lastCompletion = QRegExp(QRegExp::escape(txt), Qt::CaseInsensitive);
    cnt = selector()->slotMakeCompletion(txt);
  } else {
    QStringList parts = txt.split(MyMoneyFile::AccountSeperator, QString::SkipEmptyParts);
    QString pattern("^");
    QStringList::iterator it;
    for (it = parts.begin(); it != parts.end(); ++it) {
      if (pattern.length() > 1)
        pattern += MyMoneyFile::AccountSeperator;
      pattern += QRegExp::escape(QString(*it).trimmed()) + ".*";
    }
    pattern += '$';
    d->m_lastCompletion = QRegExp(pattern, Qt::CaseInsensitive);
    cnt = selector()->slotMakeCompletion(d->m_lastCompletion);
    // if we don't have a match, we try it again, but this time
    // we add a wildcard for the top level
    if (cnt == 0) {
      pattern = pattern.insert(1, QString(".*") + MyMoneyFile::AccountSeperator);
      d->m_lastCompletion = QRegExp(pattern, Qt::CaseInsensitive);
      cnt = selector()->slotMakeCompletion(d->m_lastCompletion);
    }
  }

  if (d->m_parent && d->m_parent->isVisible() && !isVisible() && cnt)
    show(false);
  else {
    if (cnt != 0) {
      adjustSize();
    } else {
      hide();
    }
  }
}
