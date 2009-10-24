/***************************************************************************
                          kreportchartview.cpp
                             -------------------
    begin                : Sun Aug 14 2005
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes
#include <QMouseEvent>
#include <QLabel>
#include <QFrame>
#include <QStandardItemModel>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kcolorscheme.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kreportchartview.h"
#include <KDChartBackgroundAttributes>
#include <KDChartDataValueAttributes>
#include <KDChartMarkerAttributes>
#include <KDChartGridAttributes>
#include <KDChartHeaderFooter>
#include <KDChartLegend>
#include <KDChartLineDiagram>
#include <KDChartBarDiagram>
#include <KDChartPieDiagram>
#include <KDChartRingDiagram>
#include <KDChartCartesianAxis>

using namespace reports;

KReportChartView::KReportChartView( QWidget* parent): KDChart::Chart(parent)
{
  // ********************************************************************
  // Set KMyMoney's Chart Parameter Defaults
  // ********************************************************************

  //Set the background to white
  BackgroundAttributes backAttr = backgroundAttributes();
  KColorScheme colorScheme(QPalette::Normal);
  backAttr.setBrush(colorScheme.background());
  backAttr.setVisible(true);
  setBackgroundAttributes(backAttr);

  //Line diagram
  KDChart::LineDiagram* diagram = new KDChart::LineDiagram;
  diagram->setModel(&m_model);
  this->coordinatePlane()->replaceDiagram(diagram);

  //Subdued colors
  diagram->useSubduedColors();
}

void KReportChartView::drawPivotChart(const PivotGrid &grid, const MyMoneyReport &config, int numColumns, const QStringList& columnHeadings, const QList<ERowType>& rowTypeList, const QStringList& columnTypeHeaderList)
{
  //remove existing headers
  while(headerFooters().count() > 0) {
    HeaderFooter* delHeader = headerFooters().at(0);
    takeHeaderFooter(delHeader);
    delete delHeader;
  }

  //set the new header
  HeaderFooter* header = new HeaderFooter(this);
  header->setText(config.name());
  header->setType(HeaderFooter::Header);
  header->setPosition(Position::North);
  addHeaderFooter(header);

  //set grid attributes
  GridAttributes gridAttr(coordinatePlane()->globalGridAttributes());
  gridAttr.setGridVisible(config.isChartGridLines());
  coordinatePlane()->setGlobalGridAttributes(gridAttr);

  // whether to limit the chart to use series totals only.  Used for reports which only
  // show one dimension (pie).
  bool seriesTotals = false;

  // whether series (rows) are accounts (true) or months (false). This causes a lot
  // of complexity in the charts.  The problem is that circular reports work best with
  // an account in a COLUMN, while line/bar prefer it in a ROW.
  bool accountSeries = true;

  //what values should be shown
  bool showBudget = config.hasBudget();
  bool showForecast = config.isIncludingForecast();
  bool showActual = false;
  if( (config.isIncludingBudgetActuals()) || ( !showBudget && !showForecast) )
    showActual = true;

  switch( config.chartType() )
  {
    case MyMoneyReport::eChartNone:
    case MyMoneyReport::eChartEnd:
    case MyMoneyReport::eChartLine:
    {
      KDChart::LineDiagram* diagram = new KDChart::LineDiagram;
      diagram->setModel(&m_model);
      coordinatePlane()->replaceDiagram(diagram);
      break;
    }
    case MyMoneyReport::eChartBar:
    {
      KDChart::BarDiagram* diagram = new KDChart::BarDiagram;
      diagram->setModel(&m_model);
      coordinatePlane()->replaceDiagram(diagram);
      break;
    }
    case MyMoneyReport::eChartStackedBar:
    {
      KDChart::BarDiagram* diagram = new KDChart::BarDiagram;
      diagram->setType(BarDiagram::Stacked);
      diagram->setModel(&m_model);
      coordinatePlane()->replaceDiagram(diagram);
      break;
    }
    case MyMoneyReport::eChartPie:
    {
      KDChart::PieDiagram* diagram = new KDChart::PieDiagram;
      diagram->setModel(&m_model);
      PolarCoordinatePlane* polarPlane = new PolarCoordinatePlane;
      replaceCoordinatePlane(polarPlane);
      coordinatePlane()->replaceDiagram(diagram);
      accountSeries = false;
      seriesTotals = true;
      break;
    }
    case MyMoneyReport::eChartRing:
    {
      KDChart::RingDiagram* diagram = new KDChart::RingDiagram;
      diagram->setModel(&m_model);
      PolarCoordinatePlane* polarPlane = new PolarCoordinatePlane;
      replaceCoordinatePlane(polarPlane);
      coordinatePlane()->replaceDiagram(diagram);
      //chartView.params()->setRelativeRingThickness( true );
      accountSeries = false;
      break;
    }
  }

  //set data value attributes
  DataValueAttributes valueAttr(coordinatePlane()->diagram()->dataValueAttributes());
  valueAttr.setVisible(config.isChartDataLabels());
  coordinatePlane()->diagram()->setDataValueAttributes(valueAttr);

  //Subdued colors - we set it here again because it is a property of the diagram
  coordinatePlane()->diagram()->useSubduedColors();

  //the legend will be used later
  Legend* legend = new Legend(coordinatePlane()->diagram(), this);

  //set up the axes for cartesian diagrams
  if(qobject_cast<LineDiagram*>(coordinatePlane()->diagram()) ||
      qobject_cast<BarDiagram*>(coordinatePlane()->diagram())) {
    //set x axis
    CartesianAxis *xAxis = new CartesianAxis();
    xAxis->setPosition ( CartesianAxis::Bottom );
    xAxis->setTitleText(i18n("Time"));
    TextAttributes xAxisTextAttr(xAxis->titleTextAttributes());
    xAxisTextAttr.setMinimalFontSize(12);
    xAxis->setTitleTextAttributes(xAxisTextAttr);

    // Set up X axis labels (ie "abscissa" to use the technical term)
    QStringList abscissaNames;
    if ( accountSeries ) // if not, we will set these up while putting in the chart values.
    {
      int column = 1;
      while ( column < numColumns ) {
        abscissaNames += QString(columnHeadings[column++]).replace("&nbsp;", " ");
      }
      xAxis->setLabels(abscissaNames);
    }

    //set y axis
    CartesianAxis *yAxis = new CartesianAxis ();
    yAxis->setPosition ( CartesianAxis::Left );
    yAxis->setTitleText(i18n("Balance"));
    TextAttributes yAxisTextAttr(yAxis->titleTextAttributes());
    yAxisTextAttr.setMinimalFontSize(12);
    yAxis->setTitleTextAttributes(yAxisTextAttr);

    //add the axes to the corresponding diagram
    if(qobject_cast<LineDiagram*>(coordinatePlane()->diagram())) {
      KDChart::LineDiagram* lineDiagram = qobject_cast<LineDiagram*>(coordinatePlane()->diagram());

      //set line width
      /*QPen linePen(lineDiagram->pen());
      linePen.setWidth(config.chartLineWidth());
      lineDiagram->setPen(linePen);*/

      //remove all existing axes before inserting new ones
      while(lineDiagram->axes().count() > 0) {
        CartesianAxis *delAxis  = lineDiagram->axes().at(0);
        lineDiagram->takeAxis(delAxis);
        delete delAxis;
      }

      //add the new axes
      lineDiagram->addAxis( xAxis );
      lineDiagram->addAxis( yAxis );

    } else if(qobject_cast<BarDiagram*>(coordinatePlane()->diagram())) {
      KDChart::BarDiagram* barDiagram = qobject_cast<BarDiagram*>(coordinatePlane()->diagram());
      barDiagram->addAxis( xAxis );
      barDiagram->addAxis( yAxis );
    }
  }

  // For onMouseOver events, we want to activate mouse tracking
  setMouseTracking( true );

  // The KReportChartView widget needs to know whether the legend
  // corresponds to rows or columns
  setAccountSeries( accountSeries );

  switch ( config.detailLevel() )
  {
    case MyMoneyReport::eDetailNone:
    case MyMoneyReport::eDetailEnd:
    case MyMoneyReport::eDetailAll:
    {
      int rowNum = 0;

      // iterate over outer groups
      PivotGrid::const_iterator it_outergroup = grid.begin();
      while ( it_outergroup != grid.end() )
      {
        // iterate over inner groups
        PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        while ( it_innergroup != (*it_outergroup).end() )
        {
          //
          // Rows
          //
          QString innergroupdata;
          PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();
          while ( it_row != (*it_innergroup).end() )
          {
            //Do not include investments accounts in the chart because they are merely container of stock and other accounts
            if( it_row.key().accountType() != MyMoneyAccount::Investment) {
              //iterate row types
              for(int i = 0; i < rowTypeList.size(); ++i) {
                //skip the budget difference rowset
                if(rowTypeList[i] != eBudgetDiff ) {
                  QString legendText;

                  //only show the column type in the header if there is more than one type
                  if(rowTypeList.size() > 1) {
                    legendText = QString(columnTypeHeaderList[i] + " - " + it_row.key().name());
                  } else {
                    legendText = QString(it_row.key().name());
                  }

                  //set the cell value and tooltip
                  rowNum = drawPivotRowSet(rowNum, seriesTotals, accountSeries, it_row.value(), rowTypeList[i], numColumns, legendText);

                  //set the legend text
                  legend->setText(rowNum-1, legendText);
                }
              }
            }
            ++it_row;
          }
          ++it_innergroup;
        }
        ++it_outergroup;
      }
      replaceLegend(legend);
    }
    break;

    case MyMoneyReport::eDetailTop:
    {
      int rowNum = 0;

      // iterate over outer groups
      PivotGrid::const_iterator it_outergroup = grid.begin();
      while ( it_outergroup != grid.end() )
      {

        // iterate over inner groups
        PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        while ( it_innergroup != (*it_outergroup).end() )
        {
          //iterate row types
          for(int i = 0; i < rowTypeList.size(); ++i) {
            //skip the budget difference rowset
            if(rowTypeList[i] != eBudgetDiff ) {
              QString legendText;


              //only show the column type in the header if there is more than one type
              if(rowTypeList.size() > 1) {
                legendText = QString(columnTypeHeaderList[i] + " - " + it_innergroup.key());
              } else {
                legendText = QString(it_innergroup.key());
              }

              //set the cell value and tooltip
              rowNum = drawPivotRowSet(rowNum, seriesTotals, accountSeries, (*it_innergroup).m_total, rowTypeList[i], numColumns, legendText);

              //set the legend text
              legend->setText(rowNum-1, legendText);
            }
          }
          ++it_innergroup;
        }
        ++it_outergroup;
      }
      replaceLegend(legend);
    }
    break;

    case MyMoneyReport::eDetailGroup:
    {
      int rowNum = 0;

      // iterate over outer groups
      PivotGrid::const_iterator it_outergroup = grid.begin();
      while ( it_outergroup != grid.end() )
      {
        //iterate row types
        for(int i = 0; i < rowTypeList.size(); ++i) {
          //skip the budget difference rowset
          if(rowTypeList[i] != eBudgetDiff ) {
            QString legendText;

            //only show the column type in the header if there is more than one type
            if(rowTypeList.size() > 1) {
              legendText = QString(columnTypeHeaderList[i] + " - " + it_outergroup.key());
            } else {
              legendText = QString(it_outergroup.key());
            }

            //set the cell value and tooltip
            rowNum = drawPivotRowSet(rowNum, seriesTotals, accountSeries, (*it_outergroup).m_total, rowTypeList[i], numColumns, legendText);

            //set the legend
            legend->setText(rowNum-1, legendText);
          }
        }
        ++it_outergroup;
      }

      //if selected, show totals too
      if (config.isShowingRowTotals())
      {
        //iterate row types
        for(int i = 0; i < rowTypeList.size(); ++i) {
          //skip the budget difference rowset
          if(rowTypeList[i] != eBudgetDiff ) {
            QString legendText;

            //only show the column type in the header if there is more than one type
            if(rowTypeList.size() > 1) {
              legendText = QString(columnTypeHeaderList[i] + " - " + i18nc("Total balance", "Total"));
            } else {
              legendText = QString(i18nc("Total balance", "Total"));
            }

            //set the cell value
            rowNum = drawPivotRowSet(rowNum, seriesTotals, accountSeries, grid.m_total, rowTypeList[i], numColumns, legendText);

            //set the legend
            legend->setText(rowNum -1, legendText);

          }
        }
      }
      replaceLegend(legend);
    }
    break;

    case MyMoneyReport::eDetailTotal:
    {
      int rowNum = 0;

      //iterate row types
      for(int i = 0; i < rowTypeList.size(); ++i) {
        //skip the budget difference rowset
        if(rowTypeList[i] != eBudgetDiff ) {
          QString legendText;

          //only show the column type in the header if there is more than one type
          if(rowTypeList.size() > 1) {
              legendText = QString(columnTypeHeaderList[i] + " - " + i18nc("Total balance", "Total"));
            } else {
              legendText = QString(i18nc("Total balance", "Total"));
          }

          //set cell value
          rowNum = drawPivotRowSet(rowNum, seriesTotals, accountSeries, grid.m_total, rowTypeList[i], numColumns, legendText);

          //set legend text
          legend->setText(rowNum -1, legendText);
        }
      }
      replaceLegend(legend);
    }
    break;
  }

  //set the legend basic attributes
  //this is done after adding the legend because the values are overriden when adding the legend to the chart
  legend->setPosition(Position::East);
  TextAttributes legendTextAttr(legend->textAttributes());
  legendTextAttr.setFontSize(10);
  legendTextAttr.setAutoShrink(true);
  legend->setTextAttributes(legendTextAttr);

  TextAttributes legendTitleTextAttr(legend->titleTextAttributes());
  legendTitleTextAttr.setFontSize(24);
  legendTitleTextAttr.setAutoShrink(true);
  legend->setTitleTextAttributes(legendTitleTextAttr);
  legend->setTitleText(i18nc("Chart lines legend","Legend"));
  legend->setUseAutomaticMarkerSize( false );

  //line markers
  DataValueAttributes dataValueAttr(coordinatePlane()->diagram()->dataValueAttributes());
  MarkerAttributes markerAttr(dataValueAttr.markerAttributes());
  markerAttr.setVisible(true);
  markerAttr.setMarkerStyle(MarkerAttributes::MarkerCircle);
  markerAttr.setMarkerSize(QSize(8,8));
  dataValueAttr.setMarkerAttributes(markerAttr);
  coordinatePlane()->diagram()->setDataValueAttributes(dataValueAttr);

  //make sure to show only the required number of fractional digits on the labels of the graph
  //chartView.params()->setDataValuesCalc(0, MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));
  //chartView.refreshLabels();
}

