/*
    SPDX-FileCopyrightText: 2024 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// KDE includes

#include <KLocalizedString>

// Qt includes

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtDebug>

// Project includes

#include "config-kmymoney-version.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#include "mymoneyxmlreader.h"
#include "objectinfotable.h"
#include "pivottable.h"
#include "querytable.h"

#define i18n(a) QLatin1String(a)

using namespace reports;

class Options
{
public:
    QStringList fileUrls;
    bool hasCSVOption = false;
    bool hasCreateReferenceOption = false;
    bool hasHTMLOption = false;
    bool hasListOption = false;
    bool hasObjectInfoTableOption = false;
    bool hasPivotTableOption = false;
    bool hasQueryTableOption = true;
    QString outFileName;
    QString useReportName;

    QString extension() const
    {
        if (hasCSVOption)
            return "csv";
        else
            return "html";
    }
};

QTextStream& qStdOut()
{
    static QTextStream ts(stdout);
    return ts;
}

void exportObjectInfoTable(QTextStream& o, const Options& options, const MyMoneyReport& report)
{
    ObjectInfoTable table(report);
    if (options.hasCSVOption) {
        o << table.renderCSV();
    } else if (options.hasHTMLOption) {
        o << table.renderHTML();
    }
}

void exportPivotTable(QTextStream& o, const Options& options, const MyMoneyReport& report)
{
    PivotTable table(report);
    if (options.hasCSVOption) {
        o << table.renderCSV();
    } else if (options.hasHTMLOption) {
        o << table.renderHTML();
    }
}

void exportQueryTable(QTextStream& o, const Options& options, const MyMoneyReport& report)
{
    QueryTable table(report);
    if (options.hasCSVOption) {
        o << table.renderCSV();
    } else if (options.hasHTMLOption) {
        o << table.renderHTML();
    }
}

void exportTable(QTextStream& o, const Options& options, const MyMoneyReport& report)
{
    if (options.hasObjectInfoTableOption && report.reportType() == eMyMoney::Report::ReportType::InfoTable)
        exportObjectInfoTable(o, options, report);
    if (options.hasPivotTableOption && report.reportType() == eMyMoney::Report::ReportType::PivotTable)
        exportPivotTable(o, options, report);
    if (options.hasQueryTableOption && report.reportType() == eMyMoney::Report::ReportType::QueryTable)
        exportQueryTable(o, options, report);
}

void showReportName(QTextStream& o, const Options& options, const MyMoneyReport& report)
{
    if (options.hasObjectInfoTableOption && report.reportType() == eMyMoney::Report::ReportType::InfoTable)
        o << report.name() << "\n";
    if (options.hasPivotTableOption && report.reportType() == eMyMoney::Report::ReportType::PivotTable)
        o << report.name() << "\n";
    if (options.hasQueryTableOption && report.reportType() == eMyMoney::Report::ReportType::QueryTable)
        o << report.name() << "\n";
    if (!options.hasObjectInfoTableOption && !options.hasPivotTableOption && !options.hasQueryTableOption)
        o << report.name() << "\n";
}

int main(int argc, char** argv)
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kmymoney"));
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("kmmreport-export");
    QCoreApplication::setApplicationVersion(VERSION);

    Options options;
    options.hasCSVOption = true;

    if (argc != 0) {
        QCommandLineParser parser;
        parser.setApplicationDescription(i18n("Export KMyMoney reports"));
        parser.addHelpOption();
        parser.addVersionOption();

        const QCommandLineOption csvOption(QStringLiteral("csv"), i18n("export in csv format (default)"));
        parser.addOption(csvOption);

        const QCommandLineOption htmlOption(QStringLiteral("html"), i18n("export in HTML format"));
        parser.addOption(htmlOption);

        const QCommandLineOption createReferenceOption(QStringLiteral("reference"), i18n("Create reference file from output"));
        parser.addOption(createReferenceOption);

        const QCommandLineOption infoTableOption(QStringLiteral("infotable"), i18n("Use info table for exporting"));
        parser.addOption(infoTableOption);

        const QCommandLineOption pivotTableOption(QStringLiteral("pivottable"), i18n("Use pivot table for exporting"));
        parser.addOption(pivotTableOption);

        const QCommandLineOption queryTableOption(QStringLiteral("querytable"), i18n("Use query table for exporting (default)"));
        parser.addOption(queryTableOption);

        const QCommandLineOption listOption(QStringLiteral("list"), i18n("List all custom reports"));
        parser.addOption(listOption);

        const QCommandLineOption outFileOption(QStringLiteral("output"), i18n("Filename for generated output"), "output");
        parser.addOption(outFileOption);

        const QCommandLineOption reportOption(QStringLiteral("report"), i18n("Process a specific report"), "report");
        parser.addOption(reportOption);

        parser.addPositionalArgument(QStringLiteral("url"), i18n("file to open"));

        parser.process(QCoreApplication::arguments());

        options.fileUrls = parser.positionalArguments();
        options.hasCreateReferenceOption = parser.isSet(createReferenceOption);
        options.hasCSVOption = parser.isSet(csvOption);
        options.hasHTMLOption = parser.isSet(htmlOption);
        if (options.hasCSVOption && options.hasHTMLOption) {
            qWarning() << i18n("Only one of --csv and --html is suppored");
            return 1;
        }
        options.hasListOption = parser.isSet(listOption);
        options.hasObjectInfoTableOption = parser.isSet(infoTableOption);
        options.hasPivotTableOption = parser.isSet(pivotTableOption);
        options.hasQueryTableOption = parser.isSet(queryTableOption);
        if (parser.isSet(outFileOption))
            options.outFileName = parser.value(outFileOption);
        if (parser.isSet(reportOption))
            options.useReportName = parser.value(reportOption);
    }

    for (const auto& fileUrl : options.fileUrls) {
        QFile f(fileUrl);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return 2;

        MyMoneyXmlReader reader;
        MyMoneyFile::instance()->unload();
        reader.setFile(MyMoneyFile::instance());
        reader.read(&f);
        f.close();

        for (const auto& report : MyMoneyFile::instance()->reportList()) {
            if (!options.useReportName.isEmpty() && options.useReportName != report.name())
                continue;
            if (options.hasListOption) {
                showReportName(qStdOut(), options, report);
                continue;
            }
            if (!options.outFileName.isEmpty()) {
                QFile of(options.outFileName);
                if (!of.open(QIODevice::WriteOnly))
                    return 3;
                qDebug() << i18n("saving report '%1' into '%2'").arg(report.name(), of.fileName());
                qStdOut() << of.fileName() << "\n";
                QTextStream o(&of);
                exportTable(o, options, report);
                of.close();
            } else {
                exportTable(qStdOut(), options, report);
            }
        }
    }
    return 0;
}
