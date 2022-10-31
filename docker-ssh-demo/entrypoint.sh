#!/bin/bash

# Prepare ssh...

mkdir -p /run/sshd

# Make sure all existinig settings are neutralised....

sed -i 's/^UsePAM .*//g' /etc/ssh/sshd_config 
sed -i 's/^PasswordAuthentication .*//g' /etc/ssh/sshd_config 
sed -i 's/^PubkeyAuthentication .*//g' /etc/ssh/sshd_config 
sed -i 's/^KbdInteractiveAuthentication .*//g' /etc/ssh/sshd_config 
sed -i 's/^PermitRootLogin .*//g' /etc/ssh/sshd_config 
sed -i 's/^ChallengeResponseAuthentication .*//g' /etc/ssh/sshd_config 

# Now set my settings !
echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config
echo 'UsePAM yes' >> /etc/ssh/sshd_config
echo 'ChallengeResponseAuthentication yes' >> /etc/ssh/sshd_config
echo 'PasswordAuthentication no' >> /etc/ssh/sshd_config 
echo 'KbdInteractiveAuthentication yes' >> /etc/ssh/sshd_config
echo 'PubkeyAuthentication yes' >> /etc/ssh/sshd_config 
echo 'AuthenticationMethods publickey,keyboard-interactive:pam' >> /etc/ssh/sshd_config 

ssh-keygen -A

# Prepare CRON...

read -r -d '' CRONJOB <<- EOM
  LDAP_PASSWORD="${LDAP_PASSWORD}"
  LDAP_BASE_DN="${LDAP_BASE_DN}"
  LDAP_BIND_DN="${LDAP_BIND_DN}"
  LDAP_HOST="${LDAP_HOST}"
  LDAP_MODE="${LDAP_MODE}"
  LOG_LEVEL="${LOG_LEVEL}"
  python3 /root/sync.py >> /var/log/sync.log 2>&1
EOM
crontab -l | { grep -v sync; echo "* * * * * "$CRONJOB; } | crontab -

# Prepare WebLogin

echo "auth [success=done ignore=ignore default=die] /lib/security/pam_weblogin.so /root/pam-weblogin.conf" > /etc/pam.d/weblogin
sed -i '2i@include weblogin' /etc/pam.d/sshd

cat << EOF > /root/pam-weblogin.conf
url = ${URL}
token = Bearer ${TOKEN}
retries = ${RETRIES:-3}
attribute=${ATTRIBUTE:-username}
cache_duration=${CACHE_DURATION:-60}
EOF

chmod 600 /root/pam-weblogin.conf

# Start services...
service dbus restart
service syslog-ng restart
service cron restart
service ssh restart

# Runtime...
tail -F /var/log/auth.log