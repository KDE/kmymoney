SET(libmodels_a_SOURCES
    accountsmodel.cpp
    models.cpp
   )

add_library(models STATIC ${libmodels_a_SOURCES})

ADD_DEPENDENCIES(models kmm_config)

