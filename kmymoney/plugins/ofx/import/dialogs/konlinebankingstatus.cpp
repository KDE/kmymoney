/***************************************************************************
                          konlinebankingstatus.cpp
                             -------------------
    begin                : Wed Apr 16 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "konlinebankingstatus.h"

// ----------------------------------------------------------------------------
// System Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KLed>
#include <KComboBox>
#include <KWallet>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"
#include "mymoneyaccount.h"
#include "mymoneyofxconnector.h"

using KWallet::Wallet;

KOnlineBankingStatus::KOnlineBankingStatus(const MyMoneyAccount& acc, QWidget *parent) :
    KOnlineBankingStatusDecl(parent),
    m_appId(0)
{
  m_ledOnlineStatus->off();

  m_preferredPayee->setCurrentIndex(0);
  m_uniqueTransactionId->setCurrentIndex(0);

  buttonGroupBox2->setContentsMargins(0, 0, 0, 0);

  buttonGroup2->setId(m_todayRB, 0);
  buttonGroup2->setId(m_lastUpdateRB, 1);
  buttonGroup2->setId(m_pickDateRB, 2);

  // Set up online banking settings if applicable
  MyMoneyKeyValueContainer settings = acc.onlineBankingSettings();
  m_textOnlineStatus->setText(i18n("Enabled & configured"));
  m_ledOnlineStatus->on();

  QString account = settings.value("accountid");
  QString bank = settings.value("bankname");
  QString bankid = QString("%1 %2").arg(settings.value("bankid")).arg(settings.value("branchid"));
  if (bankid.length() > 1)
    bank += QString(" (%1)").arg(bankid);
  m_textBank->setText(bank);
  m_textOnlineAccount->setText(account);

  m_appId = new OfxAppVersion(m_applicationCombo, m_applicationEdit, settings.value("appId"));
  m_headerVersion = new OfxHeaderVersion(m_headerVersionCombo, settings.value("kmmofx-headerVersion"));
  m_clientUidEdit->setText(settings.value("clientUid"));
#ifndef LIBOFX_HAVE_CLIENTUID
  // in case the installed libofx does not support clientuid
  // we disable the widget
  m_clientUidEdit->setEnabled(false);
  m_clientUidLabel->setEnabled(false);
#endif

  connect(m_applicationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(applicationSelectionChanged()));
  m_headerVersionEdit->hide();

  int numDays = 60;
  QString snumDays = settings.value("kmmofx-numRequestDays");
  if (!snumDays.isEmpty())
    numDays = snumDays.toInt();
  m_numdaysSpin->setValue(numDays);
  m_todayRB->setChecked(settings.value("kmmofx-todayMinus").isEmpty() || settings.value("kmmofx-todayMinus").toInt() != 0);
  m_lastUpdateRB->setChecked(!settings.value("kmmofx-lastUpdate").isEmpty() && settings.value("kmmofx-lastUpdate").toInt() != 0);
  m_lastUpdateTXT->setText(acc.value("lastImportedTransactionDate"));
  m_pickDateRB->setChecked(!settings.value("kmmofx-pickDate").isEmpty() && settings.value("kmmofx-pickDate").toInt() != 0);
  QString specificDate = settings.value("kmmofx-specificDate");
  if (!specificDate.isEmpty())
    m_specificDate->setDate(QDate::fromString(specificDate));
  else
    m_specificDate->setDate(QDate::currentDate());
  m_specificDate->setMaximumDate(QDate::currentDate());
  m_preferredPayee->setCurrentIndex(settings.value("kmmofx-preferName").toInt());
  m_uniqueTransactionId->setCurrentIndex(settings.value("kmmofx-uniqueIdSource").toInt());

  const int offset = settings.value("kmmofx-timestampOffset").toInt();
  m_timestampOffsetSign->setCurrentIndex(offset < 0 ? 1 : 0);
  m_timestampOffset->setTime(QTime::fromMSecsSinceStartOfDay(qAbs(offset)*60*1000));

  m_invertAmount->setChecked(settings.value("kmmofx-invertamount").toLower() == QStringLiteral("yes"));

  QString key = OFX_PASSWORD_KEY(settings.value("url"), settings.value("uniqueId"));
  QString pwd;

  // if we don't find a password in the wallet, we use the old method
  // and retrieve it from the settings stored in the KMyMoney data storage.
  if (Wallet::keyDoesNotExist(Wallet::NetworkWallet(), Wallet::PasswordFolder(), key)) {
    pwd = settings.value("password");
  } else {
    Wallet *wallet = openSynchronousWallet();
    if (wallet) {
      wallet->setFolder(Wallet::PasswordFolder());
      wallet->readPassword(key, pwd);
    }
  }
  m_password->setText(pwd);
  m_storePassword->setChecked(!pwd.isEmpty());
}

KOnlineBankingStatus::~KOnlineBankingStatus()
{
  delete m_headerVersion;
  delete m_appId;
}

void KOnlineBankingStatus::applicationSelectionChanged()
{
  m_applicationEdit->setVisible(m_appId->appId().endsWith(':'));
}


const QString KOnlineBankingStatus::appId() const
{
  if (m_appId)
    return m_appId->appId();
  return QString();
}

QString KOnlineBankingStatus::headerVersion() const
{
  if (m_headerVersion)
    return m_headerVersion->headerVersion();
  return QString();
}
