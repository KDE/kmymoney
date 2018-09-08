set(EXTPREFIX_applications "${EXTPREFIX}")

ExternalProject_Add(
    ext_akonadi
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/applications/17.12.3/src/akonadi-17.12.3.tar.xz
    URL_MD5 92f1aa77da4e51acab138f686e693b93
    INSTALL_DIR ${EXTPREFIX_applications}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_applications}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_boost ext_xslt ext_kcompletion ext_kconfig ext_kconfigwidgets ext_kcoreaddons ext_kdbusaddons ext_ki18n ext_kiconthemes ext_kitemmodels ext_kitemviews ext_kio ext_kwidgetsaddons ext_kwindowsystem ext_kxmlgui ext_kcrash
)

ExternalProject_Add(
    ext_kpimtextedit
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/applications/17.12.3/src/kpimtextedit-17.12.3.tar.xz
    URL_MD5 8d7d128e5eadbf1a0022a134a00420e6
    INSTALL_DIR ${EXTPREFIX_applications}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_applications}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcodecs ext_kconfig ext_kconfigwidgets ext_kcoreaddons ext_kemoticons ext_ki18n ext_kiconthemes ext_kio ext_sonnet ext_syntaxhighlighting ext_kwidgetsaddons ext_kxmlgui ext_grantlee
)

ExternalProject_Add(
    ext_kidentitymanagement
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/applications/17.12.3/src/kidentitymanagement-17.12.3.tar.xz
    URL_MD5 4cd6e2ddfbe9a07e7916b27ac7a468dc
    INSTALL_DIR ${EXTPREFIX_applications}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_applications}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons ext_kcompletion ext_ktextwidgets ext_kxmlgui ext_kio ext_kconfig ext_kcodecs ext_kiconthemes ext_kpimtextedit
)

ExternalProject_Add(
    ext_kcontacts
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/applications/17.12.3/src/kcontacts-17.12.3.tar.xz
    URL_MD5 bbfbc05509e92cac07ffb4ffee05c9ee
    INSTALL_DIR ${EXTPREFIX_applications}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_applications}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons ext_ki18n ext_kconfig ext_kcodecs
)


