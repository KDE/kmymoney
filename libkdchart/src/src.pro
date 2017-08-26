# Subdir relative project main directory: ./src

# Target is a library:  kdchart
TEMPLATE = lib
# Use the filename "kdchartd.dll" (or "kdchartd.lib") on Windows
# to avoid name clashes between debug/non-debug versions of the
# KD Chart library:
TARGET = kdchart
CONFIG(debug, debug|release) {
    !unix: TARGET = kdchartd
}

include( $${TOP_SOURCE_DIR}/variables.pri )

DEFINES += KDCHART_BUILD_KDCHART_LIB

RESOURCES += KDChart/KDAB_kdchart_LeveyJennings_resources.qrc

QT += svg

contains(QT_VERSION, ^5\\.[0-9]\\..*): QT += printsupport

FORMS += KDChart/KDChartDatasetSelector.ui

INSTALLHEADERS_INCLUDE = \
    KDChart/KDChartAbstractAxis \
    KDChart/KDChartAbstractCoordinatePlane \
    KDChart/KDChartAbstractDiagram \
    KDChart/KDChartAbstractProxyModel \
    KDChart/KDChartAbstractThreeDAttributes \
    KDChart/KDChartAttributesModel \
    KDChart/KDChartBackgroundAttributes \
    KDChart/KDChartChart \
    KDChart/KDChartDatasetProxyModel \
    KDChart/KDChartDatasetSelector \
    KDChart/KDChartDataValueAttributes \
    KDChart/KDChartDiagramObserver \
    KDChart/KDChartEnums \
    KDChart/KDChartFrameAttributes \
    KDChart/KDChartGlobal \
    KDChart/KDChartGridAttributes \
    KDChart/KDChartHeaderFooter \
    KDChart/KDChartLegend \
    KDChart/KDChartLineAttributes \
    KDChart/KDChartMarkerAttributes \
    KDChart/KDChartMeasure \
    KDChart/KDChartPaintContext \
    KDChart/KDChartPalette \
    KDChart/KDChartPosition \
    KDChart/KDChartRelativePosition \
    KDChart/KDChartTextAttributes \
    KDChart/KDChartThreeDLineAttributes \
    KDChart/KDChartValueTrackerAttributes \
    KDChart/KDChartWidget \
    KDChart/KDChartZoomParameters \
    KDChart/KDTextDocument \
    KDChart/KDChartAbstractCartesianDiagram \
    KDChart/KDChartAbstractTernaryDiagram \
    KDChart/KDChartBarAttributes \
    KDChart/KDChartStockBarAttributes \
    KDChart/KDChartStockDiagram \
    KDChart/KDChartLineDiagram \
    KDChart/KDChartBarDiagram \
    KDChart/KDChartThreeDBarAttributes \
    KDChart/KDChartCartesianAxis \
    KDChart/KDChartCartesianCoordinatePlane \
    KDChart/KDChartPlotter \
    KDChart/KDChartLeveyJenningsAxis \
    KDChart/KDChartLeveyJenningsCoordinatePlane \
    KDChart/KDChartLeveyJenningsDiagram \
    KDChart/KDChartLeveyJenningsGrid \
    KDChart/KDChartLeveyJenningsGridAttributes \
    KDChart/KDChartAbstractPolarDiagram \
    KDChart/KDChartAbstractPieDiagram \
    KDChart/KDChartPolarCoordinatePlane \
    KDChart/KDChartPolarDiagram \
    KDChart/KDChartPieDiagram \
    KDChart/KDChartPieAttributes \
    KDChart/KDChartThreeDPieAttributes \
    KDChart/KDChartRingDiagram \
    KDChart/KDChartRadarCoordinatePlane \
    KDChart/KDChartRadarDiagram \
    KDChart/KDChartTernaryAxis \
    KDChart/KDChartTernaryCoordinatePlane \
    KDChart/KDChartTernaryLineDiagram \
    KDChart/KDChartTernaryPointDiagram \
    KDGanttAbstractGrid \
    KDGanttAbstractRowController \
    KDGanttConstraint \
    KDGanttConstraintGraphicsItem \
    KDGanttConstraintModel \
    KDGanttDateTimeGrid \
    KDGanttForwardingProxyModel \
    KDGanttGlobal \
    KDGanttGraphicsItem \
    KDGanttGraphicsView \
    KDGanttGraphicsScene \
    KDGanttItemDelegate \
    KDGanttLegend \
    KDGanttListViewRowController \
    KDGanttStyleOptionGanttItem \
    KDGanttSummaryHandlingProxyModel \
    KDGanttTreeViewRowController \
    KDGanttView \
    KDGanttProxyModel \

