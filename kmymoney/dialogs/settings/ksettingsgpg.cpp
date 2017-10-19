/***************************************************************************
                             ksettingsgpg.cpp
                             --------------------
    copyright            : (C) 2005, 2008 by Thomas Baumgart
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

#include "ksettingsgpg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QRegExp>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLed>
#include <KLineEdit>
#include <KComboBox>
#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <kgpgfile.h>

#define RECOVER_KEY_ID      "0xD2B08440"
#define RECOVER_KEY_ID_FULL "59B0F826D2B08440"

KSettingsGpg::KSettingsGpg(QWidget* parent) :
    KSettingsGpgDecl(parent),
    m_checkCount(0),
    m_needCheckList(true),
    m_listOk(false)
{
  setEnabled(KGPGFile::GPGAvailable());

  // don't show the widget in which the master key is actually kept
  kcfg_GpgRecipient->hide();

  connect(kcfg_WriteDataEncrypted, SIGNAL(toggled(bool)), this, SLOT(slotStatusChanged(bool)));
  connect(m_masterKeyCombo, SIGNAL(activated(int)), this, SLOT(slotIdChanged()));
  connect(kcfg_GpgRecipientList, SIGNAL(changed()), this, SLOT(slotIdChanged()));
  connect(kcfg_GpgRecipientList, SIGNAL(added(QString)), this, SLOT(slotKeyListChanged()));
  connect(kcfg_GpgRecipientList, SIGNAL(removed(QString)), this, SLOT(slotKeyListChanged()));

  // Initial state setup
  slotStatusChanged(kcfg_WriteDataEncrypted->isChecked());
}

KSettingsGpg::~KSettingsGpg()
{
}

void KSettingsGpg::slotKeyListChanged()
{
  m_needCheckList = true;
  slotIdChanged();
}

void KSettingsGpg::slotIdChanged()
{
  // this looks a bit awkward. Here's why: KGPGFile::keyAvailable() starts
  // an external task and processes UI events while it waits for the external
  // process to finish. Thus, the first time we get here, the external process
  // is started and the user may press a second key which calls this routine
  // again.
  //
  // The second invocation is counted, but the check is not started until the
  // first one finishes. Once the external process finishes, we check if we
  // were called in the meantime and restart the check.
  if (++m_checkCount == 1) {
    while (1) {
      // first we check the current edit field if filled
      bool keysOk = true;
      if (!kcfg_GpgRecipientList->currentText().isEmpty()) {
        keysOk = KGPGFile::keyAvailable(kcfg_GpgRecipientList->currentText());
      }

      // if it is available, then scan the current list if we need to
      if (keysOk) {
        if (m_needCheckList) {
          QStringList keys = kcfg_GpgRecipientList->items();
          QStringList::const_iterator it_s;
          for (it_s = keys.constBegin(); keysOk && it_s != keys.constEnd(); ++it_s) {
            if (!KGPGFile::keyAvailable(*it_s))
              keysOk = false;
          }
          m_listOk = keysOk;
          m_needCheckList = false;

        } else {
          keysOk = m_listOk;
        }
      }

      // did we receive some more requests to check?
      if (m_checkCount > 1) {
        m_checkCount = 1;
        continue;
      }

      // if we have a master key, we store it in the hidden widget
      if (m_masterKeyCombo->currentIndex() != 0) {
        QRegExp keyExp(".* \\((.*)\\)");
        if (keyExp.indexIn(m_masterKeyCombo->currentText()) != -1) {
          kcfg_GpgRecipient->setText(keyExp.cap(1));
        }
      }

      m_userKeysFound->setState(static_cast<KLed::State>(keysOk && (kcfg_GpgRecipientList->items().count() != 0) ? KLed::On : KLed::Off));
      break;
    }

    --m_checkCount;
  }
}

void KSettingsGpg::showEvent(QShowEvent * event)
{
  QString masterKey;

  if (m_masterKeyCombo->currentIndex() != 0) {
    QRegExp keyExp(".* \\((.*)\\)");
    if (keyExp.indexIn(m_masterKeyCombo->currentText()) != -1) {
      masterKey = keyExp.cap(1);
    }
  } else
    masterKey = kcfg_GpgRecipient->text();

  // fill the secret key combobox with a fresh list
  m_masterKeyCombo->clear();
  QStringList keyList;
  KGPGFile::secretKeyList(keyList);

  for (QStringList::iterator it = keyList.begin(); it != keyList.end(); ++it) {
    QStringList fields = (*it).split(':', QString::SkipEmptyParts);
    if (fields[0] != RECOVER_KEY_ID_FULL) {
      // replace parenthesis in name field with brackets
      QString name = fields[1];
      name.replace('(', "[");
      name.replace(')', "]");
      name = QString("%1 (0x%2)").arg(name).arg(fields[0]);
      m_masterKeyCombo->addItem(name);
      if (name.contains(masterKey))
        m_masterKeyCombo->setCurrentItem(name);
    }
  }

  // if we don't have at least one secret key, we turn off encryption
  if (keyList.isEmpty()) {
    setEnabled(false);
    kcfg_WriteDataEncrypted->setChecked(false);
  }

  slotStatusChanged(kcfg_WriteDataEncrypted->isChecked());
  KSettingsGpgDecl::showEvent(event);
}

void KSettingsGpg::slotStatusChanged(bool state)
{
  static bool oncePerSession = true;
  if (state && !KGPGFile::GPGAvailable())
    state = false;

  if ((state == true) && (oncePerSession == true) && isVisible()) {
    KMessageBox::information(this, QString("<qt>%1</qt>").arg(i18n("<p>You have turned on the GPG encryption support. This means, that new files will be stored encrypted.</p><p>Existing files will not be encrypted automatically.  To achieve encryption of existing files, please use the <b>File/Save as...</b> feature and store the file under a different name.<br/>Once confident with the result, feel free to delete the old file and rename the encrypted one to the old name.</p>")), i18n("GPG encryption activated"), "GpgEncryptionActivated");
    oncePerSession = false;
  }

  m_idGroup->setEnabled(state);
  kcfg_EncryptRecover->setEnabled(state);
  m_masterKeyCombo->setEnabled(state);
  kcfg_GpgRecipientList->setEnabled(state);

  if (state) {
    m_recoverKeyFound->setState((KLed::State)(KGPGFile::keyAvailable(RECOVER_KEY_ID) ? KLed::On : KLed::Off));
    kcfg_EncryptRecover->setEnabled(m_recoverKeyFound->state() == KLed::On);
    slotIdChanged();

  } else {
    m_recoverKeyFound->setState(KLed::Off);
    m_userKeysFound->setState(KLed::Off);
  }
}