unsigned KReportChartView::drawPivotRowSet(int rowNum, const bool seriesTotals, const bool accountSeries, const PivotGridRowSet& rowSet, const ERowType rowType, int numColumns, const QString& legendText )
{

  // Columns
  if ( seriesTotals )
  {
    double value = rowSet[rowType].m_total.toDouble();

    //set the tooltip
    QString toolTip = QString("<h2>%1</h2><strong>%2</strong><br>")
        .arg(legendText)
        .arg(value, 0, 'f', 2);

    if ( accountSeries ) {
      //set the cell value
      this->setDataCell(rowNum, 0, value);
      this->setCellTip(rowNum, 0, toolTip);
   } else {
      this->setDataCell(0, rowNum, value);
      this->setCellTip(0, rowNum, toolTip);
    }
  } else {
    int column = 1;
    while ( column < numColumns )
    {
      double value = rowSet[rowType][column].toDouble();
      QString toolTip = QString("<h2>%1</h2><strong>%2</strong><br>")
        .arg(legendText)
        .arg(value, 0, 'f', 2);

      if ( accountSeries ) {
        this->setDataCell(column-1, rowNum, value);
        this->setCellTip(column-1, rowNum, toolTip);
      } else {
        this->setDataCell(rowNum, column-1, value );
        this->setCellTip(rowNum, column-1, toolTip);
      }
      ++column;
    }
  }
  return ++rowNum;
}