INSTALLHEADERS_SRC = \
           kdchart_export.h \
           KDChartGlobal.h \
           KDChartMeasure.h \
           KDChartAbstractCoordinatePlane.h \
           KDChartChart.h \
           KDChartWidget.h \
           KDChartAbstractDiagram.h \
           KDChartAbstractAreaBase.h \
           KDChartAbstractArea.h \
           KDChartTextArea.h \
           KDChartAbstractAreaWidget.h \
           KDChartAbstractAxis.h \
           KDChartAbstractProxyModel.h \
           KDChartAbstractGrid.h \
           KDChartAttributesModel.h \
           KDChartBackgroundAttributes.h \
           KDChartDatasetProxyModel.h \
           KDChartDatasetSelector.h \
           KDChartDataValueAttributes.h \
           KDChartDiagramObserver.h \
           KDChartEnums.h \
           KDChartFrameAttributes.h \
           KDChartGridAttributes.h \
           KDChartRulerAttributes.h \
           KDChartHeaderFooter.h \
           KDChartLayoutItems.h \
           KDChartLegend.h \
           KDChartLineAttributes.h \
           KDChartMarkerAttributes.h \
           KDChartPaintContext.h \
           KDChartPalette.h \
           KDChartPosition.h \
           KDChartRelativePosition.h \
           KDChartTextAttributes.h \
           KDTextDocument.h \
           KDChartAbstractThreeDAttributes.h \
           KDChartThreeDLineAttributes.h \
           KDChartTextLabelCache.h \
           KDChartValueTrackerAttributes.h \
           KDChartPrintingParameters.h \
           Cartesian/CartesianCoordinateTransformation.h \
           Cartesian/KDChartAbstractCartesianDiagram.h \
           Cartesian/KDChartCartesianCoordinatePlane.h \
           Cartesian/KDChartCartesianGrid.h \
           Cartesian/KDChartCartesianAxis.h \
           Cartesian/KDChartCartesianDiagramDataCompressor.h \
           Cartesian/KDChartBarDiagram.h \
           Cartesian/KDChartBarAttributes.h \
           Cartesian/KDChartStockBarAttributes.h \
           Cartesian/KDChartStockDiagram.h \
           Cartesian/KDChartThreeDBarAttributes.h \
           Cartesian/KDChartLeveyJenningsCoordinatePlane.h \
           Cartesian/KDChartLeveyJenningsDiagram.h \
           Cartesian/KDChartLeveyJenningsGrid.h \
           Cartesian/KDChartLeveyJenningsGridAttributes.h \
           Cartesian/KDChartLeveyJenningsAxis.h \
           Cartesian/KDChartLineDiagram.h \
           Cartesian/KDChartPlotter.h \
           Ternary/TernaryPoint.h \
           Ternary/TernaryConstants.h \
           Ternary/KDChartTernaryCoordinatePlane.h \
           Ternary/KDChartTernaryGrid.h \
           Ternary/KDChartTernaryAxis.h \
           Ternary/KDChartAbstractTernaryDiagram.h \
           Ternary/KDChartTernaryPointDiagram.h \
           Ternary/KDChartTernaryLineDiagram.h \
           Polar/KDChartPolarCoordinatePlane.h \
           Polar/KDChartPolarGrid.h \
           Polar/KDChartAbstractPolarDiagram.h \
           Polar/KDChartPolarDiagram.h \
           Polar/KDChartAbstractPieDiagram.h \
           Polar/KDChartPieDiagram.h \
           Polar/KDChartPieAttributes.h \
           Polar/KDChartThreeDPieAttributes.h \
           Polar/KDChartRadarDiagram.h \
           Polar/KDChartRingDiagram.h \

