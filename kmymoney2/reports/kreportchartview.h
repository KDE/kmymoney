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
// #include <KDChartTable>
// #include <KDChartParams>
// #include <KDChartAxisParams>

// ----------------------------------------------------------------------------
// Project Includes
#ifdef _CHECK_MEMORY
  #include <mymoneyutils.h>
#endif

#include "pivotgrid.h"
#include "mymoneyreport.h"

using namespace KDChart;

namespace reports {

class KReportChartView: public Chart
{
public:
  KReportChartView( QWidget* parent );
  ~KReportChartView() {}
  static bool implemented(void) { return true; }
  QStringList& abscissaNames(void) { return m_abscissaNames; }
  void setProperty(int row, int col, int id);

  void setAccountSeries(bool accountSeries) {_accountSeries = accountSeries; }
  bool getAccountSeries(void) {return _accountSeries; }

 /**
   * Draw the chart for a pivot table report
   */
  void drawPivotChart(const PivotGrid &grid, const MyMoneyReport &config, int numColumns, const QStringList& columnHeadings, const QList<ERowType>& rowTypeList, const QStringList& columnTypeHeaderList);

protected:
  bool event( QEvent* event );

private:

/**
  * Draw a PivotGridRowSet in a chart
  */
  unsigned drawPivotRowSet(int rowNum, const bool seriesTotals, const bool accountSeries, const PivotGridRowSet& rowSet, const ERowType rowType, int numColumns, const QString& legendText);

  void setDataCell( int row, int column, const double data);

  void setCellTip( int row, int column, QString tip );

  void justifyModelSize( int rows, int columns );

  QStringList m_abscissaNames;
  bool _accountSeries;

  QLabel *label;

/**
  * Model to store chart data
  */
  QStandardItemModel m_model;
};

} // end namespace reports

#endif // KREPORTCHARTVIEW_H
