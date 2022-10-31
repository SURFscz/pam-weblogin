
FROM ubuntu:latest AS builder

RUN apt update
RUN apt install -y build-essential git
RUN apt install -y make autoconf libpam-dev libcurl4-gnutls-dev libssl-dev pamtester

RUN git clone https://github.com/SURFscz/pam-weblogin /build

WORKDIR /build

RUN make && make install

FROM ubuntu:latest

RUN apt update
RUN apt install -y apt-transport-https locales ca-certificates vim
RUN apt install -y openssh-server pamtester libcurl4-gnutls-dev
RUN apt install -y libsasl2-dev libldap2-dev ldap-utils 
RUN apt install -y python3 python3-pip
RUN apt install -y cron syslog-ng

RUN mkdir -p /lib/security

COPY --from=builder /usr/local/lib/security/pam_weblogin.so /lib/security

ADD requirements.txt .
RUN pip install -r requirements.txt && rm requirements.txt

COPY ./sync.py /root/sync.py
COPY ./entrypoint.sh /
RUN chmod o+x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
