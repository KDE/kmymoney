/*
    SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reporttable.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QLocale>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmurl.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"

reports::ReportTable::ReportTable(const MyMoneyReport& _report):
    m_reportStyleSheet("reportstylesheet"),
    m_config(_report),
    m_containsNonBaseCurrency(false)
{
}

QString reports::ReportTable::cssFileNameGet()
{
    QString cssfilename;

    if (!MyMoneyFile::instance()->value(m_reportStyleSheet).isEmpty()) {
        // try to find the stylesheet specific for this report
        cssfilename = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "html/" + MyMoneyFile::instance()->value(m_reportStyleSheet));
    }

    if (cssfilename.isEmpty() || !QFile::exists(cssfilename)) {
        // if no report specific stylesheet was found, try to use the configured one
        cssfilename = KMyMoneySettings::cssFileDefault();
    }

    return cssfilename;
}

QString reports::ReportTable::renderHeader(const QString& title, const QByteArray& encoding)
{
    QString header = QString(
                         "<!DOCTYPE HTML PUBLIC"
                         " \"-//W3C//DTD HTML 4.01 //EN\""
                         " \"http://www.w3.org/TR/html4/strict.dtd\">"
                         "\n<html>\n<head>"
                         "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\" />"
                         "\n<title>%2</title>")
                         .arg(encoding, title);

    // inline the CSS
    header += "<style type=\"text/css\">\n";
    header += KMyMoneyUtils::getStylesheet(cssFileNameGet());
    header += "</style>\n";

    header += QLatin1String("</head>\n<body>\n");

    return header;
}

QString reports::ReportTable::renderFooter()
{
    return "</body>\n</html>\n";
}

QString reports::ReportTable::renderReport(const QString& type, const QByteArray& encoding, const QString& title)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    QString result;

    // convert a possible infinite report period to valid dates
    QDate fromDate, toDate;
    m_config.validDateRange(fromDate, toDate);

    if (type == QLatin1String("html")) {
        //this renders the HEAD tag and sets the correct css file
        result = renderHeader(title, encoding);

        try {
            // report's name
            result.append(QString::fromLatin1("<h2 class=\"report\">%1</h2>\n").arg(m_config.name()));

            // report's date range
            result.append(QString::fromLatin1("<div class=\"subtitle\">%1</div>\n"
                                              "<div class=\"gap\">&nbsp;</div>\n")
                              .arg(i18nc("Report date range", "%1 through %2", MyMoneyUtils::formatDate(fromDate), MyMoneyUtils::formatDate(toDate))));
            // report's currency information
            if (m_containsNonBaseCurrency) {
                result.append(QString::fromLatin1("<div class=\"subtitle\">%1</div>\n"
                                                  "<div class=\"gap\">&nbsp;</div>\n").arg(m_config.isConvertCurrency() ?
                                                          i18n("All currencies converted to %1", file->baseCurrency().name()) :
                                                          i18n("All values shown in %1 unless otherwise noted", file->baseCurrency().name())));
            } else {
                result.append(QString::fromLatin1("<div class=\"subtitle\">%1</div>\n"
                                                  "<div class=\"gap\">&nbsp;</div>\n").arg(
                                  i18n("All values shown in %1", file->baseCurrency().name())));
            }

            //this method is implemented by each concrete class
            result.append(renderHTML());
        } catch (const MyMoneyException &e) {
            result.append(QString::fromLatin1("<h1>%1</h1><p>%2</p>").arg(i18n("Unable to generate report"),
                          i18n("There was an error creating your report: \"%1\".\nPlease report this error to the developer's list: kmymoney-devel@kde.org", e.what())));
        }

        //this renders a common footer
        result.append(QLatin1String("</body>\n</html>\n"));
    } else if (type == QLatin1String("csv")) {
        result.append(QString::fromLatin1("\"Report: %1\"\n").arg(m_config.name()));
        result.append(
            QString::fromLatin1("%1\n").arg(i18nc("Report date range", "%1 through %2", MyMoneyUtils::formatDate(fromDate), MyMoneyUtils::formatDate(toDate))));
        if (m_containsNonBaseCurrency)
            result.append(QString::fromLatin1("%1\n").arg(m_config.isConvertCurrency() ?
                          i18n("All currencies converted to %1", file->baseCurrency().name()) :
                          i18n("All values shown in %1 unless otherwise noted", file->baseCurrency().name())));
        result.append(renderCSV());
    }

    return result;
}
