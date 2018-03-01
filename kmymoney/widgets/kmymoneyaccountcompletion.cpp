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
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config-kmymoney.h>

#include "kmymoneyaccountcompletion.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QRegExp>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>

kMyMoneyAccountCompletion::kMyMoneyAccountCompletion(QWidget *parent) :
    kMyMoneyCompletion(parent)
{
  delete m_selector;
  m_selector = new kMyMoneyAccountSelector(this, 0, false);
  m_selector->listView()->setFocusProxy(parent);

#ifndef KMM_DESIGNER
  // Default is to show all accounts
  // FIXME We should leave this also to the caller
  AccountSet set;
  set.addAccountGroup(MyMoneyAccount::Asset);
  set.addAccountGroup(MyMoneyAccount::Liability);
  set.addAccountGroup(MyMoneyAccount::Income);
  set.addAccountGroup(MyMoneyAccount::Expense);
  set.load(selector());
#endif

  connectSignals(m_selector, m_selector->listView());
}

kMyMoneyAccountCompletion::~kMyMoneyAccountCompletion()
{
}

void kMyMoneyAccountCompletion::slotMakeCompletion(const QString& txt)
{
  // if(txt.isEmpty() || txt.length() == 0)
  //  return;

  int cnt = 0;
  if (txt.contains(MyMoneyFile::AccountSeparator) == 0) {
    m_lastCompletion = QRegExp(QRegExp::escape(txt), Qt::CaseInsensitive);
    cnt = selector()->slotMakeCompletion(txt);
  } else {
    QStringList parts = txt.split(MyMoneyFile::AccountSeparator, QString::SkipEmptyParts);
    QString pattern("^");
    QStringList::iterator it;
    for (it = parts.begin(); it != parts.end(); ++it) {
      if (pattern.length() > 1)
        pattern += MyMoneyFile::AccountSeparator;
      pattern += QRegExp::escape(QString(*it).trimmed()) + ".*";
    }
    pattern += '$';
    m_lastCompletion = QRegExp(pattern, Qt::CaseInsensitive);
    cnt = selector()->slotMakeCompletion(m_lastCompletion);
    // if we don't have a match, we try it again, but this time
    // we add a wildcard for the top level
    if (cnt == 0) {
      pattern = pattern.insert(1, QString(".*") + MyMoneyFile::AccountSeparator);
      m_lastCompletion = QRegExp(pattern, Qt::CaseInsensitive);
      cnt = selector()->slotMakeCompletion(m_lastCompletion);
    }
  }

  if (m_parent && m_parent->isVisible() && !isVisible() && cnt)
    show(false);
  else {
    if (cnt != 0) {
      adjustSize();
    } else {
      hide();
    }
  }
}
