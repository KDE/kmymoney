/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2017 Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