INSTALLHEADERS_GANTT_SRC = \
           KDGantt/kdgantt_export.h \
           KDGantt/kdganttglobal.h \
           KDGantt/kdganttabstractrowcontroller.h \
           KDGantt/kdgantttreeviewrowcontroller.h \
           KDGantt/kdganttlistviewrowcontroller.h \
           KDGantt/kdganttview.h \
           KDGantt/kdganttstyleoptionganttitem.h \
           KDGantt/kdganttgraphicsview.h \
           KDGantt/kdganttgraphicsscene.h \
           KDGantt/kdganttgraphicsitem.h \
           KDGantt/kdganttconstraint.h \
           KDGantt/kdganttconstraintproxy.h \
           KDGantt/kdganttconstraintgraphicsitem.h \
           KDGantt/kdganttitemdelegate.h \
           KDGantt/kdganttforwardingproxymodel.h \
           KDGantt/kdganttsummaryhandlingproxymodel.h \
           KDGantt/kdganttproxymodel.h \
           KDGantt/kdganttconstraintmodel.h \
           KDGantt/kdganttabstractgrid.h \
           KDGantt/kdganttdatetimegrid.h \
           KDGantt/kdganttlegend.h \

# installation targets:
headers_include.files = $$INSTALLHEADERS_INCLUDE
headers_include.path = $$INSTALL_PREFIX/include
headers_src.files = $$INSTALLHEADERS_SRC
headers_src.path = $$INSTALL_PREFIX/src
headers_gantt_src.files = $$INSTALLHEADERS_GANTT_SRC
headers_gantt_src.path = $$INSTALL_PREFIX/src/Gantt
#INSTALLS += headers_include headers_src headers_gantt_src

# install target to install the src code for license holders:
lib.files = $${DESTDIR}
lib.path = $$INSTALL_PREFIX/
INSTALLS += lib



