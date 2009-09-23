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
//Added by qt3to4:
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

using namespace KDChart;

namespace reports {

class KReportChartView: public Widget
{
public:
  KReportChartView( QWidget* parent );
  ~KReportChartView() {}
  static bool implemented(void) { return true; }
  //void setNewData( const KDChartTableData& newdata ) { this->setData(new KDChartTableData(newdata)); }
  QStringList& abscissaNames(void) { return m_abscissaNames; }
  //void refreshLabels(void) { this->params()->setAxisLabelStringParams( KDChartAxisParams::AxisPosBottom,&m_abscissaNames,0); }
  void setProperty(int row, int col, int id);
//   void setCircularLabels(void) { this->params()->setAxisLabelStringParams( KDChartAxisParams::AxisPosCircular,&m_abscissaNames,0); }

  void setAccountSeries(bool accountSeries) {_accountSeries = accountSeries; }
  bool getAccountSeries(void) {return _accountSeries; }

protected:
  virtual void mouseMoveEvent( QMouseEvent* event );

private:
  QStringList m_abscissaNames;
  bool _accountSeries;

  // label to display when hovering on a data region
  QLabel *label;
};

} // end namespace reports

#endif // KREPORTCHARTVIEW_H
