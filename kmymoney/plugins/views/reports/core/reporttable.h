/*
 * SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
   * Subdirectory for html-resources of application.
   *
   * @see QStandardPaths
   */
  QString m_resourceHtml;

  /**
   * Notation of @c reportstylesheet as used by:
   * @code
   *  MyMoneyFile::instance()::value();
   * @endcode
  */
  QString m_reportStyleSheet;

  /**
   * Filename of default css file.
   */
  QString m_cssFileDefault;

protected:
  ReportTable(const MyMoneyReport &_report);

  /**
   * Constructs html header.
   *
   * @param title html title of report
   * @param[in] includeCSS  flag, whether the generated html has to include the css inline or whether
   * the css is referenced as a link to a file
   * @return  html header
   */
  QString renderHeader(const QString& title, const QByteArray &encoding, bool includeCSS);

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
  virtual void dump(const QString& file, const QString& context = QString()) const = 0;

  /**
   * Creates the complete html document.
   *
   * @param widget      parent widget
   * @param encoding    character set encoding
   * @param title       html title of report
   * @param includeCSS  flag, whether the generated html has
   *                        to include the css inline or whether
   *                        the css is referenced as a link to a file
   *
   * @return complete html document
   */
  QString renderReport(const QString &type, const QByteArray& encoding, const QString& title, bool includeCSS = false);
};

}
#endif
// REPORTTABLE_H
