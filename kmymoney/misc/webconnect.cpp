/*
    SPDX-FileCopyrightText: 2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "webconnect.h"

#include <QLocalSocket>
#include <QLocalServer>
#include <QStandardPaths>
#include <QDataStream>
#include <QUrl>

Q_LOGGING_CATEGORY(WebConnectLog, "WebConnect")

class WebConnect::Private
{
public:
    explicit Private(WebConnect* parent)
    : q(parent)
    , clientSocket(new QLocalSocket(parent))
    , serverSocket(0)
    , server(new QLocalServer(parent))
    , serverFail(false)
    , blockSize(0)
    {
    }

    WebConnect* q;

    QString         socketName;
    QLocalSocket*   clientSocket;
    QLocalSocket*   serverSocket;
    QLocalServer*   server;
    bool            serverFail;
    quint32         blockSize;

    void startup()
    {
        // create a per user socket name
        socketName = QString("%1/KMyMoney-WebConnect").arg(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
        // try to find a server
        if (!connectToServer()) {
            // no other instance seems to be running, so we start the server
            if (!server->listen(socketName)) {
              qCInfo(WebConnectLog) << "Starting server failed. Try to remove stale socket.";
              server->removeServer(socketName);
              if(!server->listen(socketName)) {
                qCWarning(WebConnectLog) << "Starting server failed again. WebConnect not available.";
                serverFail = true;
              }
            }
            if (!serverFail) {
                qCInfo(WebConnectLog) << "Running in server mode";
            } else {
                qCWarning(WebConnectLog) << "Unable to start server mode";
            }
        } else {
            qCInfo(WebConnectLog) << "Running in client mode";
            clientSocket->disconnectFromServer();
        }
    }

    bool connectToServer()
    {
        // try to find a server
        qCDebug(WebConnectLog) << "Try to connect to WebConnect server";
        clientSocket->setServerName(socketName);
        clientSocket->connectToServer();
        bool rc = clientSocket->waitForConnected(200);
        qCDebug(WebConnectLog) << "Connect to server" << (rc ? "is ok" : "failed");
        return rc;
    }
};

WebConnect::WebConnect(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    connect(d->clientSocket, &QLocalSocket::connected, this, &WebConnect::serverConnected);
    connect(d->clientSocket, &QLocalSocket::disconnected, this, &WebConnect::serverDisconnected);
    connect(d->server, &QLocalServer::newConnection, this, &WebConnect::clientConnected);

    d->startup();
}

WebConnect::~WebConnect()
{
    if (d->server->isListening()) {
        d->server->close();
    }
    delete d;
}

void WebConnect::clientConnected()
{
    qCDebug(WebConnectLog) << "Client connected";
    disconnect(d->server, &QLocalServer::newConnection, this, &WebConnect::clientConnected);
    if (!d->serverSocket) {
        qCDebug(WebConnectLog) << "Get next pending connection";
        // Reset to beginning of stream
        d->blockSize = 0;
        d->serverSocket = d->server->nextPendingConnection();
        connect(d->serverSocket, &QLocalSocket::disconnected, this, &WebConnect::clientDisconnected);
        connect(d->serverSocket, &QLocalSocket::readyRead, this, &WebConnect::dataAvailable);
    }
}

void WebConnect::clientDisconnected()
{
    qCDebug(WebConnectLog) << "Client disconnected";
    d->serverSocket->deleteLater();
    d->serverSocket = 0;
    if (d->server->hasPendingConnections()) {
        qCDebug(WebConnectLog) << "Processing next pending connection";
        clientConnected();
    } else {
        qCDebug(WebConnectLog) << "Wait for next client";
        connect(d->server, &QLocalServer::newConnection, this, &WebConnect::clientConnected);
    }
}

void WebConnect::serverConnected()
{
    qCDebug(WebConnectLog) << "Server connected";
}

void WebConnect::serverDisconnected()
{
    qCDebug(WebConnectLog) << "Server disconnected";
}

void WebConnect::dataAvailable()
{
    QDataStream in(d->serverSocket);
    in.setVersion(QDataStream::Qt_4_0);

    if (d->blockSize == 0) {
        // Relies on the fact that we put the length into the QDataStream
        if (d->serverSocket->bytesAvailable() < (int)sizeof(quint32)) {
            return;
        }
        in >> d->blockSize;
    }

    if (d->serverSocket->bytesAvailable() < (int)sizeof(quint32) || d->serverSocket->atEnd()) {
        return;
    }
    QUrl url;
    in >> url;
    qCDebug(WebConnectLog) << "Processing" << url;
    emit gotUrl(url);
}

void WebConnect::loadFile(const QUrl& url)
{
    if (d->connectToServer()) {
        qCDebug(WebConnectLog) << "Pass to server" << url;
        // transfer filename
        QByteArray block;
        QDataStream stream(&block, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_4_0);
        stream << (quint32) 0;
        stream << url;
        stream.device()->seek(0);
        stream << (quint32)(block.size() - sizeof(quint32));
        d->clientSocket->write(block);
        d->clientSocket->flush();
        d->clientSocket->disconnectFromServer();
    } else {
        qCWarning(WebConnectLog) << "Webconnect loadfile connection failed on client side";
    }
}

bool WebConnect::isClient() const
{
    return !d->server->isListening() && !d->serverFail;
}