HEADERS += KDChart/KDChartGlobal.h \
           KDChart/KDChartMeasure.h \
           KDChart/KDChartAbstractCoordinatePlane.h \
           KDChart/KDChartAbstractCoordinatePlane_p.h \
           KDChart/KDChartChart.h \
           KDChart/KDChartChart_p.h \
           KDChart/KDChartWidget.h \
           KDChart/KDChartWidget_p.h \
           KDChart/KDChartAbstractDiagram.h \
           KDChart/KDChartAbstractDiagram_p.h \
           KDChart/KDChartAbstractAreaBase.h \
           KDChart/KDChartAbstractAreaBase_p.h \
           KDChart/KDChartAbstractArea.h \
           KDChart/KDChartAbstractArea_p.h \
           KDChart/KDChartTextArea.h \
           KDChart/KDChartTextArea_p.h \
           KDChart/KDChartAbstractAreaWidget.h \
           KDChart/KDChartAbstractAreaWidget_p.h \
           KDChart/KDChartAbstractAxis.h \
           KDChart/KDChartAbstractProxyModel.h \
           KDChart/KDChartAbstractGrid.h \
           KDChart/KDChartAttributesModel.h \
           KDChart/KDChartBackgroundAttributes.h \
           KDChart/KDChartDatasetProxyModel.h \
           KDChart/KDChartDatasetSelector.h \
           KDChart/KDChartDataValueAttributes.h \
           KDChart/KDChartDiagramObserver.h \
           KDChart/KDChartEnums.h \
           KDChart/KDChartFrameAttributes.h \
           KDChart/KDChartGridAttributes.h \
           KDChart/KDChartRulerAttributes.h \
           KDChart/KDChartHeaderFooter.h \
           KDChart/KDChartHeaderFooter_p.h \
           KDChart/KDChartLayoutItems.h \
           KDChart/KDChartLegend.h \
           KDChart/KDChartLegend_p.h \
           KDChart/KDChartLineAttributes.h \
           KDChart/KDChartMarkerAttributes.h \
           KDChart/KDChartPaintContext.h \
           KDChart/KDChartPalette.h \
           KDChart/KDChartPosition.h \
           KDChart/KDChartRelativePosition.h \
           KDChart/KDChartTextAttributes.h \
           KDChart/KDTextDocument.h \
           KDChart/KDChartAbstractThreeDAttributes.h \
           KDChart/KDChartAbstractThreeDAttributes_p.h \
           KDChart/KDChartThreeDLineAttributes.h \
           KDChart/KDChartThreeDLineAttributes_p.h \
           KDChart/KDChartTextLabelCache.h \
           KDChart/ChartGraphicsItem.h \
           KDChart/ReverseMapper.h \
           KDChart/KDChartValueTrackerAttributes.h \
           KDChart/KDChartPrintingParameters.h \
           KDChart/KDChartModelDataCache_p.h \
           KDChart/Cartesian/CartesianCoordinateTransformation.h \
           KDChart/Cartesian/KDChartAbstractCartesianDiagram.h \
           KDChart/Cartesian/KDChartAbstractCartesianDiagram_p.h \
           KDChart/Cartesian/KDChartCartesianCoordinatePlane.h \
           KDChart/Cartesian/KDChartCartesianCoordinatePlane_p.h \
           KDChart/Cartesian/KDChartCartesianGrid.h \
           KDChart/Cartesian/KDChartBarAttributes.h \
           KDChart/Cartesian/KDChartStockBarAttributes.h \
           KDChart/Cartesian/KDChartStockDiagram.h \
           KDChart/Cartesian/KDChartStockDiagram_p.h \
           KDChart/Cartesian/KDChartBarDiagram.h \
           KDChart/Cartesian/KDChartBarDiagram_p.h \
           KDChart/Cartesian/KDChartCartesianAxis.h \
           KDChart/Cartesian/KDChartCartesianAxis_p.h \
           KDChart/Cartesian/KDChartLineDiagram.h \
           KDChart/Cartesian/KDChartLineDiagram_p.h \
           KDChart/Cartesian/KDChartCartesianDiagramDataCompressor_p.h \
           KDChart/Cartesian/KDChartThreeDBarAttributes.h \
           KDChart/Cartesian/KDChartThreeDBarAttributes_p.h \
           KDChart/Cartesian/KDChartPlotterDiagramCompressor.h \
           KDChart/Cartesian/KDChartPlotterDiagramCompressor_p.h \
           KDChart/Cartesian/KDChartLeveyJenningsCoordinatePlane.h \
           KDChart/Cartesian/KDChartLeveyJenningsCoordinatePlane_p.h \
           KDChart/Cartesian/KDChartLeveyJenningsDiagram.h \
           KDChart/Cartesian/KDChartLeveyJenningsDiagram_p.h \
           KDChart/Cartesian/KDChartLeveyJenningsGrid.h \
           KDChart/Cartesian/KDChartLeveyJenningsGridAttributes.h \
           KDChart/Cartesian/KDChartLeveyJenningsAxis.h \
           KDChart/Cartesian/KDChartLeveyJenningsAxis_p.h \
           KDChart/Cartesian/KDChartPlotter.h \
           KDChart/Cartesian/KDChartPlotter_p.h \
           KDChart/Cartesian/PaintingHelpers_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalBarDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalLyingBarDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalLineDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartStackedBarDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartStackedLyingBarDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartStackedLineDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentBarDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentLyingBarDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentLineDiagram_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalPlotter_p.h \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentPlotter_p.h \
           KDChart/Polar/KDChartPolarCoordinatePlane.h \
           KDChart/Polar/KDChartPolarCoordinatePlane_p.h \
           KDChart/Polar/KDChartRadarCoordinatePlane.h \
           KDChart/Polar/KDChartRadarCoordinatePlane_p.h \
           KDChart/Polar/KDChartPolarGrid.h \
           KDChart/Polar/KDChartRadarGrid.h \
           KDChart/Polar/KDChartAbstractPieDiagram.h \
           KDChart/Polar/KDChartAbstractPieDiagram_p.h \
           KDChart/Polar/KDChartAbstractPolarDiagram.h \
           KDChart/Polar/KDChartAbstractPolarDiagram_p.h \
           KDChart/Polar/KDChartPieDiagram.h \
           KDChart/Polar/KDChartPieDiagram_p.h \
           KDChart/Polar/KDChartPieAttributes.h \
           KDChart/Polar/KDChartPieAttributes_p.h \
           KDChart/Polar/KDChartPolarDiagram.h \
           KDChart/Polar/KDChartPolarDiagram_p.h \
           KDChart/Polar/KDChartRadarDiagram.h \
           KDChart/Polar/KDChartRadarDiagram_p.h \
           KDChart/Polar/KDChartRingDiagram.h \
           KDChart/Polar/KDChartRingDiagram_p.h \
           KDChart/Polar/KDChartThreeDPieAttributes.h \
           KDChart/Polar/KDChartThreeDPieAttributes_p.h \
           KDChart/Ternary/TernaryPoint.h \
           KDChart/Ternary/TernaryConstants.h \
           KDChart/Ternary/KDChartTernaryGrid.h \
           KDChart/Ternary/KDChartTernaryCoordinatePlane.h \
           KDChart/Ternary/KDChartTernaryCoordinatePlane_p.h \
           KDChart/Ternary/KDChartTernaryAxis.h \
           KDChart/Ternary/KDChartAbstractTernaryDiagram.h \
           KDChart/Ternary/KDChartAbstractTernaryDiagram_p.h \
           KDChart/Ternary/KDChartTernaryPointDiagram.h \
           KDChart/Ternary/KDChartTernaryLineDiagram.h \
           KDGantt/kdganttglobal.h \
           KDGantt/kdganttview.h \
           KDGantt/kdganttstyleoptionganttitem.h \
           KDGantt/kdganttgraphicsview.h \
           KDGantt/kdganttabstractrowcontroller.h \
           KDGantt/kdgantttreeviewrowcontroller.h \
           KDGantt/kdganttlistviewrowcontroller.h \
           KDGantt/kdganttgraphicsscene.h \
           KDGantt/kdganttgraphicsitem.h \
           KDGantt/kdganttconstraint.h \
           KDGantt/kdganttconstraintproxy.h \
           KDGantt/kdganttconstraintgraphicsitem.h \
           KDGantt/kdganttitemdelegate.h \
           KDGantt/kdganttforwardingproxymodel.h \
           KDGantt/kdganttsummaryhandlingproxymodel.h \
           KDGantt/kdganttproxymodel.h \
           KDGantt/kdganttconstraintmodel.h \
           KDGantt/kdganttabstractgrid.h \
           KDGantt/kdganttdatetimegrid.h \
           KDGantt/kdganttlegend.h \
           KDGantt/kdgantttreeviewrowcontroller_p.h \
           KDGantt/kdganttlistviewrowcontroller_p.h \
           KDGantt/kdganttview_p.h \
           KDGantt/kdganttgraphicsview_p.h \
           KDGantt/kdganttgraphicsscene_p.h \
           KDGantt/kdganttconstraint_p.h \
           KDGantt/kdganttitemdelegate_p.h \
           KDGantt/kdganttsummaryhandlingproxymodel_p.h \
           KDGantt/kdganttproxymodel_p.h \
           KDGantt/kdganttconstraintmodel_p.h \
           KDGantt/kdganttabstractgrid_p.h \
           KDGantt/kdganttdatetimegrid_p.h \
           KDGantt/kdganttlegend_p.h \
           KDGantt/unittest/test.h \
           KDGantt/unittest/testregistry.h \
           KDGantt/unittest/libutil.h

