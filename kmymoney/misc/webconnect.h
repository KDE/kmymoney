/*
    SPDX-FileCopyrightText: 2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WEBCONNECT_H
#define WEBCONNECT_H

#include <QObject>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(WebConnectLog)

class WebConnect : public QObject
{
    Q_OBJECT
public:
    explicit WebConnect(QObject* parent);
    virtual ~WebConnect();

    bool isClient() const;

public Q_SLOTS:
    void loadFile(const QUrl& url);

private Q_SLOTS:
    void serverConnected();
    void serverDisconnected();
    void clientConnected();
    void clientDisconnected();
    void dataAvailable();

Q_SIGNALS:
    void gotUrl(const QUrl& url);

private:
    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif // WEBCONNECT_H
