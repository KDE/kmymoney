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

#include "kreportchartview.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QLabel>
#include <QFrame>
#include <QStandardItemModel>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kcolorscheme.h>
#include <klocale.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes
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
#include <KDChartFrameAttributes>
#include "kmymoneyglobalsettings.h"
#include <kbalanceaxis.h>
#include <mymoneyfile.h>

using namespace reports;

KReportChartView::KReportChartView(QWidget* parent) :
    KDChart::Chart(parent),
    m_accountSeries(0),
    m_seriesTotals(0),
    m_numColumns(0),
    m_skipZero(0),
    m_backgroundBrush(KColorScheme(QPalette::Current).background()),
    m_foregroundBrush(KColorScheme(QPalette::Current).foreground())
{
  // ********************************************************************
  // Set KMyMoney's Chart Parameter Defaults
  // ********************************************************************


  //Set the background obtained from the color scheme
  BackgroundAttributes backAttr(backgroundAttributes());
  backAttr.setBrush(m_backgroundBrush);
  backAttr.setVisible(true);
  setBackgroundAttributes(backAttr);

  //Line diagram
  KDChart::LineDiagram* diagram = new KDChart::LineDiagram;
  diagram->setModel(&m_model);
  this->coordinatePlane()->replaceDiagram(diagram);
}

