FROM ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive

RUN echo "tzdata tzdata/Areas select Europe" | debconf-set-selections
RUN echo "tzdata tzdata/Zones/Europe select Madrid" | debconf-set-selections
RUN apt update \
    && apt install -y apt-utils gnupg wget \
    && wget -qO - http://archive.neon.kde.org/public.key | apt-key add - \
    && echo "deb http://archive.neon.kde.org/user focal main" >> /etc/apt/sources.list \
    && echo "deb-src http://archive.neon.kde.org/user focal main" >> /etc/apt/sources.list \
    && sed -i -- 's/#[ ]*deb-src/deb-src/g' /etc/apt/sources.list \
    && apt update \
    && apt install -y eatmydata \
    && eatmydata apt upgrade -y \
    && eatmydata apt install -y openssh-server gdb rsync iputils-ping telnet \
    && eatmydata apt build-dep -y kmymoney \
    && eatmydata apt install -y qt5-default \
    && eatmydata apt install -y libsqlcipher-dev \
    && eatmydata apt install -y libqt5sql5-* \
    && eatmydata apt install -y subversion python3-dev ninja-build \
    && eatmydata apt install -y breeze-icon-theme fonts-noto-color-emoji

RUN mkdir /var/run/sshd \
    && echo 'root:root' | chpasswd \
    && sed -i 's/#\?PermitRootLogin .*/PermitRootLogin yes/' /etc/ssh/sshd_config

# SSH login fix. Otherwise user is kicked off after login
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

# 22 for ssh server. 
EXPOSE 22 

########################################################
# Add custom packages and development environment here
########################################################

########################################################

CMD ["/usr/sbin/sshd", "-D"]
