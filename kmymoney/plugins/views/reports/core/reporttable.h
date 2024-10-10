/*
    SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REPORTTABLE_H
#define REPORTTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyreport.h"

namespace reports
{

class KReportChartView;

/**
  * This class serves as base class definition for the concrete report classes
  * This class is abstract but it contains common code used by all children classes
  */
class ReportTable : public QObject
{
    Q_OBJECT
private:

    /**
     * Tries to find a css file for the report.
     *
     * Search is done in following order:
     * <ol>
     *  <li> report specific stylesheet
     *  <li> configured stylesheet
     *  <li> installation default of stylesheet
     * </ol>
     *
     * @retval css-filename  if a css-file was found
     * @retval empty-string  if no css-file was found
     */
    QString cssFileNameGet();

    /**
     * Notation of @c reportstylesheet as used by:
     * @code
     *  MyMoneyFile::instance()::value();
     * @endcode
    */
    QString m_reportStyleSheet;

protected:
    ReportTable(const MyMoneyReport &_report);

    /**
     * Constructs html header.
     *
     * @param title html title of report
     * @return  html header
     */
    QString renderHeader(const QString& title, const QByteArray& encoding);

    /**
     * Constructs html footer.
     *
     * @return  html footer
     */
    QString renderFooter();

    /**
     * Constructs the body of the report. Implemented by the concrete classes
     * @see PivotTable
     * @see ListTable
     * @return QString with the html body of the report
     */
    virtual QString renderHTML() const = 0;

    MyMoneyReport m_config;
    /**
     * Does the report contain any non-base currency
     */
    mutable bool m_containsNonBaseCurrency;

    /**
     * The report may contain incorrect values
     */
    mutable bool m_mayContainIncorrectValues{false};

public:
    virtual ~ReportTable() {}

    /**
     * Constructs a comma separated-file of the report. Implemented by the concrete classes
     * @see PivotTable
     * @see ListTable
     */
    virtual QString renderCSV() const = 0;

    /**
     * Renders a graph from the report. Implemented by the concrete classes
     * @see PivotTable
     */
    virtual void drawChart(KReportChartView& view) const = 0;

    /**
     * Dump the report's HTML to a file. Implemented by the concrete classes
     *
     * @param file The filename to dump into
     * @param context QString-like format string with which the generated report is to be wrapped (at least “%1” is required)
     */
    virtual void dump(const QString& file, const QString& context = QString()) const = 0;

    /**
     * Saves report data to an xml file. Implemented by the concrete classes
     *
     * @param file file to save the report data into
     * @return state of saving
     */
    virtual bool saveToXml(const QString& file) = 0;

    /**
     * Returns report data as string in xml format. Implemented by the concrete classes
     *
     * @return string with report data in xml format
     */
    virtual QString toXml() const = 0;

    /**
     * Creates the complete html document.
     *
     * @param widget      parent widget
     * @param encoding    character set encoding
     * @param title       html title of report
     * @return complete html document
     */
    QString renderReport(const QString& type, const QByteArray& encoding, const QString& title);
};

}
#endif
// REPORTTABLE_H
