/*
    SPDX-FileCopyrightText: 2021 Igor Bugaev <freedbrt@gmail.com>
    SPDX-FileCopyrightText: 2022 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef KMMKEYCHAIN_H
#define KMMKEYCHAIN_H

#include <QObject>

#include <qt5keychain/keychain.h>

#include "kmm_keychain_export.h"

class KMM_KEYCHAIN_EXPORT KMMKeychain : public QObject
{
    Q_OBJECT
public:
    KMMKeychain(QObject* parent = nullptr);

    void readKey(const QString& key);
    QString readKeySynchronous(const QString& key);

    void writeKey(const QString& key, const QString& value);

    void deleteKey(const QString& key);

Q_SIGNALS:
    void keyStored(const QString& key);
    void keyRestored(const QString& key, const QString& value);
    void keyDeleted(const QString& key);
    void error(const QString& errorText);

private:
    QKeychain::ReadPasswordJob m_readCredentialJob;
    QKeychain::WritePasswordJob m_writeCredentialJob;
    QKeychain::DeletePasswordJob m_deleteCredentialJob;
};

#endif // KMMKEYCHAIN_H
