/*
    SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

#include "ui_ksettingsgpg.h"

#include <kgpgfile.h>

#define RECOVER_KEY_ID      "0xD2B08440"
#define RECOVER_KEY_ID_FULL "59B0F826D2B08440"

class KSettingsGpgPrivate
{
  Q_DISABLE_COPY(KSettingsGpgPrivate)

public:
  KSettingsGpgPrivate() :
    ui(new Ui::KSettingsGpg),
    m_checkCount(0),
    m_needCheckList(true),
    m_listOk(false)
  {
  }

  ~KSettingsGpgPrivate()
  {
    delete ui;
  }

  Ui::KSettingsGpg *ui;
  int               m_checkCount;
  bool              m_needCheckList;
  bool              m_listOk;
};

KSettingsGpg::KSettingsGpg(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KSettingsGpgPrivate)
{
  Q_D(KSettingsGpg);
  d->ui->setupUi(this);
  setEnabled(KGPGFile::GPGAvailable());

  // don't show the widget in which the master key is actually kept
  d->ui->kcfg_GpgRecipient->hide();

  connect(d->ui->kcfg_WriteDataEncrypted, &QAbstractButton::toggled, this, &KSettingsGpg::slotStatusChanged);
  connect(d->ui->m_masterKeyCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KSettingsGpg::*)(int)>(&KSettingsGpg::slotIdChanged));
  connect(d->ui->kcfg_GpgRecipientList, &KEditListWidget::changed, this, static_cast<void (KSettingsGpg::*)()>(&KSettingsGpg::slotIdChanged));
  connect(d->ui->kcfg_GpgRecipientList, &KEditListWidget::added, this, &KSettingsGpg::slotKeyListChanged);
  connect(d->ui->kcfg_GpgRecipientList, &KEditListWidget::removed, this, &KSettingsGpg::slotKeyListChanged);

  // Initial state setup
  slotStatusChanged(d->ui->kcfg_WriteDataEncrypted->isChecked());
}

KSettingsGpg::~KSettingsGpg()
{
  Q_D(KSettingsGpg);
  delete d;
}

void KSettingsGpg::slotKeyListChanged()
{
  Q_D(KSettingsGpg);
  d->m_needCheckList = true;
  slotIdChanged();
}

void KSettingsGpg::slotIdChanged()
{
  Q_D(KSettingsGpg);
  // this looks a bit awkward. Here's why: KGPGFile::keyAvailable() starts
  // an external task and processes UI events while it waits for the external
  // process to finish. Thus, the first time we get here, the external process
  // is started and the user may press a second key which calls this routine
  // again.
  //
  // The second invocation is counted, but the check is not started until the
  // first one finishes. Once the external process finishes, we check if we
  // were called in the meantime and restart the check.
  if (++d->m_checkCount == 1) {
    while (1) {
      // first we check the current edit field if filled
      bool keysOk = true;
      if (!d->ui->kcfg_GpgRecipientList->currentText().isEmpty()) {
        keysOk = KGPGFile::keyAvailable(d->ui->kcfg_GpgRecipientList->currentText());
      }

      // if it is available, then scan the current list if we need to
      if (keysOk) {
        if (d->m_needCheckList) {
          QStringList keys = d->ui->kcfg_GpgRecipientList->items();
          QStringList::const_iterator it_s;
          for (it_s = keys.constBegin(); keysOk && it_s != keys.constEnd(); ++it_s) {
            if (!KGPGFile::keyAvailable(*it_s))
              keysOk = false;
          }
          d->m_listOk = keysOk;
          d->m_needCheckList = false;

        } else {
          keysOk = d->m_listOk;
        }
      }

      // did we receive some more requests to check?
      if (d->m_checkCount > 1) {
        d->m_checkCount = 1;
        continue;
      }

      // if we have a master key, we store it in the hidden widget
      if (d->ui->m_masterKeyCombo->currentIndex() != 0) {
        QRegExp keyExp(".* \\((.*)\\)");
        if (keyExp.indexIn(d->ui->m_masterKeyCombo->currentText()) != -1) {
          d->ui->kcfg_GpgRecipient->setText(keyExp.cap(1));
        }
      }

      d->ui->m_userKeysFound->setState(static_cast<KLed::State>(keysOk && (d->ui->kcfg_GpgRecipientList->items().count() != 0) ? KLed::On : KLed::Off));
      break;
    }

    --d->m_checkCount;
  }
}

void KSettingsGpg::slotIdChanged(int)
{
  slotIdChanged();
}

void KSettingsGpg::showEvent(QShowEvent * event)
{
  Q_D(KSettingsGpg);
  QString masterKey;

  if (d->ui->m_masterKeyCombo->currentIndex() != 0) {
    QRegExp keyExp(".* \\((.*)\\)");
    if (keyExp.indexIn(d->ui->m_masterKeyCombo->currentText()) != -1) {
      masterKey = keyExp.cap(1);
    }
  } else
    masterKey = d->ui->kcfg_GpgRecipient->text();

  // fill the secret key combobox with a fresh list
  d->ui->m_masterKeyCombo->clear();
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
      d->ui->m_masterKeyCombo->addItem(name);
      if (name.contains(masterKey))
        d->ui->m_masterKeyCombo->setCurrentItem(name);
    }
  }

  // if we don't have at least one secret key, we turn off encryption
  if (keyList.isEmpty()) {
    setEnabled(false);
    d->ui->kcfg_WriteDataEncrypted->setChecked(false);
  }

  slotStatusChanged(d->ui->kcfg_WriteDataEncrypted->isChecked());
  QWidget::showEvent(event);
}

void KSettingsGpg::slotStatusChanged(bool state)
{
  Q_D(KSettingsGpg);
  static bool oncePerSession = true;
  if (state && !KGPGFile::GPGAvailable())
    state = false;

  if ((state == true) && (oncePerSession == true) && isVisible()) {
    KMessageBox::information(this, QString("<qt>%1</qt>").arg(i18n("<p>You have turned on the GPG encryption support. This means, that new files will be stored encrypted.</p><p>Existing files will not be encrypted automatically.  To achieve encryption of existing files, please use the <b>File/Save as...</b> feature and store the file under a different name.<br/>Once confident with the result, feel free to delete the old file and rename the encrypted one to the old name.</p>")), i18n("GPG encryption activated"), "GpgEncryptionActivated");
    oncePerSession = false;
  }

  d->ui->m_recoverKeyFound->setEnabled(state);
  d->ui->kcfg_EncryptRecover->setEnabled(state);
  d->ui->m_masterKeyCombo->setEnabled(state);
  d->ui->kcfg_GpgRecipientList->setEnabled(state);

  if (state) {
    d->ui->m_recoverKeyFound->setState((KLed::State)(KGPGFile::keyAvailable(RECOVER_KEY_ID) ? KLed::On : KLed::Off));
    d->ui->kcfg_EncryptRecover->setEnabled(d->ui->m_recoverKeyFound->state() == KLed::On);
    slotIdChanged();

  } else {
    d->ui->m_recoverKeyFound->setState(KLed::Off);
    d->ui->m_userKeysFound->setState(KLed::Off);
  }
}