void KReportChartView::setDataCell( int row, int column, const double data)
{
    if ( ! coordinatePlane()->diagram()->datasetDimension() == 1 )
        return;

    justifyModelSize( row + 1, column + 1 );

    const QModelIndex index = m_model.index( row, column );
    m_model.setData( index, QVariant( data ), Qt::DisplayRole );
}

void KReportChartView::setCellTip( int row, int column, QString tip )
{
    if ( ! coordinatePlane()->diagram()->datasetDimension() == 1 )
        return;

    justifyModelSize( row + 1, column + 1 );

    const QModelIndex index = m_model.index( row, column );
    m_model.setData( index, QVariant( tip ), Qt::ToolTipRole );
}

/**
 * Justifies the model, so that the given rows and columns fit into it.
 */
void KReportChartView::justifyModelSize( int rows, int columns )
{
    const int currentRows = m_model.rowCount();
    const int currentCols = m_model.columnCount();

    if ( currentCols < columns )
        if ( ! m_model.insertColumns( currentCols, columns - currentCols ))
            qDebug() << "justifyModelSize: could not increase model size.";
    if ( currentRows < rows )
        if ( ! m_model.insertRows( currentRows, rows - currentRows ))
            qDebug() << "justifyModelSize: could not increase model size.";

    Q_ASSERT( m_model.rowCount() >= rows );
    Q_ASSERT( m_model.columnCount() >= columns );
}

bool KReportChartView::event( QEvent* event )
{
  return Chart::event(event);
}
