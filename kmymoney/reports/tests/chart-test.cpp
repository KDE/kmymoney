/*
 * Copyright 2016-2018  Christian Dávid <christian-david@web.de>
 * Copyright 2017       Ralf Habacker <ralf.habacker@freenet.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
