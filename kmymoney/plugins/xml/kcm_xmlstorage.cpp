/*
    SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_xmlstorage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegularExpression>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "config-kmymoney.h"
#include "gpg-recover-key.h"
#include "kgpgfile.h"
#include "kmymoneysettings.h"

XMLStorageSettingsWidget::XMLStorageSettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_checkCount(0)
    , m_needCheckList(true)
    , m_listOk(false)
{
    setupUi(this);

    // hide the message widget until it is needed
    m_messageWidget->hide();

#ifdef ENABLE_GPG
    const bool available = KGPGFile::GPGAvailable();
#else
    const bool available = false;
#endif

    m_recoverKeyList = QStringLiteral(OLD_RECOVER_KEY_IDS).split(':');
    m_recoverKeyList.append(RECOVER_KEY_ID);

    for (auto it = m_recoverKeyList.begin(); it != m_recoverKeyList.end(); ++it) {
        *it = (*it).replace(QLatin1String("0x"), QString());
    }

    setEnabled(available);
    if (!available) {
        m_messageWidget->setMessageType(KMessageWidget::Warning);
        m_messageWidget->setText(i18n("GPG installation not found or not working properly."));
        m_messageWidget->show();
    }

    // don't show the widget in which the master key is actually kept
    kcfg_GpgRecipient->hide();

    connect(kcfg_WriteDataEncrypted, &QAbstractButton::toggled, this, &XMLStorageSettingsWidget::slotStatusChanged);
    connect(m_masterKeyCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (XMLStorageSettingsWidget::*)(int)>(&XMLStorageSettingsWidget::slotIdChanged));
    connect(kcfg_GpgRecipientList, &KEditListWidget::changed, this, static_cast<void (XMLStorageSettingsWidget::*)()>(&XMLStorageSettingsWidget::slotIdChanged));
    connect(kcfg_GpgRecipientList, &KEditListWidget::added, this, &XMLStorageSettingsWidget::slotKeyListChanged);
    connect(kcfg_GpgRecipientList, &KEditListWidget::removed, this, &XMLStorageSettingsWidget::slotKeyListChanged);

    // Initial state setup
    slotStatusChanged(kcfg_WriteDataEncrypted->isChecked());
}

void XMLStorageSettingsWidget::slotKeyListChanged()
{
    m_needCheckList = true;
    slotIdChanged();
}

void XMLStorageSettingsWidget::slotIdChanged()
{
#ifdef ENABLE_GPG
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
                static const QRegularExpression keyExp(".* \\((.*)\\)");
                const auto key(keyExp.match(m_masterKeyCombo->currentText()));
                if (key.hasMatch()) {
                    kcfg_GpgRecipient->setText(key.captured(1));
                }
            }

            m_userKeysFound->setState(static_cast<KLed::State>(keysOk && (kcfg_GpgRecipientList->items().count() != 0) ? KLed::On : KLed::Off));
            break;
        }

        --m_checkCount;
    }
#endif
}

void XMLStorageSettingsWidget::slotIdChanged(int)
{
    slotIdChanged();
}

void XMLStorageSettingsWidget::showEvent(QShowEvent * event)
{
    QString masterKey;

    if (m_masterKeyCombo->currentIndex() != -1) {
        static const QRegularExpression keyExp(".* \\((.*)\\)");
        const auto key(keyExp.match(m_masterKeyCombo->currentText()));
        if (key.hasMatch()) {
            masterKey = key.captured(1);
        }
    } else
        masterKey = kcfg_GpgRecipient->text();

    // fill the secret key combobox with a fresh list
    m_masterKeyCombo->clear();
    QStringList keyList;

#ifdef ENABLE_GPG

    // load m_masterKeyCombo with available private keys
    // but don't show the recover keys and omit duplicate
    // addresses for the same key id
    KGPGFile::secretKeyList(keyList);

    QStringList loadedKeys;
    for (QStringList::iterator it = keyList.begin(); it != keyList.end(); ++it) {
        QStringList fields = (*it).split(':', Qt::SkipEmptyParts);
        const auto keyId = fields[0];
        if (!m_recoverKeyList.contains(keyId) && !loadedKeys.contains(keyId)) {
            // replace parenthesis in name field with brackets
            QString name = fields[1];
            name.replace('(', "[");
            name.replace(')', "]");
            name = QString("%1 (0x%2)").arg(name, keyId);
            m_masterKeyCombo->addItem(name);
            loadedKeys.append(keyId);
            if (name.contains(masterKey))
                m_masterKeyCombo->setCurrentItem(name);
        }
    }
#endif

    // if we don't have at least one secret key, we turn off encryption
    if (keyList.isEmpty()) {
        setEnabled(false);
        m_messageWidget->setMessageType(KMessageWidget::Warning);
        m_messageWidget->setText(i18n("No GPG secret keys found, please run gpg[2] --gen-key or import keys into gpg"));
        m_messageWidget->animatedShow();

        kcfg_WriteDataEncrypted->setChecked(false);
    }

    slotStatusChanged(kcfg_WriteDataEncrypted->isChecked());
    QWidget::showEvent(event);
}

void XMLStorageSettingsWidget::slotStatusChanged(bool state)
{
#ifdef ENABLE_GPG
    static bool oncePerSession = true;
    if (state && !KGPGFile::GPGAvailable())
        state = false;

    if ((state == true) && (oncePerSession == true) && isVisible()) {
        KMessageBox::information(nativeParentWidget(),
                                 QString("<qt>%1</qt>")
                                     .arg(i18n("<p>You have turned on the GPG encryption support. This means, that new files will be stored "
                                               "encrypted.</p><p>Existing files will not be encrypted automatically.  To achieve encryption of existing files, "
                                               "please use the <b>File/Save as...</b> feature and store the file under a different name.<br/>Once confident "
                                               "with the result, feel free to delete the old file and rename the encrypted one to the old name.</p>")),
                                 i18n("GPG encryption activated"),
                                 "GpgEncryptionActivated");
        oncePerSession = false;
    }

    m_recoverKeyFound->setEnabled(state);
    kcfg_EncryptRecover->setEnabled(state);
    m_masterKeyCombo->setEnabled(state);
    kcfg_GpgRecipientList->setEnabled(state);

    if (state) {
        m_messageWidget->animatedHide();
        m_recoverKeyFound->setState((KLed::State)(KGPGFile::keyAvailable(RECOVER_KEY_ID) ? KLed::On : KLed::Off));
        kcfg_EncryptRecover->setEnabled(m_recoverKeyFound->state() == KLed::On);
        slotIdChanged();

    } else {
        m_recoverKeyFound->setState(KLed::Off);
        m_userKeysFound->setState(KLed::Off);
    }
#else
    Q_UNUSED(state)
    m_recoverKeyFound->setState(KLed::Off);
    m_userKeysFound->setState(KLed::Off);
#endif
}

KCMXMLStorage::KCMXMLStorage(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args)
{
    XMLStorageSettingsWidget* w = new XMLStorageSettingsWidget(this);
    addConfig(KMyMoneySettings::self(), w);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(w);
    setButtons(NoAdditionalButton);
    load();
}

KCMXMLStorage::~KCMXMLStorage()
{
}

K_PLUGIN_CLASS(KCMXMLStorage)

#include "kcm_xmlstorage.moc"
