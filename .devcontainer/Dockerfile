# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.202.5/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version (use Debian 11/9, Ubuntu 18.04/21.04 on local arm64/Apple Silicon): debian-11, debian-10, debian-9, ubuntu-21.04, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="ubuntu-21.04"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>
ENV DEBIAN_FRONTEND noninteractive

RUN sed -i -- 's/#[ ]*deb-src/deb-src/g' /etc/apt/sources.list
RUN apt-get update
RUN apt-get install -y eatmydata
RUN eatmydata apt-get upgrade -y
RUN eatmydata apt-get install -y openssh-server gdb rsync iputils-ping telnet \
    qtwebengine5-dev \
    libsqlcipher-dev \
    libqt5sql5-* \
    subversion python3-dev ninja-build \
    breeze-icon-theme fonts-noto-color-emoji
RUN eatmydata apt-get build-dep -y kmymoney
