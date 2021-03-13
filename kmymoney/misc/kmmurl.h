/*
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMURL_H
#define KMMURL_H

#undef QUrl

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QUrl>


class KMMUrl: public QUrl
{
public:
    KMMUrl();
    KMMUrl(const KMMUrl &copy);
    KMMUrl(const QUrl &copy);

    static KMMUrl fromUserInput(const QString &userInput);
    QString toLocalFile() const;

private:
    static QString normalizeUrlString(const QString &url);

};

#define QUrl KMMUrl
#endif // KMMURL_H
