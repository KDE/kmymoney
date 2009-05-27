

## KDE4 includes needed for kdemacros.h
include_directories(
        ${KDE4_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}/../include 
	${CMAKE_CURRENT_SOURCE_DIR}/../kdablibfakes/include 
)

ADD_DEFINITIONS(-DKDCHART_BUILD_KDCHART_LIB)

########### next target ###############

set(kdchart_LIB_SRCS 
    KDChartMeasure.cpp
    KDChartAbstractCartesianDiagram.cpp
    KDChartAbstractCoordinatePlane.cpp
    KDChartCartesianCoordinatePlane.cpp
    KDChartPolarCoordinatePlane.cpp
    KDChartChart.cpp
    KDChartWidget.cpp
    KDChartAbstractDiagram.cpp
    KDChartAbstractPieDiagram.cpp
    KDChartAbstractPolarDiagram.cpp
    KDChartAbstractAreaBase.cpp
    KDChartAbstractArea.cpp
    KDChartTextArea.cpp
    KDChartAbstractAreaWidget.cpp
    KDChartAbstractAxis.cpp
    KDChartAbstractProxyModel.cpp
    KDChartAbstractGrid.cpp
    KDChartCartesianGrid.cpp
    KDChartPolarGrid.cpp
    KDChartAttributesModel.cpp
    KDChartBackgroundAttributes.cpp
    KDChartBarAttributes.cpp
    KDChartBarDiagram.cpp
    KDChartNormalLyingBarDiagram_p.cpp
    KDChartStackedLyingBarDiagram_p.cpp
    KDChartPercentPlotter_p.cpp
    KDChartCartesianAxis.cpp
    KDChartDatasetProxyModel.cpp
    KDChartDatasetSelector.cpp
    KDChartDataValueAttributes.cpp
    KDChartDiagramObserver.cpp
    KDChartFrameAttributes.cpp
    KDChartGridAttributes.cpp
    KDChartHeaderFooter.cpp
    KDChartLayoutItems.cpp
    KDChartLegend.cpp
    KDChartLineAttributes.cpp
    KDChartLineDiagram.cpp
    KDChartMarkerAttributes.cpp
    KDChartPaintContext.cpp
    KDChartPalette.cpp
    KDChartPieDiagram.cpp
    KDChartPolarDiagram.cpp
    KDChartPosition.cpp
    KDChartRelativePosition.cpp
    KDChartRingDiagram.cpp
    KDTextDocument.cpp
    KDChartTextAttributes.cpp
    KDChartPieAttributes.cpp
    KDChartAbstractThreeDAttributes.cpp
    KDChartThreeDBarAttributes.cpp
    KDChartThreeDLineAttributes.cpp     
    KDChartThreeDPieAttributes.cpp
    KDChartSignalCompressor.cpp
    KDChartBarDiagram_p.cpp
    KDChartCartesianDiagramDataCompressor_p.cpp
    KDChartLineDiagram_p.cpp
    KDChartNormalBarDiagram_p.cpp
    KDChartNormalLineDiagram_p.cpp
    KDChartNormalPlotter_p.cpp
    KDChartPercentBarDiagram_p.cpp
    KDChartPercentLyingBarDiagram_p.cpp
    KDChartPercentLineDiagram_p.cpp
    KDChartPlotter.cpp
    KDChartPlotter_p.cpp
    KDChartStackedBarDiagram_p.cpp
    KDChartStackedLineDiagram_p.cpp
    KDChartValueTrackerAttributes.cpp
    KDChartRulerAttributes.cpp
    KDChartPrintingParameters.cpp
    KDChartModelDataCache_p.cpp
    PrerenderedElements/KDChartTextLabelCache.cpp
    LeveyJennings/KDChartLeveyJenningsDiagram.cpp
    LeveyJennings/KDChartLeveyJenningsDiagram_p.cpp
    LeveyJennings/KDChartLeveyJenningsAxis.cpp
    LeveyJennings/KDChartLeveyJenningsCoordinatePlane.cpp
    LeveyJennings/KDChartLeveyJenningsGrid.cpp
    LeveyJennings/KDChartLeveyJenningsGridAttributes.cpp
    Scenery/ReverseMapper.cpp
    Scenery/ChartGraphicsItem.cpp
)

qt4_wrap_ui(kdchart_LIB_SRCS
    KDChartDatasetSelector.ui
)

kde4_add_library(kmm_kdchart SHARED ${kdchart_LIB_SRCS})

target_link_libraries(kmm_kdchart  ${QT_QTSVG_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${ZLIB_LIBRARY} )
SET_TARGET_PROPERTIES(kmm_kdchart PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kmm_kdchart  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############


#NOTHING
