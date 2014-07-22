set(libmodels_a_SOURCES
    accountsmodel.cpp
    models.cpp
   )

add_library(models STATIC ${libmodels_a_SOURCES})
target_link_libraries(models Alkimia::alkimia)

add_dependencies(models kmm_config)

