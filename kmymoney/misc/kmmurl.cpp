/*
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


// ----------------------------------------------------------------------------
// QT Includes

#include <QRegularExpression>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmurl.h"

#undef QUrl

KMMUrl::KMMUrl()
    : QUrl()
{}

KMMUrl::KMMUrl(const KMMUrl &copy)
    : QUrl(copy)
{}

KMMUrl::KMMUrl(const QUrl &copy)
    : QUrl(copy)
{}

KMMUrl KMMUrl::fromUserInput(const QString &userInput)
{
    auto properUrl = normalizeUrlString(userInput);

    if (userInput.at(0) == ':') {
        auto properQUrl = QUrl::fromLocalFile(properUrl.remove(0, 1));
        properQUrl.setScheme(QStringLiteral("qrc"));

        return properQUrl;
    }
    else {
        return QUrl::fromUserInput(properUrl);
    }
}

QString KMMUrl::toLocalFile() const
{
    if (this->scheme() == QStringLiteral("qrc") || this->url().startsWith(QStringLiteral("qrc"))) {
        return normalizeUrlString(this->url());
    }
    else {
        return QUrl::toLocalFile();
    }
}

QString KMMUrl::normalizeUrlString(const QString &url)
{
    return QString(url).remove(QRegularExpression("^(file:|qrc)", QRegularExpression::CaseInsensitiveOption));
}

