/***************************************************************************
                          kreportchartview.h
                             -------------------
    begin                : Sat May 22 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KREPORTCHARTVIEW_H
#define KREPORTCHARTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
// Some STL headers in GCC4.3 contain operator new. Memory checker mangles these
#ifdef _CHECK_MEMORY
#undef new
#endif

#include <QLabel>
#include <QStandardItemModel>
#include <QMouseEvent>
#include <KDChartWidget>
#include <KDChartChart>
#include <KDChartAbstractDiagram>
#include <KDChartAbstractCoordinatePlane>

// ----------------------------------------------------------------------------
// Project Includes
#ifdef _CHECK_MEMORY
#include <mymoneyutils.h>
#endif

#include "pivotgrid.h"
#include "mymoneyreport.h"

using namespace KDChart;

namespace reports
{

class KReportChartView: public Chart
{
public:
  KReportChartView(QWidget* parent);
  ~KReportChartView() {}

  /**
    * Whether the calling report has chart capabilities
    */
  static bool implemented(void) {
    return true;
  }

  /**
    * Returns the labels for the X axis
    * @see m_abscissaNames
    */
  QStringList& abscissaNames(void) {
    return m_abscissaNames;
  }

  /**
    * Draw the chart for a pivot table report
    */
  void drawPivotChart(const PivotGrid &grid, const MyMoneyReport &config, int numberColumns, const QStringList& columnHeadings, const QList<ERowType>& rowTypeList, const QStringList& columnTypeHeaderList);

  /**
    * Draw a limit chart
    * @param limit is either a maximum credit or minimum balance for an account
    */
  void drawLimitLine(const double limit);

  /**
    * Remove the chart legend
    */
  void removeLegend(void);

protected:
  /**
    * This is an overload method needed to capture the mouse events
    */
  bool event(QEvent* event);

private:

  /**
    * Draw a PivotGridRowSet in a chart
    */
  unsigned drawPivotRowSet(int rowNum, const PivotGridRowSet& rowSet, const ERowType rowType, const QString& legendText, int startColumn = 1, int endColumn = 0);

  /**
    * Set the data value
    */
  void setDataCell(int row, int column, const double data);

  /**
    * Set the tooltip for a data value
    */
  void setCellTip(int row, int column, QString tip);

  /**
    * Make sure the model has the right size
    */
  void justifyModelSize(int rows, int columns);

  /**
    * Adjust line width of all datasets
    */
  void setLineWidth(const int lineWidth);

  /**
    * Set the accountSeries
    * @see m_accountSeries
    */
  void setAccountSeries(bool accountSeries) {
    m_accountSeries = accountSeries;
  }

  /**
    * Returns accountSeries
    * @see m_accountSeries
    */
  bool accountSeries(void) {
    return m_accountSeries;
  }

  /**
    * Set the seriesTotals
    * @see m_seriesTotals
    */
  void setSeriesTotals(bool seriesTotals) {
    m_seriesTotals = seriesTotals;
  }

  /**
    * Returns accountSeries
    * @see m_seriesTotals
    */
  bool seriesTotals(void) {
    return m_seriesTotals;
  }

  /**
    * Set the number of columns
    * @see m_numColumns
    */
  void setNumColumns(int numColumns) {
    m_numColumns = numColumns;
  }

  /**
    * Returns number of columns
    * @see m_numColumns
    */
  int numColumns(void) {
    return m_numColumns;
  }

  /**
    * The labels of the X axis
    */
  QStringList m_abscissaNames;

  /**
    * whether series (rows) are accounts (true) or months (false). This causes a lot
    * of complexity in the charts.  The problem is that circular reports work best with
    * an account in a COLUMN, while line/bar prefer it in a ROW.
    */
  bool m_accountSeries;

  /**
    * whether to limit the chart to use series totals only.  Used for reports which only
    * show one dimension (pie)
    */
  bool m_seriesTotals;

  /**
    * Number of columns on the report
    */
  int m_numColumns;

  /**
    * Model to store chart data
    */
  QStandardItemModel m_model;
};

} // end namespace reports

#endif // KREPORTCHARTVIEW_H
