SET(EXTPREFIX_frameworks "${EXTPREFIX}" )
#
# All needed frameworks:
# Tier 1
#   Archive
#   Codecs
#   Config
#   CoreAddons
#   DBusAddons
#   GuiAddons
#   I18n
#   ItemModels
#   ItemViews
#   WidgetsAddons
#   WindowSystem
#   Solid
#   Sonnet
# Tier 2
#   Auth
#   Completion
#   Crash
#   JobWidgets
#   Notifications
#   Package
# Tier 3
#   ConfigWidgets
#   IconThemes
#   Service
#   TextWidgets
#   XmlGui
#   Bookmarks
#   GlobalAccel
#   KIO
#   Declarative
#   KCMUtils
#   Wallet

ExternalProject_Add(
    ext_extra_cmake_modules
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/extra-cmake-modules-5.44.0.zip
    URL_MD5 74aa8fc501e27024390b01c81f2925eb
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/ecm_install_to_share.diff
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
)

ExternalProject_Add(
    ext_karchive
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/karchive-5.44.0.zip
    URL_MD5 c60a8e22b88cc7328610041638459689
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kcodecs
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kcodecs-5.44.0.zip
    URL_MD5 786f4a1870825c7df5934da35374b51c
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kconfig-5.44.0.zip
    URL_MD5 d0223ea471bbf463ec42c2a2355a5183
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kconfig.diff
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kdbusaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kdbusaddons-5.44.0.zip
    URL_MD5 bee1d8ac4e6e87e3fe17ac7adba40b76
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/desktoptojson.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kcoreaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kcoreaddons-5.44.0.zip
    URL_MD5 16a7379f3e2941d1c19d6f80939f15e8
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/desktoptojson.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kguiaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kguiaddons-5.44.0.zip
    URL_MD5 440eefbf5abcafc492dcf857f7e4eaf5
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_ki18n
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://download.kde.org/stable/frameworks/5.44/ki18n-5.44.0.zip
    URL_MD5 333ab0a3f65a298e928d746144d4dc8e
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/ki18n-appdatalocation.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kitemmodels
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kitemmodels-5.44.0.zip
    URL_MD5 ea43a5e2cc7033eb672796b108d7403b
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kitemviews
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kitemviews-5.44.0.zip
    URL_MD5 8b15c703313c7a790c7db897ef17de7d
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kwidgetsaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kwidgetsaddons-5.44.0.zip
    URL_MD5 a9911d8d0f8aaf7a7afd84c41c8f80a1
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kwidgetsaddons.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kwindowsystem
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kwindowsystem-5.44.0.zip
    URL_MD5 75329f47cf8cd413fa1d15a57c298563
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kwindowsystem-x11.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_solid
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/solid-5.44.0.zip
    URL_MD5 2697e1633428a63c6dc1ce1a26dc5485
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_sonnet
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/sonnet-5.44.0.zip
    URL_MD5 6c59ade0268bffdb5af93af340aef5ee
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kauth
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kauth-5.44.0.zip
    URL_MD5 b77ecf084b227f0ac3118f6af775299c
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons
)

ExternalProject_Add(
    ext_kcompletion
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kcompletion-5.44.0.zip
    URL_MD5 0647885a702c338a1b656eb4f311ad16
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kwidgetsaddons ext_kconfig
)

ExternalProject_Add(
    ext_kcrash
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kcrash-5.44.0.zip
    URL_MD5 61adc0e125c65288968d958acf25f4aa
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons ext_kwindowsystem
)

ExternalProject_Add(
    ext_kjobwidgets
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kjobwidgets-5.44.0.zip
    URL_MD5 0297d9655c85309bc85c0f7ff50fdb72
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons ext_kwidgetsaddons
)

ExternalProject_Add(
    ext_knotifications
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/knotifications-5.44.0.zip
    URL_MD5 c94e747cb1184f91aa084047e5c41d61
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kwindowsystem ext_kconfig ext_kcodecs ext_kcoreaddons
)

ExternalProject_Add(
    ext_kpackage
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kpackage-5.44.0.zip
    URL_MD5 bab368ead50a516cd4f2dce1157415af
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_karchive ext_ki18n ext_kcoreaddons
)