void KReportChartView::drawPivotChart(const PivotGrid &grid, const MyMoneyReport &config, int numberColumns, const QStringList& columnHeadings, const QList<ERowType>& rowTypeList, const QStringList& columnTypeHeaderList)
{
  //set the number of columns
  setNumColumns(numberColumns);

  //set skipZero
  m_skipZero = config.isSkippingZero();

  //remove existing headers
  while (headerFooters().count() > 0) {
    HeaderFooter* delHeader = headerFooters().at(0);
    takeHeaderFooter(delHeader);
    delete delHeader;
  }

  //make sure the model is clear
  m_model.removeColumns(0, m_model.columnCount());
  m_model.removeRows(0, m_model.rowCount());

  //set the new header
  HeaderFooter* header = new HeaderFooter(this);
  header->setText(config.name());
  header->setType(HeaderFooter::Header);
  header->setPosition(Position::North);
  TextAttributes headerTextAttr(header->textAttributes());
  headerTextAttr.setPen(m_foregroundBrush.color());
  header->setTextAttributes(headerTextAttr);
  addHeaderFooter(header);

  // whether to limit the chart to use series totals only.  Used for reports which only
  // show one dimension (pie).
  setSeriesTotals(false);

  // whether series (rows) are accounts (true) or months (false). This causes a lot
  // of complexity in the charts.  The problem is that circular reports work best with
  // an account in a COLUMN, while line/bar prefer it in a ROW.
  setAccountSeries(true);

  switch (config.chartType()) {
    case MyMoneyReport::eChartNone:
    case MyMoneyReport::eChartEnd:
    case MyMoneyReport::eChartLine: {
        KDChart::LineDiagram* diagram = new KDChart::LineDiagram;

        if (config.isSkippingZero()) {
          LineAttributes attributes = diagram->lineAttributes();
          attributes.setMissingValuesPolicy(LineAttributes::MissingValuesAreBridged);
          diagram->setLineAttributes(attributes);
        }

        CartesianCoordinatePlane* cartesianPlane = new CartesianCoordinatePlane;
        replaceCoordinatePlane(cartesianPlane);
        coordinatePlane()->replaceDiagram(diagram);
        break;
      }
    case MyMoneyReport::eChartBar: {
        KDChart::BarDiagram* diagram = new KDChart::BarDiagram;
        CartesianCoordinatePlane* cartesianPlane = new CartesianCoordinatePlane;
        replaceCoordinatePlane(cartesianPlane);
        coordinatePlane()->replaceDiagram(diagram);
        break;
      }
    case MyMoneyReport::eChartStackedBar: {
        KDChart::BarDiagram* diagram = new KDChart::BarDiagram;
        CartesianCoordinatePlane* cartesianPlane = new CartesianCoordinatePlane;
        replaceCoordinatePlane(cartesianPlane);
        diagram->setType(BarDiagram::Stacked);
        coordinatePlane()->replaceDiagram(diagram);
        break;
      }
    case MyMoneyReport::eChartPie: {
        KDChart::PieDiagram* diagram = new KDChart::PieDiagram;
        PolarCoordinatePlane* polarPlane = new PolarCoordinatePlane;
        replaceCoordinatePlane(polarPlane);
        coordinatePlane()->replaceDiagram(diagram);
        setAccountSeries(false);
        setSeriesTotals(true);
        break;
      }
    case MyMoneyReport::eChartRing: {
        KDChart::RingDiagram* diagram = new KDChart::RingDiagram;
        PolarCoordinatePlane* polarPlane = new PolarCoordinatePlane;
        replaceCoordinatePlane(polarPlane);
        polarPlane->replaceDiagram(diagram);
        //chartView.params()->setRelativeRingThickness( true );
        setAccountSeries(false);
        break;
      }
  }
  //get the diagram for later use
  AbstractDiagram* planeDiagram = coordinatePlane()->diagram();
  planeDiagram->setAntiAliasing(true);

  //set grid attributes
  GridAttributes gridAttr(coordinatePlane()->globalGridAttributes());
  gridAttr.setGridVisible(config.isChartGridLines());
  coordinatePlane()->setGlobalGridAttributes(gridAttr);

  //the palette - we set it here because it is a property of the diagram
  switch (KMyMoneySettings::chartsPalette()) {
    case 0:
      planeDiagram->useDefaultColors();
      break;
    case 1:
      planeDiagram->useRainbowColors();
      break;
    case 2:
    default:
      planeDiagram->useSubduedColors();
      break;
  }

  //the legend will be used later
  Legend* legend = new Legend(planeDiagram, this);
  legend->setTitleText(i18nc("Chart legend title", "Legend"));

  //set up the axes for cartesian diagrams
  if (config.chartType() == MyMoneyReport::eChartLine ||
      config.chartType() == MyMoneyReport::eChartBar ||
      config.chartType() == MyMoneyReport::eChartStackedBar) {
    //set x axis
    CartesianAxis *xAxis = new CartesianAxis();
    xAxis->setPosition(CartesianAxis::Bottom);
    xAxis->setTitleText(i18n("Time"));
    TextAttributes xAxisTitleTextAttr(xAxis->titleTextAttributes());
    xAxisTitleTextAttr.setMinimalFontSize(KGlobalSettings::generalFont().pointSize());
    xAxisTitleTextAttr.setPen(m_foregroundBrush.color());
    xAxis->setTitleTextAttributes(xAxisTitleTextAttr);
    TextAttributes xAxisTextAttr(xAxis->textAttributes());
    xAxisTextAttr.setPen(m_foregroundBrush.color());
    xAxisTextAttr.setAutoRotate(true);
    xAxis->setTextAttributes(xAxisTextAttr);
    RulerAttributes xAxisRulerAttr(xAxis->rulerAttributes());
    xAxisRulerAttr.setTickMarkPen(m_foregroundBrush.color());
    xAxisRulerAttr.setShowRulerLine(true);
    xAxis->setRulerAttributes(xAxisRulerAttr);


    //set y axis
    KBalanceAxis *yAxis = new KBalanceAxis();
    yAxis->setPosition(CartesianAxis::Left);

    // TODO
    // if the chart shows prices and no balance
    // the axis title should be 'Price'
    if (config.isIncludingPrice()) {
      yAxis->setTitleText(i18n("Price"));
    } else {
      yAxis->setTitleText(i18n("Balance"));
    }

    TextAttributes yAxisTitleTextAttr(yAxis->titleTextAttributes());
    yAxisTitleTextAttr.setMinimalFontSize(KGlobalSettings::generalFont().pointSize());
    yAxisTitleTextAttr.setPen(m_foregroundBrush.color());
    yAxis->setTitleTextAttributes(yAxisTitleTextAttr);
    TextAttributes yAxisTextAttr(yAxis->textAttributes());
    yAxisTextAttr.setPen(m_foregroundBrush.color());
    yAxis->setTextAttributes(yAxisTextAttr);
    RulerAttributes yAxisRulerAttr(yAxis->rulerAttributes());
    yAxisRulerAttr.setTickMarkPen(m_foregroundBrush.color());
    yAxisRulerAttr.setShowRulerLine(true);
    yAxis->setRulerAttributes(yAxisRulerAttr);

    //add the axes to the corresponding diagram
    if (config.chartType() == MyMoneyReport::eChartLine) {
      KDChart::LineDiagram* lineDiagram = qobject_cast<LineDiagram*>(planeDiagram);
      lineDiagram->addAxis(xAxis);
      lineDiagram->addAxis(yAxis);
    } else if (config.chartType() == MyMoneyReport::eChartBar ||
               config.chartType() == MyMoneyReport::eChartStackedBar) {
      KDChart::BarDiagram* barDiagram = qobject_cast<BarDiagram*>(planeDiagram);
      barDiagram->addAxis(xAxis);
      barDiagram->addAxis(yAxis);
    }
  }

  switch (config.detailLevel()) {
    case MyMoneyReport::eDetailNone:
    case MyMoneyReport::eDetailEnd:
    case MyMoneyReport::eDetailAll: {
        int rowNum = 0;

        // iterate over outer groups
        PivotGrid::const_iterator it_outergroup = grid.begin();
        while (it_outergroup != grid.end()) {
          // iterate over inner groups
          PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
          while (it_innergroup != (*it_outergroup).end()) {
            //
            // Rows
            //
            QString innergroupdata;
            PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();
            while (it_row != (*it_innergroup).end()) {
              //Do not include investments accounts in the chart because they are merely container of stock and other accounts
              if (it_row.key().accountType() != MyMoneyAccount::Investment) {
                //iterate row types
                for (int i = 0; i < rowTypeList.size(); ++i) {
                  //skip the budget difference rowset
                  if (rowTypeList[i] != eBudgetDiff) {
                    QString legendText;

                    //only show the column type in the header if there is more than one type
                    if (rowTypeList.size() > 1) {
                      legendText = QString(columnTypeHeaderList[i] + " - " + it_row.key().name());
                    } else {
                      legendText = QString(it_row.key().name());
                    }

                    //set the cell value and tooltip
                    rowNum = drawPivotRowSet(rowNum, it_row.value(), rowTypeList[i], legendText, 1, numColumns());

                    //set the legend text
                    legend->setText(rowNum - 1, legendText);
                  }
                }
              }
              ++it_row;
            }
            ++it_innergroup;
          }
          ++it_outergroup;
        }
      }
      break;

    case MyMoneyReport::eDetailTop: {
        int rowNum = 0;

        // iterate over outer groups
        PivotGrid::const_iterator it_outergroup = grid.begin();
        while (it_outergroup != grid.end()) {

          // iterate over inner groups
          PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
          while (it_innergroup != (*it_outergroup).end()) {
            //iterate row types
            for (int i = 0; i < rowTypeList.size(); ++i) {
              //skip the budget difference rowset
              if (rowTypeList[i] != eBudgetDiff) {
                QString legendText;


                //only show the column type in the header if there is more than one type
                if (rowTypeList.size() > 1) {
                  legendText = QString(columnTypeHeaderList[i] + " - " + it_innergroup.key());
                } else {
                  legendText = QString(it_innergroup.key());
                }

                //set the cell value and tooltip
                rowNum = drawPivotRowSet(rowNum, (*it_innergroup).m_total, rowTypeList[i], legendText, 1, numColumns());

                //set the legend text
                legend->setText(rowNum - 1, legendText);
              }
            }
            ++it_innergroup;
          }
          ++it_outergroup;
        }
      }
      break;

    case MyMoneyReport::eDetailGroup: {
        int rowNum = 0;

        // iterate over outer groups
        PivotGrid::const_iterator it_outergroup = grid.begin();
        while (it_outergroup != grid.end()) {
          //iterate row types
          for (int i = 0; i < rowTypeList.size(); ++i) {
            //skip the budget difference rowset
            if (rowTypeList[i] != eBudgetDiff) {
              QString legendText;

              //only show the column type in the header if there is more than one type
              if (rowTypeList.size() > 1) {
                legendText = QString(columnTypeHeaderList[i] + " - " + it_outergroup.key());
              } else {
                legendText = QString(it_outergroup.key());
              }

              //set the cell value and tooltip
              rowNum = drawPivotRowSet(rowNum, (*it_outergroup).m_total, rowTypeList[i], legendText, 1, numColumns());

              //set the legend
              legend->setText(rowNum - 1, legendText);
            }
          }
          ++it_outergroup;
        }

        //if selected, show totals too
        if (config.isShowingRowTotals()) {
          //iterate row types
          for (int i = 0; i < rowTypeList.size(); ++i) {
            //skip the budget difference rowset
            if (rowTypeList[i] != eBudgetDiff) {
              QString legendText;

              //only show the column type in the header if there is more than one type
              if (rowTypeList.size() > 1) {
                legendText = QString(columnTypeHeaderList[i] + " - " + i18nc("Total balance", "Total"));
              } else {
                legendText = QString(i18nc("Total balance", "Total"));
              }

              //set the cell value
              rowNum = drawPivotRowSet(rowNum, grid.m_total, rowTypeList[i], legendText, 1, numColumns());

              //set the legend
              legend->setText(rowNum - 1, legendText);

            }
          }
        }
      }
      break;

    case MyMoneyReport::eDetailTotal: {
        int rowNum = 0;

        //iterate row types
        for (int i = 0; i < rowTypeList.size(); ++i) {
          //skip the budget difference rowset
          if (rowTypeList[i] != eBudgetDiff) {
            QString legendText;

            //only show the column type in the header if there is more than one type
            if (rowTypeList.size() > 1) {
              legendText = QString(columnTypeHeaderList[i] + " - " + i18nc("Total balance", "Total"));
            } else {
              legendText = QString(i18nc("Total balance", "Total"));
            }

            if (config.isMixedTime() && (rowTypeList[i] == eActual || rowTypeList[i] == eForecast)) {
              if (rowTypeList[i] == eActual) {
                rowNum = drawPivotRowSet(rowNum, grid.m_total, rowTypeList[i], legendText, 1, config.currentDateColumn());
              } else if (rowTypeList[i] == eForecast) {
                rowNum = drawPivotRowSet(rowNum, grid.m_total, rowTypeList[i], legendText, config.currentDateColumn(), numColumns());
              } else {
                rowNum = drawPivotRowSet(rowNum, grid.m_total, rowTypeList[i], legendText, 1, numColumns());
              }
            } else {
              //set cell value
              rowNum = drawPivotRowSet(rowNum, grid.m_total, rowTypeList[i], legendText, 1, numColumns());
            }

            //set legend text
            legend->setText(rowNum - 1, legendText);
          }
        }
      }
      break;
  }

  // Set up X axis labels (ie "abscissa" to use the technical term)
  QStringList abscissaNames;
  if (accountSeries()) { // if not, we will set these up while putting in the chart values.
    int column = 1;
    while (column < numColumns()) {
      abscissaNames += QString(columnHeadings[column++]).replace("&nbsp;", " ");
    }
    m_model.setVerticalHeaderLabels(abscissaNames);
  }

  //assign model to the diagram
  planeDiagram->setModel(&m_model);

  //set the legend basic attributes
  //this is done after adding the legend because the values are overridden when adding the legend to the chart
  for (uint i = static_cast<uint>(KMyMoneyGlobalSettings::maximumLegendItems()); i < legend->datasetCount(); ++i) {
    legend->setDatasetHidden(i, true);
  }
  legend->setTitleText(i18nc("Chart lines legend", "Legend"));
  legend->setUseAutomaticMarkerSize(false);
  FrameAttributes legendFrameAttr(legend->frameAttributes());
  legendFrameAttr.setPen(m_foregroundBrush.color());
  // leave some space between the content and the frame
  legendFrameAttr.setPadding(2);
  legend->setFrameAttributes(legendFrameAttr);
  legend->setPosition(Position::East);
  legend->setTextAlignment(Qt::AlignLeft);
  legend->setLegendStyle(KDChart::Legend::MarkersAndLines);
  replaceLegend(legend);

  // set the text attributes after calling replaceLegend() otherwise fon sizes will get overwritten
  qreal generalFontSize = KGlobalSettings::generalFont().pointSizeF();
  if (generalFontSize == -1)
    generalFontSize = 8; // this is a fallback if the fontsize was specified in pixels
  TextAttributes legendTextAttr(legend->textAttributes());
  legendTextAttr.setPen(m_foregroundBrush.color());
  legendTextAttr.setFontSize(KDChart::Measure(generalFontSize, KDChartEnums::MeasureCalculationModeAbsolute));
  legend->setTextAttributes(legendTextAttr);

  TextAttributes legendTitleTextAttr(legend->titleTextAttributes());
  legendTitleTextAttr.setPen(m_foregroundBrush.color());
  legendTitleTextAttr.setFontSize(KDChart::Measure(generalFontSize + 4, KDChartEnums::MeasureCalculationModeAbsolute));
  legend->setTitleTextAttributes(legendTitleTextAttr);

  //this sets the line width only for line diagrams
  setLineWidth(config.chartLineWidth());

  //set data value attributes
  //make sure to show only the required number of fractional digits on the labels of the graph
  DataValueAttributes dataValueAttr(planeDiagram->dataValueAttributes());
  MarkerAttributes markerAttr(dataValueAttr.markerAttributes());
  markerAttr.setVisible(true);
  markerAttr.setMarkerStyle(MarkerAttributes::MarkerCircle);
  markerAttr.setMarkerSize(QSize(8, 8));
  dataValueAttr.setMarkerAttributes(markerAttr);
  TextAttributes dataValueTextAttr(dataValueAttr.textAttributes());
  dataValueTextAttr.setPen(m_foregroundBrush.color());
  dataValueAttr.setTextAttributes(dataValueTextAttr);
  dataValueAttr.setVisible(config.isChartDataLabels());
  dataValueAttr.setDecimalDigits(MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));
  planeDiagram->setDataValueAttributes(dataValueAttr);
  planeDiagram->setAllowOverlappingDataValueTexts(true);

  if (qMin(static_cast<uint>(KMyMoneyGlobalSettings::maximumLegendItems()), legend->datasetCount()) < 2) {
    // the legend is needed only if at least two data sets are rendered
    removeLegend();
  }

  // fix vertical plane range in case only one value is displayed
  CartesianCoordinatePlane* plane = dynamic_cast<CartesianCoordinatePlane*>(coordinatePlane());
  if (plane) {
    QPair<qreal,qreal> range = plane->verticalRange();
    double center = (range.first + range.second) / 2.0;
    if (fabs(range.first - range.second) < 0.01) {
      range.first = center - 1.001;
      range.second = center + 1.001;
      plane->setVerticalRange(range);
    }
  }
}

