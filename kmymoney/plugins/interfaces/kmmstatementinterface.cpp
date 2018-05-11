/***************************************************************************
                          kmmstatementinterface.cpp
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include "kmmstatementinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneystatementreader.h"

KMyMoneyPlugin::KMMStatementInterface::KMMStatementInterface(QObject* parent, const char* name) :
    StatementInterface(parent, name)
{
}

QStringList KMyMoneyPlugin::KMMStatementInterface::import(const MyMoneyStatement& s, bool silent)
{
  qDebug("KMyMoneyPlugin::KMMStatementInterface::import start");
  return MyMoneyStatementReader::importStatement(s, silent);
}

MyMoneyAccount KMyMoneyPlugin::KMMStatementInterface::account(const QString& key, const QString& value) const
{
  QList<MyMoneyAccount> list;
  QList<MyMoneyAccount>::const_iterator it_a;
  MyMoneyFile::instance()->accountList(list);
  QString accId;
  for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
    // search in the account's kvp container
    const auto& accountKvpValue = (*it_a).value(key);
    // search in the account's online settings kvp container
    const auto& onlineSettingsKvpValue = (*it_a).onlineBankingSettings().value(key);
    if (accountKvpValue.contains(value) || onlineSettingsKvpValue.contains(value)) {
      if(accId.isEmpty()) {
        accId = (*it_a).id();
      }
    }
    if (accountKvpValue == value || onlineSettingsKvpValue == value) {
      accId = (*it_a).id();
      break;
    }
  }

  // return the account found or an empty element
  return MyMoneyFile::instance()->account(accId);
}

void KMyMoneyPlugin::KMMStatementInterface::setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const
{
  MyMoneyFileTransaction ft;
  try {
    auto oAcc = MyMoneyFile::instance()->account(acc.id());
    oAcc.setOnlineBankingSettings(kvps);
    MyMoneyFile::instance()->modifyAccount(oAcc);
    ft.commit();

  } catch (const MyMoneyException) {
    qDebug("Unable to setup online parameters for account '%s'", qPrintable(acc.name()));
//    KMessageBox::detailedSorry(0, i18n("Unable to setup online parameters for account '%1'", acc.name()), e.what());
  }
}
