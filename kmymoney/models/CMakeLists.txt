set(libmodels_a_SOURCES
    accountsmodel.cpp
    onlinejobmodel.cpp
    onlinebankingaccountsfilterproxymodel.cpp
    models.cpp
    payeeidentifiercontainermodel.cpp
    onlinejobmessagesmodel.cpp
   )

kde4_add_library(models STATIC ${libmodels_a_SOURCES})

target_link_libraries(models LINK_PUBLIC
    kmm_mymoney
    kmymoney_common
)

add_dependencies(models kmm_settings)
