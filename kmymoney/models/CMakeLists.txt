set(libmodels_a_SOURCES
    accountsmodel.cpp
    models.cpp
   )

kde4_add_library(models STATIC ${libmodels_a_SOURCES})

add_dependencies(models kmm_config)

