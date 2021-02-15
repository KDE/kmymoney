/*
    SPDX-FileCopyrightText: 2016-2018 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "chart-test.h"

#include <QTest>
#include <QTimer>

#include <KChartWidget>
#include <KChartLineDiagram>
#include <KChartCartesianAxis>

void ChartTest::createChart()
{
  using namespace KChart;

  Widget widget;
  widget.resize( 600, 600 );

  QVector< double > vec0,  vec1, vec2;
  vec0 << 5 << 1 << 3 << 4 << 1;
  vec1 << 3 << 6 << 2 << 4 << 8;
  vec2 << 0 << 7 << 1 << 2 << 1;
  widget.setDataset( 0, vec0, "vec0" );
  widget.setDataset( 1, vec1, "vec1" );
  widget.setDataset( 2, vec2, "vec2" );
  CartesianAxis *xAxis = new CartesianAxis( widget.lineDiagram() );
  CartesianAxis *yAxis = new CartesianAxis (widget.lineDiagram() );
  xAxis->setPosition ( CartesianAxis::Bottom );
  yAxis->setPosition ( CartesianAxis::Left );
  xAxis->setTitleText ( "Abscissa bottom position" );
  yAxis->setTitleText ( "Ordinate left position" );
  widget.lineDiagram()->addAxis( xAxis );
  widget.lineDiagram()->addAxis( yAxis );


  widget.show();
  QVERIFY(QTest::qWaitForWindowActive(&widget, 10000));
}

QTEST_MAIN(ChartTest)
