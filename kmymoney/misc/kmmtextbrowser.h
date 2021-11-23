/*
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMTEXTBROWSER_H
#define KMMTEXTBROWSER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTextBrowser>
class QString;

class KMMTextBrowser : public QTextBrowser
{
public:
    KMMTextBrowser(QWidget* parent = nullptr);
    void print(QPagedPaintDevice* printer);
    void setHtml(const QString& text);

private:
    QString m_html;
    QString m_css;
};

#endif // KMMTEXTBROWSER_H
