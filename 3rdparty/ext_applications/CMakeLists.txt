set(EXTPREFIX_applications "${EXTPREFIX}")

ExternalProject_Add(
    ext_akonadi
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/applications/18.08.1/src/akonadi-18.08.1.tar.xz
    URL_MD5 38d273a3c934b8d5828c669e482209d0
    INSTALL_DIR ${EXTPREFIX_applications}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_applications}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_sharedmimeinfo ext_boost ext_xslt ext_kcompletion ext_kconfig ext_kconfigwidgets ext_kcoreaddons ext_kdbusaddons ext_ki18n ext_kiconthemes ext_kitemmodels ext_kitemviews ext_kio ext_kwidgetsaddons ext_kwindowsystem ext_kxmlgui ext_kcrash
)

ExternalProject_Add(
    ext_kpimtextedit
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/applications/18.08.1/src/kpimtextedit-18.08.1.tar.xz
    URL_MD5 869135774d0cdf71977f96b508a1c3d9
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
    URL https://download.kde.org/stable/applications/18.08.1/src/kidentitymanagement-18.08.1.tar.xz
    URL_MD5 ab4e43ed9b464f153e4ed2befa12ad20
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
    URL https://download.kde.org/stable/applications/18.08.1/src/kcontacts-18.08.1.tar.xz
    URL_MD5 cd24d5ae7dc44c5599b16a5200894118
    INSTALL_DIR ${EXTPREFIX_applications}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_applications}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons ext_ki18n ext_kconfig ext_kcodecs
)


