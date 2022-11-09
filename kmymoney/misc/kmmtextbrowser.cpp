/*
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QRegularExpression>
#include <QTextBrowser>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmtextbrowser.h"

KMMTextBrowser::KMMTextBrowser(QWidget* parent)
    : QTextBrowser(parent)
{
}
/*
 * This implementation can be simplified with the port to Qt 6.3 by using
 * QTextDocument::setMetaInformation(QTextDocument::CssMedia, "print"),
 * see https://doc.qt.io/qt-6/qtextdocument.html#MetaInformation-enum
 */
void KMMTextBrowser::print(QPagedPaintDevice* printer)
{
    QTextDocument documentCopy;
    documentCopy.setDefaultStyleSheet("");
    documentCopy.setHtml(m_html);
    documentCopy.print(printer);
}

void KMMTextBrowser::setHtml(const QString& text)
{
    m_html = text;
    m_html.replace("@media screen", "@media _screen").replace("@media print", "@media screen");
    QTextBrowser::setHtml(text);
}