SOURCES += \
           KDChart/KDChartMeasure.cpp \
           KDChart/KDChartAbstractCoordinatePlane.cpp \
           KDChart/KDChartChart.cpp \
           KDChart/KDChartWidget.cpp \
           KDChart/KDChartAbstractDiagram.cpp \
           KDChart/KDChartAbstractDiagram_p.cpp \
           KDChart/KDChartAbstractAreaBase.cpp \
           KDChart/KDChartAbstractArea.cpp \
           KDChart/KDChartTextArea.cpp \
           KDChart/KDChartAbstractAreaWidget.cpp \
           KDChart/KDChartAbstractAxis.cpp \
           KDChart/KDChartAbstractProxyModel.cpp \
           KDChart/KDChartAbstractGrid.cpp \
           KDChart/KDChartAttributesModel.cpp \
           KDChart/KDChartBackgroundAttributes.cpp \
           KDChart/KDChartDatasetProxyModel.cpp \
           KDChart/KDChartDatasetSelector.cpp \
           KDChart/KDChartDataValueAttributes.cpp \
           KDChart/KDChartDiagramObserver.cpp \
           KDChart/KDChartFrameAttributes.cpp \
           KDChart/KDChartGridAttributes.cpp \
           KDChart/KDChartRulerAttributes.cpp \
           KDChart/KDChartHeaderFooter.cpp \
           KDChart/KDChartLayoutItems.cpp \
           KDChart/KDChartLegend.cpp \
           KDChart/KDChartLineAttributes.cpp \
           KDChart/KDChartMarkerAttributes.cpp \
           KDChart/KDChartPaintContext.cpp \
           KDChart/KDChartPalette.cpp \
           KDChart/KDChartPosition.cpp \
           KDChart/KDChartRelativePosition.cpp \
           KDChart/KDTextDocument.cpp \
           KDChart/KDChartTextAttributes.cpp \
           KDChart/KDChartAbstractThreeDAttributes.cpp \
           KDChart/KDChartThreeDLineAttributes.cpp \
           KDChart/KDChartTextLabelCache.cpp \
           KDChart/ChartGraphicsItem.cpp \
           KDChart/ReverseMapper.cpp \
           KDChart/KDChartValueTrackerAttributes.cpp \
           KDChart/KDChartPrintingParameters.cpp \
           KDChart/KDChartModelDataCache_p.cpp \
           KDChart/Cartesian/KDChartAbstractCartesianDiagram.cpp \
           KDChart/Cartesian/KDChartCartesianCoordinatePlane.cpp \
           KDChart/Cartesian/KDChartCartesianAxis.cpp \
           KDChart/Cartesian/KDChartCartesianGrid.cpp \
           KDChart/Cartesian/KDChartBarDiagram.cpp \
           KDChart/Cartesian/KDChartBarDiagram_p.cpp \
           KDChart/Cartesian/KDChartThreeDBarAttributes.cpp \
           KDChart/Cartesian/KDChartBarAttributes.cpp \
           KDChart/Cartesian/KDChartStockBarAttributes.cpp \
           KDChart/Cartesian/KDChartStockDiagram.cpp \
           KDChart/Cartesian/KDChartStockDiagram_p.cpp \
           KDChart/Cartesian/KDChartLineDiagram.cpp \
           KDChart/Cartesian/KDChartLineDiagram_p.cpp \
           KDChart/Cartesian/KDChartCartesianDiagramDataCompressor_p.cpp \
           KDChart/Cartesian/KDChartPlotter.cpp \
           KDChart/Cartesian/KDChartPlotter_p.cpp \
           KDChart/Cartesian/KDChartPlotterDiagramCompressor.cpp \
           KDChart/Cartesian/KDChartLeveyJenningsCoordinatePlane.cpp \
           KDChart/Cartesian/KDChartLeveyJenningsDiagram.cpp \
           KDChart/Cartesian/KDChartLeveyJenningsDiagram_p.cpp \
           KDChart/Cartesian/KDChartLeveyJenningsGrid.cpp \
           KDChart/Cartesian/KDChartLeveyJenningsGridAttributes.cpp \
           KDChart/Cartesian/KDChartLeveyJenningsAxis.cpp \
           KDChart/Cartesian/PaintingHelpers_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalPlotter_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentPlotter_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartStackedLyingBarDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartStackedLineDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartStackedBarDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentBarDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentLyingBarDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartPercentLineDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalBarDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalLyingBarDiagram_p.cpp \
           KDChart/Cartesian/DiagramFlavors/KDChartNormalLineDiagram_p.cpp \
           KDChart/Polar/KDChartPolarCoordinatePlane.cpp \
           KDChart/Polar/KDChartRadarCoordinatePlane.cpp \
           KDChart/Polar/KDChartAbstractPieDiagram.cpp \
           KDChart/Polar/KDChartAbstractPolarDiagram.cpp \
           KDChart/Polar/KDChartPolarGrid.cpp \
           KDChart/Polar/KDChartRadarGrid.cpp \
           KDChart/Polar/KDChartPieDiagram.cpp \
           KDChart/Polar/KDChartPolarDiagram.cpp \
           KDChart/Polar/KDChartRadarDiagram.cpp \
           KDChart/Polar/KDChartRingDiagram.cpp \
           KDChart/Polar/KDChartPieAttributes.cpp \
           KDChart/Polar/KDChartThreeDPieAttributes.cpp \
           KDChart/Ternary/KDChartTernaryAxis.cpp \
           KDChart/Ternary/KDChartTernaryGrid.cpp \
           KDChart/Ternary/TernaryPoint.cpp \
           KDChart/Ternary/TernaryConstants.cpp \
           KDChart/Ternary/KDChartTernaryCoordinatePlane.cpp \
           KDChart/Ternary/KDChartAbstractTernaryDiagram.cpp \
           KDChart/Ternary/KDChartTernaryPointDiagram.cpp \
           KDChart/Ternary/KDChartTernaryLineDiagram.cpp \
           KDGantt/kdganttglobal.cpp \
           KDGantt/kdganttview.cpp \
           KDGantt/kdganttstyleoptionganttitem.cpp \
           KDGantt/kdganttgraphicsview.cpp \
           KDGantt/kdganttabstractrowcontroller.cpp \
           KDGantt/kdgantttreeviewrowcontroller.cpp \
           KDGantt/kdganttlistviewrowcontroller.cpp \
           KDGantt/kdganttgraphicsscene.cpp \
           KDGantt/kdganttgraphicsitem.cpp \
           KDGantt/kdganttconstraint.cpp \
           KDGantt/kdganttconstraintproxy.cpp \
           KDGantt/kdganttconstraintgraphicsitem.cpp \
           KDGantt/kdganttitemdelegate.cpp \
           KDGantt/kdganttforwardingproxymodel.cpp \
           KDGantt/kdganttsummaryhandlingproxymodel.cpp \
           KDGantt/kdganttproxymodel.cpp \
           KDGantt/kdganttconstraintmodel.cpp \
           KDGantt/kdganttabstractgrid.cpp \
           KDGantt/kdganttdatetimegrid.cpp \
           KDGantt/kdganttlegend.cpp \
           KDGantt/unittest/test.cpp \
           KDGantt/unittest/testregistry.cpp \

