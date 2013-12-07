SET(libmodels_a_SOURCES
    accountsmodel.cpp
    onlinejobmodel.cpp
    models.cpp
   )

KDE4_ADD_LIBRARY(models STATIC ${libmodels_a_SOURCES})

TARGET_LINK_LIBRARIES(models
    kmm_mymoney
)

ADD_DEPENDENCIES(models kmm_config)

