

## KDE4 includes needed for kdemacros.h
include_directories(
        ${KDE4_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../kdablibfakes/include
)

add_definitions(-DKDCHART_BUILD_KDCHART_LIB)

########### next target ###############

set(kdchart_LIB_SRCS
    ChartGraphicsItem.cpp
    KDChartAbstractAreaBase.cpp
    KDChartAbstractArea.cpp
    KDChartAbstractAreaWidget.cpp
    KDChartAbstractAxis.cpp
    KDChartAbstractCartesianDiagram.cpp
    KDChartAbstractCoordinatePlane.cpp
    KDChartAbstractDiagram.cpp
    KDChartAbstractGrid.cpp
    KDChartAbstractPieDiagram.cpp
    KDChartAbstractPolarDiagram.cpp
    KDChartAbstractProxyModel.cpp
    KDChartAbstractTernaryDiagram.cpp
    KDChartAbstractThreeDAttributes.cpp
    KDChartAttributesModel.cpp
    KDChartBackgroundAttributes.cpp
    KDChartBarAttributes.cpp
    KDChartBarDiagram.cpp
    KDChartBarDiagram_p.cpp
    KDChartCartesianAxis.cpp
    KDChartCartesianCoordinatePlane.cpp
    KDChartCartesianDiagramDataCompressor_p.cpp
    KDChartCartesianGrid.cpp
    KDChartChart.cpp
    KDChartDatasetProxyModel.cpp
    KDChartDatasetSelector.cpp
    KDChartDataValueAttributes.cpp
    KDChartDiagramObserver.cpp
    KDChartFrameAttributes.cpp
    KDChartGridAttributes.cpp
    KDChartHeaderFooter.cpp
    KDChartLayoutItems.cpp
    KDChartLegend.cpp
    KDChartLeveyJenningsAxis.cpp
    KDChartLeveyJenningsCoordinatePlane.cpp
    KDChartLeveyJenningsDiagram.cpp
    KDChartLeveyJenningsDiagram_p.cpp
    KDChartLeveyJenningsGridAttributes.cpp
    KDChartLeveyJenningsGrid.cpp
    KDChartLineAttributes.cpp
    KDChartLineDiagram.cpp
    KDChartLineDiagram_p.cpp
    KDChartMarkerAttributes.cpp
    KDChartMeasure.cpp
    KDChartModelDataCache_p.cpp
    KDChartNormalBarDiagram_p.cpp
    KDChartNormalLineDiagram_p.cpp
    KDChartNormalLyingBarDiagram_p.cpp
    KDChartNormalPlotter_p.cpp
    KDChartPaintContext.cpp
    KDChartPalette.cpp
    KDChartPercentBarDiagram_p.cpp
    KDChartPercentLineDiagram_p.cpp
    KDChartPercentLyingBarDiagram_p.cpp
    KDChartPercentPlotter_p.cpp
    KDChartPieAttributes.cpp
    KDChartPieDiagram.cpp
    KDChartPlotter.cpp
    KDChartPlotter_p.cpp
    KDChartPolarCoordinatePlane.cpp
    KDChartPolarDiagram.cpp
    KDChartPolarGrid.cpp
    KDChartPosition.cpp
    KDChartPrintingParameters.cpp
    KDChartRelativePosition.cpp
    KDChartRingDiagram.cpp
    KDChartRulerAttributes.cpp
    KDChartSignalCompressor.cpp
    KDChartStackedBarDiagram_p.cpp
    KDChartStackedLineDiagram_p.cpp
    KDChartStackedLyingBarDiagram_p.cpp
    KDChartStockBarAttributes.cpp
    KDChartStockDiagram.cpp
    KDChartStockDiagram_p.cpp
    KDChartTernaryAxis.cpp
    KDChartTernaryCoordinatePlane.cpp
    KDChartTernaryGrid.cpp
    KDChartTernaryLineDiagram.cpp
    KDChartTernaryPointDiagram.cpp
    KDChartTextArea.cpp
    KDChartTextAttributes.cpp
    KDChartTextLabelCache.cpp
    KDChartThreeDBarAttributes.cpp
    KDChartThreeDLineAttributes.cpp
    KDChartThreeDPieAttributes.cpp
    KDChartValueTrackerAttributes.cpp
    KDChartWidget.cpp
    KDTextDocument.cpp
    ReverseMapper.cpp
    TernaryConstants.cpp
    TernaryPoint.cpp
)

qt5_wrap_ui(kdchart_LIB_SRCS
    KDChartDatasetSelector.ui
)

qt_wrap_cpp(kdchart kdchart_LIB_SRCS KDChartEnums.h)

add_library(kmm_kdchart SHARED ${kdchart_LIB_SRCS})

target_link_libraries(kmm_kdchart PUBLIC Qt5::Svg Qt5::Xml Qt5::Gui Qt5::Core ${ZLIB_LIBRARY} )
set_target_properties(kmm_kdchart PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kmm_kdchart  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############


#NOTHING
