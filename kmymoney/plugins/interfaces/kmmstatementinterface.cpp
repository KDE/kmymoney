/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmstatementinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>

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

void KMyMoneyPlugin::KMMStatementInterface::resetMessages() const
{
  MyMoneyStatementReader::clearResultMessages();
}

void KMyMoneyPlugin::KMMStatementInterface::showMessages(int statementCount) const
{
  const auto resultMessages = MyMoneyStatementReader::resultMessages();
  KMessageBox::informationList(nullptr,
                                i18np("One statement has been processed with the following results:",
                                      "%1 statements have been processed with the following results:",
                                      statementCount),
                                !resultMessages.isEmpty() ?
                                    resultMessages :
                                    QStringList { i18np("No new transaction has been imported.", "No new transactions have been imported.", statementCount) },
                                i18n("Statement import statistics"));
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

  } catch (const MyMoneyException &) {
    qDebug("Unable to setup online parameters for account '%s'", qPrintable(acc.name()));
//    KMessageBox::detailedSorry(0, i18n("Unable to setup online parameters for account '%1'", acc.name()), QString::fromLatin1(e.what()));
  }
}
