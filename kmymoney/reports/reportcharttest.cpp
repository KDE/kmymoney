#include <QApplication>
#include <KDChartWidget>
#include <KDChartLineDiagram>
#include <KDChartCartesianAxis>

using namespace KDChart;

int main( int argc, char** argv )
{
  QApplication app( argc, argv );
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

  return app.exec();
}
