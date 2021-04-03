/*
    SPDX-FileCopyrightText: 2009 Alvaro Soliverez <asoliverez@gmail.com>
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

    const QString welcomeFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/welcome.css");
    header += QString("<link href=\"%1\" rel=\"stylesheet\" type=\"text/css\">").arg(QUrl::fromLocalFile(welcomeFilename).url());
    header += QString("</head>");

    //body of the page
    body = QString("<body>");
    //"background_image", if enabled, displays an image in the background of this page.
    //If you wish to use a background, un-comment the following line
    const QString backgroundFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/background.png");
    body += QString("<img id=\"background_image\" src=\"%1\" height=\"100%\">").arg(QUrl::fromLocalFile(backgroundFilename).url());
    const QString logoFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/trans_logo.png");
    body += QString("<img id=\"KMyMoneyLogo\" src=\"%1\">").arg(QUrl::fromLocalFile(logoFilename).url());
    body += QString("<h3 id=\"title\">" + i18n("Welcome to KMyMoney") + "</h3>");
    body += QString("<h4 id=\"subtitle\">" + i18n("The free, easy to use, personal finance manager by KDE") + "</h4>");
    const QString backArrowFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/backarrow.png");
    body += QString("<div id=\"returnLink\"><a href=\"/home\"><img src=\"%1\">").arg(QUrl::fromLocalFile(backArrowFilename).url());
    body += QString(i18n("Go to My Financial Summary"));
    body += QString("</a></div>");
    body += QString("<div id=\"topleft\">");

    //topright
    body += QString("<div id=\"topright\"></div>");
    body += QString("<div id=\"rightborder\">");
    body += QString("<table style=\"width: 100%;\">");
    body += QString("<tbody>");
    body += QString("<tr>");
    body += QString("<td></td>");
    body += QString("<td style=\"height: 150px;\">");
    body += QString("<div id=\"welcomeMenu\">");
    body += QString("<h4>" + i18n("Start with one of the following activities...") + "</h4>");

    //Welcome menu
    body += QString("<ul>");
    const QString newFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/filenew.png");
    body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(newFilename).url());
    body += QString("<a href=\"/action?id=file_new\">" + i18n("Get started and setup accounts") + "</a></li>");
    const QString dataFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/kmymoneydata.png");
    body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(dataFilename).url());
    body += QString("<a href=\"/action?id=file_open\">" + i18n("Open an existing data file") + "</a></li>");
    const QString manualFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/manual.png");
    body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(manualFilename).url());
    body += QString("<a href=\"/action?id=help_contents\">" + i18n("Learn how to use KMyMoney") + "</a></li>");
    const QString konquerorFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/konqueror.png");
    body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(konquerorFilename).url());
    body += QString("<a href=\"https://kmymoney.org\">" + i18n("Visit our website") + "</a></li>");
    const QString aboutFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/about_kde.png");
    body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(aboutFilename).url());
    body += QString("<a href=\"https://forum.kde.org/viewforum.php?f=69\">" + i18n("Get help from our community") + "</a></li>");
    const QString messageFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/messagebox_info.png");
    body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(messageFilename).url());
    body += QString("<a href=\"/welcome?mode=whatsnew\">" + i18n("See what's new in this version") + "</a></li>");
    body += QString("</ul>");

    body += QString("</div>");
    body += QString("</td>");
    body += QString("<td></td>");
    body += QString("</tr>");
    body += QString("</tbody>");
    body += QString("</table>");
    //right border
    body += QString("</div>");
    body += QString("<div id=\"bottomleft\">");

    //bottom right
    body += QString("<div id=\"bottomright\"></div>");
    //bottom left
    body += QString("</div>");
    //top left
    body += QString("</div>");
    body += QString("</body>");

    //footer
    footer = "</html>";

    return header + body + footer;
}

const QString KWelcomePage::whatsNewPage()
{
    QString header;
    QString footer;
    QString body;

    header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");
    header += QString("<html>");
    header += QString("<head>");
    header += QString("<title>" + i18n("What's new in this version") + "</title>");

    const QString welcomeFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/welcome.css");
    header += QString("<link href=\"%1\" rel=\"stylesheet\" type=\"text/css\">").arg(QUrl::fromLocalFile(welcomeFilename).url());
    header += QString("</head>");

    //body of the page
    body = QString("<body>");
    //"background_image", if enabled, displays an image in the background of this page.
    //If you wish to use a background, un-comment the following line
    const QString backgroundFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/background.png");
    body += QString("<img id=\"background_image\" src=\"%1\" height=\"100%\">").arg(QUrl::fromLocalFile(backgroundFilename).url());
    const QString logoFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/trans_logo.png");
    body += QString("<img id=\"KMyMoneyLogo\" src=\"%1\">").arg(QUrl::fromLocalFile(logoFilename).url());
    body += QString("<h3 id=\"title\">" + i18n("What's new in KMyMoney %1", QApplication::applicationVersion()) + "</h3>");
    const QString backArrowFilename = QStandardPaths::locate(QStandardPaths::DataLocation, "html/images/backarrow.png");
    body += QString("<div id=\"returnLink\"><img src=\"%1\">").arg(QUrl::fromLocalFile(backArrowFilename).url());
    body += QString("<a href=\"/welcome\">" + i18n("Return to the Welcome page") + "</a></div>");

    body += QString("<div id=\"topleft\">");

    body += QString("<div id=\"topright\"></div>");
    body += QString("<div id=\"rightborder\"><table><tr><td>");
//This is where the content should be put to show up inside the decorative frame
//Begin content

    body += QString("<p>" + i18n("We are pleased to announce a major step forward for what has been described as \"the BEST personal finance manager for FREE users\".") + "</p>");
    body += QString("<h4>" + i18n("What's new in this version:") + "</h4>");

    QStringList featuresList = KWelcomePage::featuresList();

    bool inGroup = false;
    QStringList::ConstIterator feature_it;
    for (feature_it = featuresList.constBegin(); feature_it != featuresList.constEnd(); ++feature_it) {
        if (isGroupHeader(*feature_it)) {
            if (inGroup) {
                body += QString("</ul>");
            }
            body += QString("<b>");
            body += (*feature_it).midRef(2);
            body += QString("</b><br/>");
            body += QString("<ul>");
            inGroup = true;

        } else if(isGroupItem(*feature_it)) {
            body += QString("<li>");
            body += (*feature_it).midRef(2);
            body += QString("</li>");
        } else {
            body += *feature_it;
        }
    }
    if (inGroup) {
        body += QString("</ul>");
    }

    body += QString("<p>" + i18n("Let us know what you think. We hope that you enjoy using the application.") + "</p>");
    body += QString("<p>" + i18n("Please let us know about any abnormal behavior in the program by selecting <a href=\"/action?id=help_report_bug\">\"Report bug...\"</a> from the help menu or by sending an e-mail to the developers mailing list."));
    body += QString("<font color=\"blue\"><a href=\"mailto:kmymoney-devel@kde.org\">kmymoney-devel@kde.org</a></font></p>");
    body += QString("<p><div align=\"right\">");
    body += QString(i18n("The KMyMoney Development Team"));
    body += QString("</div></p>");

    //End content
    body += QString("</td></tr></table>");
    body += QString("</div>");
    body += QString("<div id=\"bottomleft\"><div id=\"bottomright\"></div>");
    body += QString("</div>");
    body += QString("</div>");
    body += QString("<div style=\"margin-bottom: 65px\"></div>");
    body += QString("</body>");

    //footer
    footer += QString("</html>");

    return header + body + footer;
}

const QStringList KWelcomePage::featuresList()
{
    QStringList featuresList;

    featuresList.append(i18n("* General changes"));
    featuresList.append(i18n("- Port to KDE frameworks and Qt5"));

    featuresList.append(i18n("* User Interface changes"));
    featuresList.append(i18n("- Show more tooltips why features are not available"));
    featuresList.append(i18n("- Improved compatibility with dark color schemes"));
    featuresList.append(i18n("- Fast switching of main views via Ctrl + number key"));
    featuresList.append(i18n("- Improved keyboard navigation"));
    featuresList.append(i18n("- View columns are user selectable"));
    featuresList.append(i18n("- Use QWebEngine in favor of KHTML when available"));

    featuresList.append(i18n("* Im-/Exporter"));
    featuresList.append(i18n("- Added support for Weboob"));
    featuresList.append(i18n("- Improved CSV importer"));
    featuresList.append(i18n("- Added CSV exporter"));
    featuresList.append(i18n("- Improved payee matching when importing transactions"));

    featuresList.append(i18n("* Online services"));
    featuresList.append(i18n("- Updated list of application versions for OFX direct import"));
    featuresList.append(i18n("- Get rid of Yahoo as price source"));
    featuresList.append(i18n("- Supporting OFX client uid required by some banks"));
    featuresList.append(i18n("- Support price download by ISIN"));

    return featuresList;
}

