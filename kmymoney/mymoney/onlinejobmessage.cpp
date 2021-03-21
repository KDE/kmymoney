/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejobmessage.h"

#include <QString>
#include <QDateTime>

#include "mymoneyenums.h"

class onlineJobMessagePrivate
{
public:
    onlineJobMessagePrivate() :
        m_type(eMyMoney::OnlineJob::MessageType::Log)
    {
    }

    ~onlineJobMessagePrivate()
    {
    }

    eMyMoney::OnlineJob::MessageType m_type;
    QString m_sender;
    QString m_message;
    QDateTime m_timestamp;
    QString m_senderErrorCode;
};

onlineJobMessage::onlineJobMessage() :
    d_ptr(new onlineJobMessagePrivate)
{
    Q_D(onlineJobMessage);
    d->m_type = eMyMoney::OnlineJob::MessageType::Error;
    d->m_sender = QString();
    d->m_message = QString();
    d->m_timestamp = QDateTime();
}

onlineJobMessage::onlineJobMessage(eMyMoney::OnlineJob::MessageType type,
                                   QString sender,
                                   QString message,
                                   QDateTime timestamp) :
    d_ptr(new onlineJobMessagePrivate)
{
    Q_D(onlineJobMessage);
    d->m_type = type;
    d->m_sender = sender;
    d->m_message = message;
    d->m_timestamp = timestamp;
}

onlineJobMessage::onlineJobMessage(eMyMoney::OnlineJob::MessageType type,
                                   QString sender,
                                   QString message) :
    d_ptr(new onlineJobMessagePrivate)
{
    Q_D(onlineJobMessage);
    d->m_type = type;
    d->m_sender = sender;
    d->m_message = message;
    d->m_timestamp = QDateTime::currentDateTime();
}

onlineJobMessage::onlineJobMessage(const onlineJobMessage& other) :
    d_ptr(new onlineJobMessagePrivate(*other.d_func()))
{
}

onlineJobMessage::~onlineJobMessage()
{
    Q_D(onlineJobMessage);
    delete d;
}

bool onlineJobMessage::isDebug() const
{
    Q_D(const onlineJobMessage);
    return (d->m_type == eMyMoney::OnlineJob::MessageType::Debug);
}

bool onlineJobMessage::isLog() const
{
    Q_D(const onlineJobMessage);
    return (d->m_type == eMyMoney::OnlineJob::MessageType::Log);
}

bool onlineJobMessage::isInformation() const
{
    Q_D(const onlineJobMessage);
    return (d->m_type == eMyMoney::OnlineJob::MessageType::Information);
}

bool onlineJobMessage::isWarning() const
{
    Q_D(const onlineJobMessage);
    return (d->m_type == eMyMoney::OnlineJob::MessageType::Warning);
}

bool onlineJobMessage::isError() const
{
    Q_D(const onlineJobMessage);
    return (d->m_type == eMyMoney::OnlineJob::MessageType::Error);
}

bool onlineJobMessage::isPersistant() const
{
    Q_D(const onlineJobMessage);
    return (d->m_type != eMyMoney::OnlineJob::MessageType::Debug);
}

eMyMoney::OnlineJob::MessageType onlineJobMessage::type() const
{
    Q_D(const onlineJobMessage);
    return d->m_type;
}

QString onlineJobMessage::sender() const
{
    Q_D(const onlineJobMessage);
    return d->m_sender;
}

QString onlineJobMessage::message() const
{
    Q_D(const onlineJobMessage);
    return d->m_message;
}

QDateTime onlineJobMessage::timestamp() const
{
    Q_D(const onlineJobMessage);
    return d->m_timestamp;
}

void onlineJobMessage::setSenderErrorCode(const QString& errorCode)
{
    Q_D(onlineJobMessage);
    d->m_senderErrorCode = errorCode;
}

QString onlineJobMessage::senderErrorCode()
{
    Q_D(onlineJobMessage);
    return d->m_senderErrorCode;
}