unsigned KReportChartView::drawPivotRowSet(int rowNum, const PivotGridRowSet& rowSet, const ERowType rowType, const QString& legendText, int startColumn, int endColumn)
{
  //if endColumn is invalid, make it the same as numColumns
  if (endColumn == 0) {
    endColumn = numColumns();
  }

  // Columns
  if (seriesTotals()) {
    double value = rowSet[rowType].m_total.toDouble();

    //set the tooltip
    QString toolTip = QString("<h2>%1</h2><strong>%2</strong><br>")
                      .arg(legendText)
                      .arg(value, 0, 'f', 2);

    if (accountSeries()) {
      //set the cell value
      this->setDataCell(rowNum, 0, value);
      this->setCellTip(rowNum, 0, toolTip);
    } else {
      this->setDataCell(0, rowNum, value);
      this->setCellTip(0, rowNum, toolTip);
    }
  } else {
    int column = startColumn;
    while (column <= endColumn && column < numColumns()) {
      double value = rowSet[rowType][column].toDouble();

      //if zero and set to skip, increase column and continue with next value
      if (m_skipZero && rowSet[rowType][column].isZero()) {
        ++column;
        continue;
      } else {
        QString toolTip = QString("<h2>%1</h2><strong>%2</strong><br>")
                          .arg(legendText)
                          .arg(value, 0, 'f', 2);

        if (accountSeries()) {
          this->setDataCell(column - 1, rowNum, value);
          this->setCellTip(column - 1, rowNum, toolTip);
        } else {
          this->setDataCell(rowNum, column - 1, value);
          this->setCellTip(rowNum, column - 1, toolTip);
        }
      }
      ++column;
    }
  }
  return ++rowNum;
}

