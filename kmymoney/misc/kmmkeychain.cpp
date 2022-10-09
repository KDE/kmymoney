/*
    SPDX-FileCopyrightText: 2021 Igor Bugaev <freedbrt@gmail.com>
    SPDX-FileCopyrightText: 2022 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include <QDebug>
#include <QEventLoop>

#include "kmmkeychain.h"

KMMKeychain::KMMKeychain(QObject* parent)
    : QObject(parent)
    , m_readCredentialJob(QLatin1String("org.kde.kmymoney"))
    , m_writeCredentialJob(QLatin1String("org.kde.kmymoney"))
    , m_deleteCredentialJob(QLatin1String("org.kde.kmymoney"))
{
    m_readCredentialJob.setAutoDelete(false);
    m_writeCredentialJob.setAutoDelete(false);
    m_deleteCredentialJob.setAutoDelete(false);
}

void KMMKeychain::readKey(const QString& key)
{
    m_readCredentialJob.setKey(key);

    QObject::connect(&m_readCredentialJob, &QKeychain::ReadPasswordJob::finished, [=]() {
        if (m_readCredentialJob.error()) {
            auto errorMessage = QString("%1/%2 key read failed: %2")
                                    .arg(m_readCredentialJob.service())
                                    .arg(m_readCredentialJob.key())
                                    .arg(qPrintable(m_readCredentialJob.errorString()));

            qDebug() << errorMessage;
            Q_EMIT error(errorMessage);
        } else {
            qDebug() << QString("%1/%2 key read succeeded").arg(m_readCredentialJob.service()).arg(m_readCredentialJob.key());
            Q_EMIT keyRestored(key, m_readCredentialJob.textData());
        }
    });

    m_readCredentialJob.start();
}

QString KMMKeychain::readKeySynchronous(const QString& key)
{
    QString value = QString("");

    QEventLoop loop;
    connect(this, &KMMKeychain::keyRestored, [&](const QString, const QString textData) {
        value = textData;
    });
    connect(this, &KMMKeychain::error, &loop, &QEventLoop::quit);
    connect(this, &KMMKeychain::keyRestored, &loop, &QEventLoop::quit);

    readKey(key);
    loop.exec();

    return value;
}

void KMMKeychain::writeKey(const QString& key, const QString& value)
{
    m_writeCredentialJob.setKey(key);

    QObject::connect(&m_writeCredentialJob, &QKeychain::WritePasswordJob::finished, [=]() {
        if (m_writeCredentialJob.error()) {
            auto errorMessage = QString("%1/%2 key write failed: %2")
                                    .arg(m_writeCredentialJob.service())
                                    .arg(m_writeCredentialJob.key())
                                    .arg(qPrintable(m_writeCredentialJob.errorString()));

            qDebug() << errorMessage;
            Q_EMIT error(errorMessage);
        } else {
            Q_EMIT keyStored(key);
        }
    });

    m_writeCredentialJob.setTextData(value);
    m_writeCredentialJob.start();
}

void KMMKeychain::deleteKey(const QString& key)
{
    m_deleteCredentialJob.setKey(key);

    QObject::connect(&m_deleteCredentialJob, &QKeychain::DeletePasswordJob::finished, [=]() {
        if (m_deleteCredentialJob.error()) {
            auto errorMessage = QString("%1/%2 key delete failed: %2")
                                    .arg(m_deleteCredentialJob.service())
                                    .arg(m_deleteCredentialJob.key())
                                    .arg(qPrintable(m_deleteCredentialJob.errorString()));

            qDebug() << errorMessage;
            Q_EMIT error(errorMessage);
        } else {
            Q_EMIT keyDeleted(key);
        }
    });

    m_deleteCredentialJob.start();
}
