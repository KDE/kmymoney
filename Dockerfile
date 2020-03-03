FROM ubuntu:latest

ENV DEBIAN_FRONTEND noninteractive

RUN echo "tzdata tzdata/Areas select Europe" | debconf-set-selections
RUN echo "tzdata tzdata/Zones/Europe select Madrid" | debconf-set-selections
RUN apt-get update
RUN apt-get install -y apt-utils
RUN apt-get install -y gnupg wget
RUN wget -qO - http://archive.neon.kde.org/public.key | apt-key add -
RUN echo "deb http://archive.neon.kde.org/testing bionic main" >> /etc/apt/sources.list
RUN echo "deb-src http://archive.neon.kde.org/testing bionic main" >> /etc/apt/sources.list
RUN sed -i -- 's/#[ ]*deb-src/deb-src/g' /etc/apt/sources.list
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y openssh-server gdb gdbserver rsync
RUN apt-get build-dep -y kmymoney

RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN sed -i 's/PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# SSH login fix. Otherwise user is kicked off after login
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

# 22 for ssh server. 7777 for gdb server.
EXPOSE 22 7777

RUN useradd -ms /bin/bash debugger
RUN echo 'debugger:pwd' | chpasswd

########################################################
# Add custom packages and development environment here
########################################################

########################################################

CMD ["/usr/sbin/sshd", "-D"]
