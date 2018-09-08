set(EXTPREFIX_phonon "${EXTPREFIX}")

ExternalProject_Add(
    ext_phonon
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/phonon/4.10.1/phonon-4.10.1.tar.xz
    URL_MD5 edb9ab608c7b74b4f0f10ebcf53e050d
    INSTALL_DIR ${EXTPREFIX_phonon}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_phonon}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                -DBUILD_TESTING=false
                -DPHONON_BUILD_PHONON4QT5=ON
    UPDATE_COMMAND ""
)