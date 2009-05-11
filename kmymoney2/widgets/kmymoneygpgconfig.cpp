/***************************************************************************
                          kmymoneyonlinequoteconfig.cpp  -  description
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#include <qcheckbox.h>
#include <q3groupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kled.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneygpgconfig.h"
#include "libkgpgfile/kgpgfile.h"

#define RECOVER_KEY_ID  "0xD2B08440"

kMyMoneyGPGConfig::kMyMoneyGPGConfig(QWidget *parent, const char *name )
  : kMyMoneyGPGConfigDecl(parent, name),
  m_checkCount(0)
{
  m_idGroup->setEnabled(KGPGFile::GPGAvailable());
  m_recoveryGroup->setEnabled(KGPGFile::keyAvailable(RECOVER_KEY_ID));

  m_userKeyFound->off();
  m_recoverKeyFound->off();

  connect(m_useEncryption, SIGNAL(toggled(bool)), this, SLOT(slotStatusChanged(bool)));
  connect(m_userId, SIGNAL(textChanged(const QString&)), this, SLOT(slotIdChanged(const QString&)));
}

void kMyMoneyGPGConfig::resetConfig(void)
{
  m_useEncryption->setChecked(m_resetUseEncryption);
  m_userId->setText(m_resetUserId);
  m_recover->setChecked(m_resetRecover);
  slotStatusChanged(m_resetUseEncryption);
  writeConfig();
}

void kMyMoneyGPGConfig::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  m_resetUseEncryption = config->readBoolEntry("WriteDataEncrypted", false);
  m_resetRecover = config->readBoolEntry("EncryptRecover", false);
  m_resetUserId = config->readEntry("GPG-Recipient");

  resetConfig();
}

void kMyMoneyGPGConfig::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  config->writeEntry("WriteDataEncrypted", m_useEncryption->isChecked());
  config->writeEntry("EncryptRecover", m_recover->isChecked());
  config->writeEntry("GPG-Recipient", m_userId->text());
}

void kMyMoneyGPGConfig::slotIdChanged(const QString& /*txt*/)
{
  // this looks a bit awkward. Here's why: KGPGFile::keyAvailable() starts
  // an external task and processes UI events while it waits for the external
  // process to finish. Thus, the first time we get here, the external process
  // is started and the user my press a second key which calls this routine
  // again.
  //
  // The second invocation is counted, but the check is not started until the
  // first one finishes. Once the external process finishes, we check if we
  // were called in the meantime and restart the check.
  if(++m_checkCount == 1) {
    while(1) {
      if(m_userId->text().trimmed().length() > 0) {
        m_userKeyFound->setState((KLed::State) (KGPGFile::keyAvailable(m_userId->text()) ? KLed::On : KLed::Off));
        if(m_checkCount > 1) {
          m_checkCount = 1;
          continue;
        }
      } else {
        m_userKeyFound->setState(KLed::Off);
      }
      break;
    }
    --m_checkCount;
  }
}

void kMyMoneyGPGConfig::slotStatusChanged(bool state)
{
  if(state) {
    m_idGroup->setEnabled(KGPGFile::GPGAvailable());
    m_recoveryGroup->setEnabled(KGPGFile::GPGAvailable());
    m_recoverKeyFound->setState((KLed::State) (KGPGFile::keyAvailable(RECOVER_KEY_ID) ? KLed::On : KLed::Off));
    if(m_userId->text().isEmpty())
      m_userKeyFound->setState(KLed::Off);
    else
      m_userKeyFound->setState((KLed::State) (KGPGFile::keyAvailable(m_userId->text()) ? KLed::On : KLed::Off));
  } else {
    m_idGroup->setEnabled(false);
    m_recoveryGroup->setEnabled(false);
    m_recoverKeyFound->setState(KLed::Off);
    m_userKeyFound->setState(KLed::Off);
  }
}

#include "kmymoneygpgconfig.moc"