void KReportChartView::setDataCell(int row, int column, const double data)
{
  if (coordinatePlane()->diagram()->datasetDimension() != 1)
    return;

  justifyModelSize(row + 1, column + 1);

  const QModelIndex index = m_model.index(row, column);
  m_model.setData(index, QVariant(data), Qt::DisplayRole);
}

void KReportChartView::setCellTip(int row, int column, QString tip)
{
  if (coordinatePlane()->diagram()->datasetDimension() != 1)
    return;

  justifyModelSize(row + 1, column + 1);

  const QModelIndex index = m_model.index(row, column);
  m_model.setData(index, QVariant(tip), Qt::ToolTipRole);
}

/**
 * Justifies the model, so that the given rows and columns fit into it.
 */
void KReportChartView::justifyModelSize(int rows, int columns)
{
  const int currentRows = m_model.rowCount();
  const int currentCols = m_model.columnCount();

  if (currentCols < columns)
    if (! m_model.insertColumns(currentCols, columns - currentCols))
      qDebug() << "justifyModelSize: could not increase model size.";
  if (currentRows < rows)
    if (! m_model.insertRows(currentRows, rows - currentRows))
      qDebug() << "justifyModelSize: could not increase model size.";

  Q_ASSERT(m_model.rowCount() >= rows);
  Q_ASSERT(m_model.columnCount() >= columns);
}

