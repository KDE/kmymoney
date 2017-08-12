/***************************************************************************
                          kreportchartview.cpp
                             -------------------
    begin                : Sun Aug 14 2005
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
#include <QFontDatabase>
#include <QtMath>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kcolorscheme.h>
#include <KLocalizedString>
#include <math.h>

// ----------------------------------------------------------------------------
// Project Includes
#include <KChartBackgroundAttributes>
#include <KChartDataValueAttributes>
#include <KChartMarkerAttributes>
#include <KChartGridAttributes>
#include <KChartHeaderFooter>
#include <KChartLegend>
#include <KChartLineDiagram>
#include <KChartBarDiagram>
#include <KChartPieDiagram>
#include <KChartRingDiagram>
#include <KChartCartesianAxis>
#include <KChartFrameAttributes>
#include "kmymoneyglobalsettings.h"
#include <kbalanceaxis.h>
#include <mymoneyfile.h>

using namespace reports;

KReportChartView::KReportChartView(QWidget* parent) :
    KChart::Chart(parent),
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
}

void KReportChartView::drawPivotChart(const PivotGrid &grid, const MyMoneyReport &config, int numberColumns, const QStringList& columnHeadings, const QList<ERowType>& rowTypeList, const QStringList& columnTypeHeaderList)
{
  if (numberColumns == 0)
    return;
  //set the number of columns
  setNumColumns(numberColumns);

  //set skipZero
  m_skipZero = config.isSkippingZero();

  //remove existing headers
  const HeaderFooterList hfList = headerFooters();
  foreach (const auto hf, hfList)
    delete hf;

  //remove existing legends
  const LegendList lgList = legends();
  foreach (const auto lg, lgList)
    delete lg;

  //make sure the model is clear
  m_model.clear();
  const bool blocked = m_model.blockSignals(true); // don't emit dataChanged() signal during each drawPivotRowSet

  //set the new header
  HeaderFooter* header = new HeaderFooter;
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
    case MyMoneyReport::eChartLine:
    case MyMoneyReport::eChartBar:
    case MyMoneyReport::eChartStackedBar: {
      CartesianCoordinatePlane* cartesianPlane = new CartesianCoordinatePlane(this);
      cartesianPlane->setAutoAdjustVerticalRangeToData(2);
      replaceCoordinatePlane(cartesianPlane);

      // set-up axis type
      if (config.isLogYAxis())
        cartesianPlane->setAxesCalcModeY(KChart::AbstractCoordinatePlane::Logarithmic);
      else
        cartesianPlane->setAxesCalcModeY(KChart::AbstractCoordinatePlane::Linear);

      QLocale loc = locale();
      // set-up grid
      GridAttributes ga = cartesianPlane->gridAttributes(Qt::Vertical);
      ga.setGridVisible(config.isChartCHGridLines());
      ga.setGridStepWidth(config.isDataUserDefined() ? loc.toDouble(config.dataMajorTick()) : 0.0);
      ga.setGridSubStepWidth(config.isDataUserDefined() ? loc.toDouble(config.dataMinorTick()) : 0.0);
      cartesianPlane->setGridAttributes(Qt::Vertical, ga);

      ga = cartesianPlane->gridAttributes(Qt::Horizontal);
      ga.setGridVisible(config.isChartSVGridLines());
      cartesianPlane->setGridAttributes(Qt::Horizontal, ga);

      // set-up data range
      cartesianPlane->setVerticalRange(qMakePair(config.isDataUserDefined() ? loc.toDouble(config.dataRangeStart()) : 0.0,
                                                 config.isDataUserDefined() ? loc.toDouble(config.dataRangeEnd()) : 0.0));

      //set-up x axis
      CartesianAxis *xAxis = new CartesianAxis();
      xAxis->setPosition(CartesianAxis::Bottom);
      xAxis->setTitleText(i18n("Time"));
      TextAttributes xAxisTitleTextAttr(xAxis->titleTextAttributes());
      xAxisTitleTextAttr.setMinimalFontSize(QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize());
      xAxisTitleTextAttr.setPen(m_foregroundBrush.color());
      xAxis->setTitleTextAttributes(xAxisTitleTextAttr);
      TextAttributes xAxisTextAttr(xAxis->textAttributes());
      xAxisTextAttr.setPen(m_foregroundBrush.color());
      xAxis->setTextAttributes(xAxisTextAttr);
      RulerAttributes xAxisRulerAttr(xAxis->rulerAttributes());
      xAxisRulerAttr.setTickMarkPen(m_foregroundBrush.color());
      xAxisRulerAttr.setShowRulerLine(true);
      xAxis->setRulerAttributes(xAxisRulerAttr);

      //set-up y axis
      KBalanceAxis *yAxis = new KBalanceAxis();
      yAxis->setPosition(CartesianAxis::Left);

      if (config.isIncludingPrice())
        yAxis->setTitleText(i18n("Price"));
      else
        yAxis->setTitleText(i18n("Balance"));

      TextAttributes yAxisTitleTextAttr(yAxis->titleTextAttributes());
      yAxisTitleTextAttr.setMinimalFontSize(QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize());
      yAxisTitleTextAttr.setPen(m_foregroundBrush.color());
      yAxis->setTitleTextAttributes(yAxisTitleTextAttr);
      TextAttributes yAxisTextAttr(yAxis->textAttributes());
      yAxisTextAttr.setPen(m_foregroundBrush.color());
      yAxis->setTextAttributes(yAxisTextAttr);
      RulerAttributes yAxisRulerAttr(yAxis->rulerAttributes());
      yAxisRulerAttr.setTickMarkPen(m_foregroundBrush.color());
      yAxisRulerAttr.setShowRulerLine(true);
      yAxis->setRulerAttributes(yAxisRulerAttr);

      switch (config.chartType()) {
        case MyMoneyReport::eChartEnd:
        case MyMoneyReport::eChartLine: {
            KChart::LineDiagram* diagram = new KChart::LineDiagram(this, cartesianPlane);

            if (config.isSkippingZero()) {
              LineAttributes attributes = diagram->lineAttributes();
              attributes.setMissingValuesPolicy(LineAttributes::MissingValuesAreBridged);
              diagram->setLineAttributes(attributes);
            }
            cartesianPlane->replaceDiagram(diagram);
            diagram->addAxis(xAxis);
            diagram->addAxis(yAxis);
            break;
          }
        case MyMoneyReport::eChartBar: {
            KChart::BarDiagram* diagram = new KChart::BarDiagram(this, cartesianPlane);
            cartesianPlane->replaceDiagram(diagram);
            diagram->addAxis(xAxis);
            diagram->addAxis(yAxis);
            break;
          }
        case MyMoneyReport::eChartStackedBar: {
            KChart::BarDiagram* diagram = new KChart::BarDiagram(this, cartesianPlane);
            diagram->setType(BarDiagram::Stacked);
            cartesianPlane->replaceDiagram(diagram);
            diagram->addAxis(xAxis);
            diagram->addAxis(yAxis);
            break;
          }
      default:
        break;
      }
      break;
    }
    case MyMoneyReport::eChartPie:
    case MyMoneyReport::eChartRing:{
      PolarCoordinatePlane* polarPlane = new PolarCoordinatePlane(this);
      replaceCoordinatePlane(polarPlane);

      // set-up grid
      GridAttributes ga = polarPlane->gridAttributes(true);
      ga.setGridVisible(config.isChartCHGridLines());
      polarPlane->setGridAttributes(true, ga);

      ga = polarPlane->gridAttributes(false);
      ga.setGridVisible(config.isChartSVGridLines());
      polarPlane->setGridAttributes(false, ga);

      setAccountSeries(false);

      switch (config.chartType()) {
        case MyMoneyReport::eChartPie: {
            KChart::PieDiagram* diagram = new KChart::PieDiagram(this, polarPlane);
            polarPlane->replaceDiagram(diagram);
            setSeriesTotals(true);
            break;
          }
        case MyMoneyReport::eChartRing: {
            KChart::RingDiagram* diagram = new KChart::RingDiagram(this, polarPlane);
            polarPlane->replaceDiagram(diagram);
            break;
          }
      default:
        break;
      }

      break;
    }
  default:  // no valid chart types
    return;
  }
  //get the coordinate plane  and the diagram for later use
  AbstractCoordinatePlane* cPlane = coordinatePlane();
  AbstractDiagram* planeDiagram = cPlane->diagram();
  planeDiagram->setAntiAliasing(true);

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

  int eBudgetDiffIdx = rowTypeList.indexOf(eBudgetDiff);
  QList<ERowType> myRowTypeList = rowTypeList;
  myRowTypeList.removeAt(eBudgetDiffIdx);
  QStringList myColumnTypeHeaderList = columnTypeHeaderList;
  myColumnTypeHeaderList.removeAt(eBudgetDiffIdx);
  int myRowTypeListSize = myRowTypeList.size();
  MyMoneyFile* file = MyMoneyFile::instance();
  int precision = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
  int rowNum = 0;
  QStringList legendNames;

  switch (config.detailLevel()) {
    case MyMoneyReport::eDetailNone:
    case MyMoneyReport::eDetailAll: {
        // iterate over outer groups
        PivotGrid::const_iterator it_outergroup = grid.begin();
        while (it_outergroup != grid.end()) {
          // iterate over inner groups
          PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
          while (it_innergroup != (*it_outergroup).end()) {
            // iterate over accounts
            PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();
            while (it_row != (*it_innergroup).end()) {
              //Do not include investments accounts in the chart because they are merely container of stock and other accounts
              if (it_row.key().accountType() != MyMoneyAccount::Investment) {

                // get displayed precision
                int currencyPrecision = precision;
                int securityPrecision = precision;
                if (!it_row.key().id().isEmpty()) {
                  const MyMoneyAccount acc = file->account(it_row.key().id());
                  if (acc.isInvest()) {
                    securityPrecision = file->currency(acc.currencyId()).pricePrecision();
                    // stock account isn't eveluated in currency, so take investment account instead
                    currencyPrecision = MyMoneyMoney::denomToPrec(file->account(acc.parentAccountId()).fraction());
                  } else
                    currencyPrecision = MyMoneyMoney::denomToPrec(acc.fraction());
                }

                // iterate row types
                for (int i = 0 ; i < myRowTypeListSize; ++i) {
                  QString legendText;

                  //only show the column type in the header if there is more than one type
                  if (myRowTypeListSize > 1)
                    legendText = QString(myColumnTypeHeaderList.at(i) + QLatin1Literal(" - ") + it_row.key().name());
                  else
                    legendText = it_row.key().name();

                  //set the legend text
                  legendNames.append(legendText);

                  precision = myRowTypeList.at(i) == ePrice ? securityPrecision : currencyPrecision;

                  //set the cell value and tooltip
                  rowNum = drawPivotGridRow(rowNum, it_row.value().value(myRowTypeList.at(i)),
                                            config.isChartDataLabels() ? legendText : QString(),
                                            0, numberColumns, precision);
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
        // iterate over outer groups
        PivotGrid::const_iterator it_outergroup = grid.begin();
        while (it_outergroup != grid.end()) {

          // iterate over inner groups
          PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
          while (it_innergroup != (*it_outergroup).end()) {
            // iterate row types
            for (int i = 0 ; i < myRowTypeListSize; ++i) {
              QString legendText;

              //only show the column type in the header if there is more than one type
              if (myRowTypeListSize > 1)
                legendText = QString(myColumnTypeHeaderList.at(i) + QLatin1Literal(" - ") + it_innergroup.key());
              else
                legendText = it_innergroup.key();

              //set the legend text
              legendNames.append(legendText);

              //set the cell value and tooltip
              rowNum = drawPivotGridRow(rowNum, (*it_innergroup).m_total.value(myRowTypeList.at(i)),
                                        config.isChartDataLabels() ? legendText : QString(),
                                        0, numberColumns, precision);
            }
            ++it_innergroup;
          }
          ++it_outergroup;
        }
      }
      break;

    case MyMoneyReport::eDetailGroup: {
        // iterate over outer groups
        PivotGrid::const_iterator it_outergroup = grid.begin();
        while (it_outergroup != grid.end()) {
          // iterate row types
          for (int i = 0 ; i < myRowTypeListSize; ++i) {
            QString legendText;

            //only show the column type in the header if there is more than one type
            if (myRowTypeListSize > 1)
              legendText = QString(myColumnTypeHeaderList.at(i) + QLatin1Literal(" - ") + it_outergroup.key());
            else
              legendText = it_outergroup.key();

            //set the legend text
            legendNames.append(legendText);

            //set the cell value and tooltip
            rowNum = drawPivotGridRow(rowNum, (*it_outergroup).m_total.value(myRowTypeList.at(i)),
                                      config.isChartDataLabels() ? legendText : QString(),
                                      0, numberColumns, precision);
          }
          ++it_outergroup;
        }

        //if selected, show totals too
        if (config.isShowingRowTotals()) {
          // iterate row types
          for (int i = 0 ; i < myRowTypeListSize; ++i) {
            QString legendText;

            //only show the column type in the header if there is more than one type
            if (myRowTypeListSize > 1)
              legendText = QString(myColumnTypeHeaderList.at(i) + QLatin1Literal(" - ") + i18nc("Total balance", "Total"));
            else
              legendText = QString(i18nc("Total balance", "Total"));

            //set the legend text
            legendNames.append(legendText);

            //set the cell value and tooltip
            rowNum = drawPivotGridRow(rowNum, grid.m_total.value(myRowTypeList.at(i)),
                                      config.isChartDataLabels() ? legendText : QString(),
                                      0, numberColumns, precision);
          }
        }
      }
      break;

    case MyMoneyReport::eDetailTotal: {
        // iterate row types
        for (int i = 0 ; i < myRowTypeListSize; ++i) {
          QString legendText;

          //only show the column type in the header if there is more than one type
          if (myRowTypeListSize > 1)
            legendText = QString(myColumnTypeHeaderList.at(i) + QLatin1Literal(" - ") + i18nc("Total balance", "Total"));
          else
            legendText = QString(i18nc("Total balance", "Total"));

          //set the legend text
          legendNames.append(legendText);

          //set the cell value and tooltip
          if (config.isMixedTime()) {
            if (myRowTypeList.at(i) == eActual)
              rowNum = drawPivotGridRow(rowNum, grid.m_total.value(myRowTypeList.at(i)),
                                        config.isChartDataLabels() ? legendText : QString(),
                                        0, config.currentDateColumn(), precision);
            else if (myRowTypeList.at(i)== eForecast) {
              rowNum = drawPivotGridRow(rowNum, grid.m_total.value(myRowTypeList.at(i)),
                                        config.isChartDataLabels() ? legendText : QString(),
                                        config.currentDateColumn(), numberColumns - config.currentDateColumn(), precision);

            } else
              rowNum = drawPivotGridRow(rowNum, grid.m_total.value(myRowTypeList.at(i)),
                                        config.isChartDataLabels() ? legendText : QString(),
                                        0, numberColumns, precision);
          } else
            rowNum = drawPivotGridRow(rowNum, grid.m_total.value(myRowTypeList.at(i)),
                                      config.isChartDataLabels() ? legendText : QString(),
                                      0, numberColumns, precision);
        }

      }
      break;
  default:
    case MyMoneyReport::eDetailEnd:
    return;
  }

  // Set up X axis labels (ie "abscissa" to use the technical term)
  if (accountSeries()) { // if not, we will set these up while putting in the chart values.
    QStringList xLabels;
    foreach (const auto colHeading, columnHeadings)
      xLabels.append(QString(colHeading).replace(QLatin1String("&nbsp;"), QLatin1String(" ")));
    m_model.setVerticalHeaderLabels(xLabels);
  }
  m_model.setHorizontalHeaderLabels(legendNames);

  // set line width for line chart
  if (config.chartType() == MyMoneyReport::eChartLine) {
    AttributesModel* diagramAttributes = planeDiagram->attributesModel();
    int penWidth = config.chartLineWidth();
    for (int i = 0 ; i < rowNum ; ++i) {
      QPen pen = diagramAttributes->headerData(i, Qt::Horizontal, DatasetPenRole).value< QPen >();
      pen.setWidth(penWidth);
      m_model.setHeaderData(i, Qt::Horizontal, qVariantFromValue(pen), DatasetPenRole);
    }
  }

  // the legend is needed only if at least two data sets are rendered
  if (qMin(static_cast<int>(KMyMoneyGlobalSettings::maximumLegendItems()), rowNum) > 1) {
    //the legend will be used later
    Legend* legend = new Legend(planeDiagram, this);
    legend->setTitleText(i18nc("Chart legend title", "Legend"));

    //set the legend basic attributes
    //this is done after adding the legend because the values are overridden when adding the legend to the chart
    for (int i = static_cast<int>(KMyMoneyGlobalSettings::maximumLegendItems()); i < rowNum; ++i)
      legend->setDatasetHidden(i, true);

    legend->setUseAutomaticMarkerSize(false);
    FrameAttributes legendFrameAttr(legend->frameAttributes());
    legendFrameAttr.setPen(m_foregroundBrush.color());
    // leave some space between the content and the frame
    legendFrameAttr.setPadding(2);
    legend->setFrameAttributes(legendFrameAttr);
    legend->setPosition(Position::East);
    legend->setTextAlignment(Qt::AlignLeft);
    if (config.isChartDataLabels())
      legend->setLegendStyle(KChart::Legend::MarkersAndLines);
    else
      legend->setLegendStyle(KChart::Legend::LinesOnly);
    replaceLegend(legend);

    // set the text attributes after calling replaceLegend() otherwise fon sizes will get overwritten
    qreal generalFontSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSizeF();
    if (generalFontSize == -1)
      generalFontSize = 8; // this is a fallback if the fontsize was specified in pixels
    TextAttributes legendTextAttr(legend->textAttributes());
    legendTextAttr.setPen(m_foregroundBrush.color());
    legendTextAttr.setFontSize(KChart::Measure(generalFontSize, KChartEnums::MeasureCalculationModeAbsolute));
    legend->setTextAttributes(legendTextAttr);

    TextAttributes legendTitleTextAttr(legend->titleTextAttributes());
    legendTitleTextAttr.setPen(m_foregroundBrush.color());
    legendTitleTextAttr.setFontSize(KChart::Measure(generalFontSize + 4, KChartEnums::MeasureCalculationModeAbsolute));
    legend->setTitleTextAttributes(legendTitleTextAttr);
  }

  //set data value attributes
  //make sure to show only the required number of fractional digits on the labels of the graph
  DataValueAttributes dataValueAttr(planeDiagram->dataValueAttributes());
  MarkerAttributes markerAttr(dataValueAttr.markerAttributes());
  markerAttr.setVisible(true);
  markerAttr.setMarkerStyle(MarkerAttributes::MarkerCircle);
  dataValueAttr.setMarkerAttributes(markerAttr);
  TextAttributes dataValueTextAttr(dataValueAttr.textAttributes());
  dataValueTextAttr.setPen(m_foregroundBrush.color());
  dataValueAttr.setTextAttributes(dataValueTextAttr);
  m_precision = config.yLabelsPrecision();
  dataValueAttr.setDecimalDigits(config.yLabelsPrecision());
  dataValueAttr.setVisible(config.isChartDataLabels());
  planeDiagram->setDataValueAttributes(dataValueAttr);
  planeDiagram->setAllowOverlappingDataValueTexts(true);

  m_model.blockSignals(blocked); // reenable dataChanged() signal

  //assign model to the diagram
  planeDiagram->setModel(&m_model);

  // connect needLayoutPlanes, so dimension of chart can be known, so custom Y labels can be generated
  connect(cPlane, SIGNAL(needLayoutPlanes()), this, SLOT(slotNeedUpdate()));
}
void KReportChartView::slotNeedUpdate()
{
  disconnect(coordinatePlane(), SIGNAL(needLayoutPlanes()), this, SLOT(slotNeedUpdate())); // this won't cause hang-up in KReportsView::slotConfigure
  QList<DataDimension> grids = coordinatePlane()->gridDimensionsList();
  if (grids.isEmpty())  // ring and pie charts have no dimensions
    return;

  if (grids.at(1).stepWidth == 0) // no labels?
    return;
  QLocale loc = locale();
  QChar separator = loc.groupSeparator();
  QChar decimalPoint = loc.decimalPoint();

  QStringList labels;
  if (m_precision > 10 || m_precision <= 0) // assure that conversion through QLocale::toString() will always work
    m_precision = 1;

  CartesianCoordinatePlane* cartesianplane = qobject_cast<CartesianCoordinatePlane*>(coordinatePlane());
  if (cartesianplane) {
    if (cartesianplane->axesCalcModeY() == KChart::AbstractCoordinatePlane::Logarithmic) {
      qreal labelValue = qFloor(log10(grids.at(1).start)); // first label is 10 to power of labelValue
      int labelCount = qFloor(log10(grids.at(1).end)) - qFloor(log10(grids.at(1).start)) + 1;
      for (auto i = 0; i < labelCount; ++i) {
        labels.append(loc.toString(qPow(10.0, labelValue), 'f', m_precision).remove(separator).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\" + decimalPoint + "$")));
        ++labelValue; // next label is 10 to power of previous exponent + 1
      }
    } else {
      qreal labelValue = grids.at(1).start; // first label is start value
      qreal step = grids.at(1).stepWidth;
      int labelCount = qFloor((grids.at(1).end - grids.at(1).start) / grids.at(1).stepWidth) + 1;
      for (auto i = 0; i < labelCount; ++i) {
        labels.append(loc.toString(labelValue, 'f', m_precision).remove(separator).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\" + decimalPoint + "$")));
        labelValue += step; // next label is previous value + step value
      }
    }
  } else
    return; // nothing but cartesian plane is handled

  KChart::LineDiagram* lineDiagram = qobject_cast<LineDiagram*>(coordinatePlane()->diagram());
  if (lineDiagram)
    lineDiagram->axes().at(1)->setLabels(labels);

  KChart::BarDiagram* barDiagram = qobject_cast<BarDiagram*>(coordinatePlane()->diagram());
  if (barDiagram)
    barDiagram->axes().at(1)->setLabels(labels);
}

int KReportChartView::drawPivotGridRow(int rowNum, const PivotGridRow& gridRow, const QString& legendText, const int startColumn, const int columnsToDraw, const int precision)
{
  // Columns
  QString toolTip = QString(QLatin1Literal("<h2>%1</h2><strong>%2</strong><br>")).arg(legendText);
  bool isToolTip = !legendText.isEmpty();
  if (seriesTotals()) {
    QStandardItem* item = new QStandardItem();
    double value = gridRow.m_total.toDouble();
    item->setData(QVariant(value), Qt::DisplayRole);
    if (isToolTip)
      item->setToolTip(toolTip.arg(value, 0, 'f', precision));

    //set the cell value
    if (accountSeries()) {
      m_model.insertRows(rowNum, 1);
      m_model.setItem(rowNum, 0, item);
    } else {
      m_model.insertColumns(rowNum, 1);
      m_model.setItem(0, rowNum, item);
    }

  } else {
    QList<QStandardItem*> itemList;
    for (int i = startColumn; i < columnsToDraw; ++i) {
      QStandardItem* item = new QStandardItem();
      if (!m_skipZero || !gridRow.at(i).isZero()) {
        double value = gridRow.at(i).toDouble();
        item->setData(QVariant(value), Qt::DisplayRole);
        if (isToolTip)
          item->setToolTip(toolTip.arg(value, 0, 'f', precision));
      }
      itemList.append(item);
    }
    if (accountSeries())
      m_model.appendColumn(itemList);
    else
      m_model.appendRow(itemList);
  }
  return ++rowNum;
}

void KReportChartView::setDataCell(int row, int column, const double value, QString tip)
{
  QMap<int, QVariant> data;
  data.insert(Qt::DisplayRole, QVariant(value));
  if (!tip.isEmpty())
    data.insert(Qt::ToolTipRole, QVariant(tip));
  const QModelIndex index = m_model.index(row, column);
  m_model.setItemData(index, data);
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
  LineDiagram* lineDiagram = qobject_cast<LineDiagram*>(coordinatePlane()->diagram());
  if (lineDiagram) {
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
  if (coordinatePlane()->diagram()->datasetDimension() != 1)
    return;
  // temporarily disconnect the view from the model to aovid update of view on
  // emission of the dataChanged() signal for each call of setDataCell().
  // This speeds up the runtime of drawLimitLine() by a factor of
  // approx. 60 on my box (1831ms vs. 31ms).
  AbstractDiagram* planeDiagram = coordinatePlane()->diagram();
  planeDiagram->setModel(0);

  //we get the current number of rows and we add one after that
  int row = m_model.rowCount();
  justifyModelSize(m_numColumns, row + 1);
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
