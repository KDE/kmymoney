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

void KMMTextBrowser::print(QPagedPaintDevice* printer)
{
    // TODO: QTBUG: https://bugreports.qt.io/browse/QTBUG-98408
    auto printCss = QString(m_css).replace("@media screen", "@media _screen").replace("@media print", "@media screen");

    QTextDocument documentCopy;
    documentCopy.setDefaultStyleSheet(printCss);
    documentCopy.setHtml(m_html);
    // end QTBUG

    documentCopy.print(printer);
}

// https://invent.kde.org/office/kmymoney/-/merge_requests/118?commit_id=2e5d13137dbe391a4cb3ee1b57890c2973633c62#note_346468
void KMMTextBrowser::setHtml(const QString& text)
{
    m_html = text;

    QRegularExpression re("<style.*?>(.*)<\\/style>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(text);

    if (match.hasMatch()) {
        m_css = match.captured(1);
        m_html = m_html.remove(match.capturedRef(0).position(), match.capturedRef(1).length());
    }

    QTextBrowser::setHtml(text);
}
