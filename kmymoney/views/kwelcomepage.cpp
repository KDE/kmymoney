/*
    SPDX-FileCopyrightText: 2009 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// Project Includes

#include "kwelcomepage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QStandardPaths>
#include <QApplication>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

KWelcomePage::KWelcomePage()
{
}

KWelcomePage::~KWelcomePage()
{
}

bool KWelcomePage::isGroupHeader(const QString& item)
{
    return item.startsWith(QLatin1Char('*'));
}

bool KWelcomePage::isGroupItem(const QString& item)
{
    return item.startsWith(QLatin1Char('-'));
}


const QString KWelcomePage::welcomePage()
{
    QString header;
    QString footer;
    QString body;

    //header
    header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");
    header = QString("<html>");
    header += QString("<head>");
    header += QString("<title>" + i18n("Home Page") + "</title>");
    header += QString("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">");

    header += QString("</head>");

    //body of the page
    body = QString("<body style=\"margin-top:100px; background: url(qrc:/html/images/bg-texture.png)\">");

    //topright
    body += QString("<table cellpadding=10 align=center width=80% height=100%\">");
    body += QString("<tr><td><h1 id=\"title\">" + i18n("Welcome to KMyMoney") + "</h3></td></tr>");
    body += QString("<tr><td><h3 id=\"subtitle\">" + i18n("The free, easy to use, personal finance manager by KDE") + "</h4></td></tr>");
    body += QString("<tr>");
    body += QString("<td align=left valign='middle'>");
    body += QString("<table width=100% height=100%>");

    //Welcome menu
    body += QString("<tr><td width=5%></td><td width=95%></td></tr>");
    body += QString("<tr><td><img src=\"qrc:/icons/breeze/actions/22/document-new.svg\"></td>");
    body += QString("<td><a href=\"/action?id=file_new\">" + i18n("Get started and setup accounts") + "</a></td></tr>");
    body += QString("<tr><td><img src=\"qrc:/icons/breeze/actions/22/document-open.svg\"></td>");
    body += QString("<td><a href=\"/action?id=file_open\">" + i18n("Open an existing data file") + "</a></td></tr>");
    body += QString("<tr><td><img src=\"qrc:/icons/breeze/actions/22/help-contents.svg\"></td>");
    body += QString("<td><a href=\"/action?id=help_contents\">" + i18n("Open the Handbook and learn how to use KMyMoney") + "</a></td></tr>");
    body += QString("<tr><td><img src=\"qrc:/icons/breeze/actions/22/globe.svg\"></td>");
    body += QString("<td><a href=\"https://kmymoney.org\">" + i18n("Visit our website") + "</a></td></tr>");
    body += QString("<tr><td><img src=\"qrc:/icons/breeze/actions/22/system-users.svg\"></td>");
    body += QString("<td><a href=\"https://kmymoney.org/support.html\">" + i18n("Get help") + "</a></td></tr>");
    body += QString("<tr><td><img src=\"qrc:/icons/breeze/status/22/dialog-information.svg\"></td>");
    body += QString("<td><a href=\"https://kmymoney.org/news/\">" + i18n("See what's new in this version") + "</a></td></tr>");

    body += QString("</table>");
    body += QString("</td>");
    body += QString("<td></td>");
    body += QString("</tr>");
    body += QString("</table>");

    body += QString("</body>");

    //footer
    footer = "</html>";

    return header + body + footer;
}
