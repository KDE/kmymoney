#!/bin/bash

# SPDX-FileCopyrightText: 2015-2016 Collabora Ltd.
# SPDX-FileCopyrightText: 2020-2024 Ralf Habacker ralf.habacker @freenet.de
#
# SPDX-License-Identifier: MIT

# add timestamps
export PS4='[$(date "+%T.%3N")]'" $PS4"

# fails also on piped failure
set -euo pipefail

# enable trace
set -x

NULL=

# ci_build
# configure and build
: "${ci_build:=yes}"

# ci_clean
# clean the build root
: "${ci_clean:=yes}"

# ci_cmake_options
# additional cmake options
: "${ci_cmake_options:=}"

# ci_distro:
# OS distribution in which we are testing
# Typical values: ubuntu, debian; maybe fedora in future
: "${ci_distro:=opensuse}"

# ci_docker:
# If non-empty, this is the name of a Docker image. ci-install.sh will
# fetch it with "docker pull" and use it as a base for a new Docker image
# named "ci-image" in which we will do our testing.
#
# If empty, we test on "bare metal".
# Typical values: ubuntu:xenial, debian:jessie-slim
: "${ci_docker:=}"

# ci_host:
# See ci-install.sh
: "${ci_host:=native}"

# ci_parallel:
# A number of parallel jobs, passed to make -j
: "${ci_parallel:=1}"

# ci_sudo:
# If yes, assume we can get root using sudo; if no, only use current user
: "${ci_sudo:=no}"

# ci_test:
# If yes, run tests; if no, just build
: "${ci_test:=yes}"

# ci_test_fatal:
# If yes, test failures break the build; if no, they are reported but ignored
: "${ci_test_fatal:=yes}"

# ci_variant:
# One of debug, reduced, legacy, production
: "${ci_variant:=production}"

# ci_build_opt:
# verbose building
: "${ci_verbose:=no}"

if [ -n "$ci_docker" ]; then
    exec docker run \
        --env=ci_docker="" \
        --env=ci_host="${ci_host}" \
        --env=ci_parallel="${ci_parallel}" \
        --env=ci_sudo=yes \
        --env=ci_test="${ci_test}" \
        --env=ci_test_fatal="${ci_test_fatal}" \
        --env=ci_variant="${ci_variant}" \
        --privileged \
        ci-image \
        tools/ci-build.sh
fi

# show env
env  | sort | gawk ' $1 ~ /^[A-Z]+/ { print "# " $0 }'

maybe_fail_tests () {
    if [ "$ci_test_fatal" = yes ]; then
        exit 1
    fi
}

srcdir="$(pwd)"
if test "$ci_clean" = "yes"; then
    rm -rf ci-build-${ci_variant}-${ci_host}
    mkdir -p ci-build-${ci_variant}-${ci_host}
fi
cd ci-build-${ci_variant}-${ci_host}

if test "$ci_build" = "yes"; then
    export QT_QPA_PLATFORM=offscreen
    cmake_options="-DCMAKE_BUILD_TYPE=RelWithDebInfo $ci_cmake_options"
    # kmymoney specific command line
    case $ci_variant in
        (kf6*)
            cmake $cmake_options -DBUILD_WITH_QT6=1 -DBUILD_WITH_QT6_CONFIRMED=1 -DUSE_MODELTEST=on -DWARNINGS_AS_ERRORS=on -Werror=dev ..
            ;;
        (*)
            cmake $cmake_options ..
            ;;
    esac

    if test "$ci_verbose" = "yes"; then
        options=-v
    else
        options=
    fi
    cmake --build . $options -j$ci_parallel
fi

# start x session if not present
own_session=0
if ! [[ -v DISPLAY ]]; then
    xvfb-run -s '+extension GLX +render' -a -n 99 openbox &
    export DISPLAY=:99
    own_session=1
fi

# setup Qt plugin path
export QT_PLUGIN_PATH=$(pwd)/lib:${QT_PLUGIN_PATH:-.}

# fix for finding kmymoney plugins
# for unknown reasons plugins are expected below bin and not lib
if ! test -e $(pwd)/bin/kmymoney_plugins; then
    ln -fs ../lib $(pwd)/bin/kmymoney_plugins
fi

# start KDE session
kdeinit5

# run tests
[ "$ci_test" = no ] || ctest -VV -j $ci_parallel || maybe_fail_tests

# install files
cmake --build . -t install DESTDIR=$(pwd)/DESTDIR

# list files
( cd DESTDIR && find . -ls)

# kill background processes
if [ "$own_session" -eq 1 ]; then
    killall -s 9 xvfb kdeinit5 openbox dbus-daemon || true
fi
