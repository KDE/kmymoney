SET(libmodels_a_SOURCES
    accountsmodel.cpp
    models.cpp
   )

KDE4_ADD_LIBRARY(models STATIC ${libmodels_a_SOURCES})