CONFIG += warn-on

# We want users of kdchart to be able to use the lib without interference with Qt-specific keywords, e.g. "signals" that collides with Boost's Signals
DEFINES += QT_NO_KEYWORDS
DEFINES += emit=""

LIBFAKES_PATH = $${TOP_SOURCE_DIR}/kdablibfakes

DEPENDPATH = $${TOP_SOURCE_DIR}/include \
            $$LIBFAKES_PATH/include \
            $${TOP_SOURCE_DIR}/src
INCLUDEPATH = $$LIBFAKES_PATH/include \
              KDGantt \
              KDChart \
              KDChart/Cartesian \
              KDChart/Cartesian/DiagramFlavors \
              KDChart/Polar \
              KDChart/Ternary \
              $${TOP_SOURCE_DIR}/src \
              $${TOP_SOURCE_DIR}/include

linux-*{
  version_script{
    QMAKE_LFLAGS += -Wl,--version-script=libkdchart.map
    TARGETDEPS += libkdchart.map
  }
}

solaris-*{
LIBS *= -lsunmath
}

qsa{
  message(compiling QSA support into KD Chart)
  SOURCES += KDChartObjectFactory.cpp \
  KDChartWrapperFactory.cpp \
  wrappers/KDChartAxisParamsWrapper.cpp \
  wrappers/KDChartParamsWrapper.cpp \
  wrappers/KDChartCustomBoxWrapper.cpp
  HEADERS += KDChartObjectFactory.h \
  KDChartWrapperFactory.h \
  wrappers/KDChartAxisParamsWrapper.h \
  wrappers/KDChartParamsWrapper.h \
  wrappers/KDChartCustomBoxWrapper.h \
  factories/QtFactory.h \
  factories/QFontFactory.h
}

#*g++*{
#  QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
#}
