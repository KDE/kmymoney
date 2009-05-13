INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ${ZLIB_INCLUDE_DIR} )

########### next target ###############

SET(libkmm_kdchart_la_SOURCES 
  KDChartAreaPainter.cpp	
  KDChartAxesPainter.cpp	
  KDChartAxisParams.cpp	
  KDChartAxisParamsWrapper.cpp 
  KDChartBarPainter.cpp	
  KDChartBaseSeries.cpp 
  KDChartBWPainter.cpp	
  KDChart.cpp 		
  KDChartCustomBox.cpp 	
  KDChartCustomBoxWrapper.cpp 
  KDChartDataIntern.cpp 	
  KDChartEnums.cpp 
  KDChartHiLoPainter.cpp	
  KDChartLinesPainter.cpp	
  KDChartPainter.cpp	
  KDChartParams.cpp	
  KDChartParams_frame.cpp
  KDChartParams_io.cpp	
  KDChartParamsWrapper.cpp 
  KDChartPiePainter.cpp	
  KDChartPlaneSeries.cpp 
  KDChartPolarPainter.cpp	
  KDChartPropertySet.cpp	
  KDChartRingPainter.cpp	
  KDChartSeriesCollection.cpp 
  KDChartTableBase.cpp 
  KDChartTableDataWrapper.cpp
  KDChartTextPiece.cpp	
  KDChartVectorSeries.cpp 
  KDChartVectorTable.cpp 
  KDChartWidget.cpp 	
  KDDrawText.cpp		
  KDFrame.cpp 		
  KDFrameProfileSection.cpp 
  KDXMLTools.cpp          
  )

KDE4_ADD_LIBRARY(kmm_kdchart SHARED ${libkmm_kdchart_la_SOURCES})
SET_TARGET_PROPERTIES(kmm_kdchart PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
TARGET_LINK_LIBRARIES(kmm_kdchart ${QT_AND_KDECORE_LIBS} ${ZLIB_LIBRARIES})

########### install files ###############
INSTALL(TARGETS kmm_kdchart 
	${INSTALL_TARGETS_DEFAULT_ARGS})
