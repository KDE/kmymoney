//Added by qt3to4:
#include <QMouseEvent>
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


#ifdef HAVE_KDCHART
// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
// Some STL headers in GCC4.3 contain operator new. Memory checker mangles these
#ifdef _CHECK_MEMORY
  #undef new
#endif

#include <qlabel.h>
#include <KDChartWidget.h>
#include <KDChartTable.h>
#include <KDChartParams.h>
#include <KDChartAxisParams.h>

// ----------------------------------------------------------------------------
// Project Includes
#ifdef _CHECK_MEMORY
  #include <mymoneyutils.h>
#endif

namespace reports {

class KReportChartView: public KDChartWidget
{
public:
  KReportChartView( QWidget* parent, const char* name );
  ~KReportChartView() {}
  static bool implemented(void) { return true; }
  void setNewData( const KDChartTableData& newdata ) { this->setData(new KDChartTableData(newdata)); }
  QStringList& abscissaNames(void) { return m_abscissaNames; }
  void refreshLabels(void) { this->params()->setAxisLabelStringParams( KDChartAxisParams::AxisPosBottom,&m_abscissaNames,0); }
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

#else

namespace reports {

class KReportChartView : public QWidget
{
public:
  KReportChartView( QWidget* parent, const char* name ): QWidget(parent,name) {}
  ~KReportChartView() {}
  static bool implemented(void) { return false; }
};

} // end namespace reports

#endif

#endif // KREPORTCHARTVIEW_H
