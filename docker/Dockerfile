FROM ubuntu:20.04

ARG UID=1000
ARG GID=1000

ENV UID=${UID}
ENV GID=${GID}

RUN apt update
RUN apt install -y openssh-server
RUN echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config
RUN mkdir /run/sshd

RUN apt install -y build-essential make libpam-dev pamtester vim ca-certificates git sudo rsyslog 
RUN apt install -y autoconf libcurl4-gnutls-dev libhiredis-dev libssl-dev

RUN sed -i '/imklog/s/^/#/' /etc/rsyslog.conf
RUN sed -i 's/^ChallengeResponseAuthentication no$/ChallengeResponseAuthentication yes/g' /etc/ssh/sshd_config

RUN ssh-keygen -A

ENV UID=33333
ENV GID=33333

RUN groupadd --gid ${GID} workers \
    && useradd --uid ${UID} --gid workers --shell /bin/bash --create-home worker \
    && adduser worker sudo

ARG PASSWORD=secret
RUN bash -c "echo worker:${PASSWORD} | chpasswd"

RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

WORKDIR /home/worker/work

USER ${UID}

ENTRYPOINT sudo rsyslogd && sudo service ssh restart && tail -f /dev/null