void KReportChartView::setLineWidth(const int lineWidth)
{
  if (qobject_cast<LineDiagram*>(coordinatePlane()->diagram())) {
    LineDiagram* lineDiagram = qobject_cast<LineDiagram*>(coordinatePlane()->diagram());
    QList <QPen> pens;
    pens = lineDiagram->datasetPens();
    for (int i = 0; i < pens.count(); ++i) {
      pens[i].setWidth(lineWidth);
      lineDiagram->setPen(i, pens.at(i));
    }
  }
}

void KReportChartView::drawLimitLine(const double limit)
{
  // temporarily disconnect the view from the model to aovid update of view on
  // emission of the dataChanged() signal for each call of setDataCell().
  // This speeds up the runtime of drawLimitLine() by a factor of
  // approx. 60 on my box (1831ms vs. 31ms).
  AbstractDiagram* planeDiagram = coordinatePlane()->diagram();
  planeDiagram->setModel(0);

  //we get the current number of rows and we add one after that
  int row = m_model.rowCount();

  for (int col = 0; col < m_numColumns; ++col) {
    setDataCell(col, row, limit);
  }

  planeDiagram->setModel(&m_model);

//TODO: add format to the line
}

void KReportChartView::removeLegend()
{
  Legend* chartLegend = Chart::legend();
  delete chartLegend;
}