ExternalProject_Add(
    ext_kconfigwidgets
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kconfigwidgets-5.44.0.zip
    URL_MD5 46e2c1b755b6ee4f932a609451f32b42
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kauth ext_kcoreaddons ext_kcodecs ext_kconfig ext_kguiaddons ext_ki18n ext_kwidgetsaddons
)

ExternalProject_Add(
    ext_kiconthemes
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kiconthemes-5.44.0.zip
    URL_MD5 080541d989b2a816c17a41339b9d35b3
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_karchive ext_ki18n ext_kcoreaddons ext_kconfigwidgets ext_kwidgetsaddons ext_kitemviews
)

ExternalProject_Add(
    ext_kservice
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kservice-5.44.0.zip
    URL_MD5 62950b073eee9ad99a9eb73d2439b655
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kconfig ext_kcoreaddons ext_kcrash ext_kdbusaddons ext_ki18n
)

ExternalProject_Add(
    ext_ktextwidgets
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/ktextwidgets-5.44.0.zip
    URL_MD5 d5e7f26038efeec17069ad7a231f2e13
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules ext_kcompletion ext_kconfig ext_kconfigwidgets ext_ki18n ext_kiconthemes ext_kservice ext_kwidgetsaddons ext_kwindowsystem ext_sonnet
)

ExternalProject_Add(
    ext_kxmlgui
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kxmlgui-5.44.0.zip
    URL_MD5 b89499ffee5c768e87d4db20829467bd
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons ext_kitemviews ext_kconfig ext_kconfigwidgets ext_ki18n ext_kiconthemes ext_ktextwidgets ext_kwidgetsaddons ext_kwindowsystem
)

ExternalProject_Add(
    ext_kbookmarks
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kbookmarks-5.44.0.zip
    URL_MD5 fd78c7fd52c324b919c878a7a3d0d887
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kconfig ext_kcoreaddons ext_kcodecs ext_kconfigwidgets ext_kiconthemes ext_kwidgetsaddons ext_kxmlgui
)

ExternalProject_Add(
    ext_kglobalaccel
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kglobalaccel-5.44.0.zip
    URL_MD5 dabfff26dbed4494754f7327fadd3cf0
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kconfig ext_kcoreaddons ext_kcrash ext_kdbusaddons ext_kwindowsystem
)

ExternalProject_Add(
    ext_kio
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kio-5.44.0.zip
    URL_MD5 44b6d7eaa9d56a0dd171e7c1f8cf7efe
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules ext_karchive ext_kconfig ext_kcoreaddons ext_kdbusaddons ext_ki18n ext_kservice ext_solid ext_ki18n ext_kwindowsystem ext_kwidgetsaddons ext_kcompletion ext_kconfigwidgets ext_kiconthemes ext_kitemviews ext_kjobwidgets ext_kbookmarks
)

ExternalProject_Add(
    ext_kdeclarative
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kdeclarative-5.44.0.zip
    URL_MD5 1b66d349365c586d4b67ceaf5fa35071
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kconfig ext_ki18n ext_kiconthemes ext_kio ext_kwidgetsaddons ext_kwindowsystem ext_kglobalaccel ext_kguiaddons ext_kpackage
)

ExternalProject_Add(
    ext_kcmutils
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kcmutils-5.44.0.zip
    URL_MD5 920c1922396592be54002f84b4f1a816
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kitemviews ext_kconfigwidgets ext_kcoreaddons ext_ki18n ext_kiconthemes ext_kservice ext_kxmlgui ext_kdeclarative
)

ExternalProject_Add(
    ext_kwallet
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kwallet-5.44.0.zip
    URL_MD5 7d8022a521208ece7d4d6ec8291ea377
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons ext_kconfig ext_kwindowsystem ext_ki18n
)

ExternalProject_Add(
    ext_kimageformats
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.44/kimageformats-5.44.0.zip
    URL_MD5 02a98b682f9cb655592148d7ebcc05e7
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kimageformats.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kitemviews
)
