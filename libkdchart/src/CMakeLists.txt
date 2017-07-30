

## KDE4 includes needed for kdemacros.h
include_directories(
  ${KDE4_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/KDChart
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/KDGantt
  ${CMAKE_CURRENT_SOURCE_DIR}/../kdablibfakes/include
  ${CMAKE_CURRENT_SOURCE_DIR}/KDChart
  ${CMAKE_CURRENT_SOURCE_DIR}/KDChart/Cartesian
  ${CMAKE_CURRENT_SOURCE_DIR}/KDChart/Cartesian/DiagramFlavors
  ${CMAKE_CURRENT_SOURCE_DIR}/KDChart/Polar
  ${CMAKE_CURRENT_SOURCE_DIR}/KDChart/Ternary
)

add_definitions(-DKDCHART_BUILD_KDCHART_LIB)

########### next target ###############

set(kdchart_LIB_SRCS
    KDChart/KDChartMeasure.cpp
    KDChart/KDChartAbstractCoordinatePlane.cpp
    KDChart/KDChartChart.cpp
    KDChart/KDChartWidget.cpp
    KDChart/KDChartAbstractDiagram.cpp
    KDChart/KDChartAbstractDiagram_p.cpp
    KDChart/KDChartAbstractAreaBase.cpp
    KDChart/KDChartAbstractArea.cpp
    KDChart/KDChartTextArea.cpp
    KDChart/KDChartAbstractAreaWidget.cpp
    KDChart/KDChartAbstractAxis.cpp
    KDChart/KDChartAbstractProxyModel.cpp
    KDChart/KDChartAbstractGrid.cpp
    KDChart/KDChartAttributesModel.cpp
    KDChart/KDChartBackgroundAttributes.cpp
    KDChart/KDChartDatasetProxyModel.cpp
    KDChart/KDChartDatasetSelector.cpp
    KDChart/KDChartDataValueAttributes.cpp
    KDChart/KDChartDiagramObserver.cpp
    KDChart/KDChartFrameAttributes.cpp
    KDChart/KDChartGridAttributes.cpp
    KDChart/KDChartRulerAttributes.cpp
    KDChart/KDChartHeaderFooter.cpp
    KDChart/KDChartLayoutItems.cpp
    KDChart/KDChartLegend.cpp
    KDChart/KDChartLineAttributes.cpp
    KDChart/KDChartMarkerAttributes.cpp
    KDChart/KDChartPaintContext.cpp
    KDChart/KDChartPalette.cpp
    KDChart/KDChartPosition.cpp
    KDChart/KDChartRelativePosition.cpp
    KDChart/KDTextDocument.cpp
    KDChart/KDChartTextAttributes.cpp
    KDChart/KDChartAbstractThreeDAttributes.cpp
    KDChart/KDChartThreeDLineAttributes.cpp
    KDChart/KDChartTextLabelCache.cpp
    KDChart/ChartGraphicsItem.cpp
    KDChart/ReverseMapper.cpp
    KDChart/KDChartValueTrackerAttributes.cpp
    KDChart/KDChartPrintingParameters.cpp
    KDChart/KDChartModelDataCache_p.cpp
    KDChart/Cartesian/KDChartAbstractCartesianDiagram.cpp
    KDChart/Cartesian/KDChartCartesianCoordinatePlane.cpp
    KDChart/Cartesian/KDChartCartesianAxis.cpp
    KDChart/Cartesian/KDChartCartesianGrid.cpp
    KDChart/Cartesian/KDChartBarDiagram.cpp
    KDChart/Cartesian/KDChartBarDiagram_p.cpp
    KDChart/Cartesian/KDChartThreeDBarAttributes.cpp
    KDChart/Cartesian/KDChartBarAttributes.cpp
    KDChart/Cartesian/KDChartStockBarAttributes.cpp
    KDChart/Cartesian/KDChartStockDiagram.cpp
    KDChart/Cartesian/KDChartStockDiagram_p.cpp
    KDChart/Cartesian/KDChartLineDiagram.cpp
    KDChart/Cartesian/KDChartLineDiagram_p.cpp
    KDChart/Cartesian/KDChartCartesianDiagramDataCompressor_p.cpp
    KDChart/Cartesian/KDChartPlotter.cpp
    KDChart/Cartesian/KDChartPlotter_p.cpp
    KDChart/Cartesian/KDChartPlotterDiagramCompressor.cpp
    KDChart/Cartesian/KDChartLeveyJenningsCoordinatePlane.cpp
    KDChart/Cartesian/KDChartLeveyJenningsDiagram.cpp
    KDChart/Cartesian/KDChartLeveyJenningsDiagram_p.cpp
    KDChart/Cartesian/KDChartLeveyJenningsGrid.cpp
    KDChart/Cartesian/KDChartLeveyJenningsGridAttributes.cpp
    KDChart/Cartesian/KDChartLeveyJenningsAxis.cpp
    KDChart/Cartesian/PaintingHelpers_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartNormalPlotter_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartPercentPlotter_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartStackedLyingBarDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartStackedLineDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartStackedBarDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartPercentBarDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartPercentLyingBarDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartPercentLineDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartNormalBarDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartNormalLyingBarDiagram_p.cpp
    KDChart/Cartesian/DiagramFlavors/KDChartNormalLineDiagram_p.cpp
    KDChart/Polar/KDChartPolarCoordinatePlane.cpp
    KDChart/Polar/KDChartRadarCoordinatePlane.cpp
    KDChart/Polar/KDChartAbstractPieDiagram.cpp
    KDChart/Polar/KDChartAbstractPolarDiagram.cpp
    KDChart/Polar/KDChartPolarGrid.cpp
    KDChart/Polar/KDChartRadarGrid.cpp
    KDChart/Polar/KDChartPieDiagram.cpp
    KDChart/Polar/KDChartPolarDiagram.cpp
    KDChart/Polar/KDChartRadarDiagram.cpp
    KDChart/Polar/KDChartRingDiagram.cpp
    KDChart/Polar/KDChartPieAttributes.cpp
    KDChart/Polar/KDChartThreeDPieAttributes.cpp
    KDChart/Ternary/KDChartTernaryAxis.cpp
    KDChart/Ternary/KDChartTernaryGrid.cpp
    KDChart/Ternary/TernaryPoint.cpp
    KDChart/Ternary/TernaryConstants.cpp
    KDChart/Ternary/KDChartTernaryCoordinatePlane.cpp
    KDChart/Ternary/KDChartAbstractTernaryDiagram.cpp
    KDChart/Ternary/KDChartTernaryPointDiagram.cpp
    KDChart/Ternary/KDChartTernaryLineDiagram.cpp
    #KDGantt/kdganttglobal.cpp
    #KDGantt/kdganttview.cpp
    #KDGantt/kdganttstyleoptionganttitem.cpp
    #KDGantt/kdganttgraphicsview.cpp
    #KDGantt/kdganttabstractrowcontroller.cpp
    #KDGantt/kdgantttreeviewrowcontroller.cpp
    #KDGantt/kdganttlistviewrowcontroller.cpp
    #KDGantt/kdganttgraphicsscene.cpp
    #KDGantt/kdganttgraphicsitem.cpp
    #KDGantt/kdganttconstraint.cpp
    #KDGantt/kdganttconstraintproxy.cpp
    #KDGantt/kdganttconstraintgraphicsitem.cpp
    #KDGantt/kdganttitemdelegate.cpp
    #KDGantt/kdganttforwardingproxymodel.cpp
    #KDGantt/kdganttsummaryhandlingproxymodel.cpp
    #KDGantt/kdganttproxymodel.cpp
    #KDGantt/kdganttconstraintmodel.cpp
    #KDGantt/kdganttabstractgrid.cpp
    #KDGantt/kdganttdatetimegrid.cpp
    #KDGantt/kdganttlegend.cpp
    #KDGantt/unittest/test.cpp
    #KDGantt/unittest/testregistry.cpp
)

qt4_wrap_ui(kdchart_LIB_SRCS
    KDChart/KDChartDatasetSelector.ui
)

qt_wrap_cpp(kdchart kdchart_LIB_SRCS KDChart/KDChartEnums.h)

kde4_add_library(kmm_kdchart SHARED ${kdchart_LIB_SRCS})

target_link_libraries(kmm_kdchart LINK_PUBLIC ${QT_QTSVG_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${ZLIB_LIBRARY} )
set_target_properties(kmm_kdchart PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kmm_kdchart  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############


#NOTHING
