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

#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif


// ----------------------------------------------------------------------------
// System Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qdatetimeedit.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kled.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "konlinebankingstatus.h"
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kmymoney/mymoneyaccount.h>
#include <libofx/libofx.h>
#include "mymoneyofxconnector.h"

KOnlineBankingStatus::KOnlineBankingStatus(const MyMoneyAccount& acc, QWidget *parent, const char *name) :
  KOnlineBankingStatusDecl(parent,name),
  m_appId(0)
{
  m_ledOnlineStatus->off();

  // Set up online banking settings if applicable
  MyMoneyKeyValueContainer settings = acc.onlineBankingSettings();
  m_textOnlineStatus->setText(i18n("Enabled & configured"));
  m_ledOnlineStatus->on();

  QString account = settings.value("accountid");
  QString bank = settings.value("bankname");
  QString bankid = QString("%1 %2").arg(settings.value("bankid")).arg(settings.value("branchid"));
  if ( bankid.length() > 1 )
    bank += QString(" (%1)").arg(bankid);
  m_textBank->setText(bank);
  m_textOnlineAccount->setText(account);

  m_appId = new OfxAppVersion(m_applicationCombo, settings.value("appId"));
  m_headerVersion = new OfxHeaderVersion(m_headerVersionCombo, settings.value("kmmofx-headerVersion"));

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
  m_specificDate->setMaxValue(QDate::currentDate());
  m_payeeidRB->setChecked(settings.value("kmmofx-preferPayeeid").isEmpty() || settings.value("kmmofx-preferPayeeid").toInt() != 0);
  m_nameRB->setChecked(!settings.value("kmmofx-preferName").isEmpty() && settings.value("kmmofx-preferName").toInt() != 0);
}

KOnlineBankingStatus::~KOnlineBankingStatus()
{
  delete m_appId;
}

const QString& KOnlineBankingStatus::appId(void) const
{
  if(m_appId)
    return m_appId->appId();
  return QString::null;
}

QString KOnlineBankingStatus::headerVersion(void) const
{
  if(m_headerVersion)
    return m_headerVersion->headerVersion();
  return QString::null;
}

#include "konlinebankingstatus.moc"

