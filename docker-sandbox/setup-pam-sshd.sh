#!/usr/bin/env bash
set -euo pipefail

if ! command -v docker >/dev/null 2>&1; then
  echo "docker is required" >&2
  exit 1
fi

USERNAME="${USERNAME:-}"
TOKEN="${TOKEN:-}"
TOKEN_FILE="${TOKEN_FILE:-}"
URL="${URL:-https://sram.surf.nl/pam-weblogin}"
ATTRIBUTE="${ATTRIBUTE:-username}"
RETRIES="${RETRIES:-3}"
CACHE_DURATION="${CACHE_DURATION:-30}"

if [[ -z "${USERNAME}" ]]; then
  echo "Set USERNAME before running (example: USERNAME=alice)." >&2
  exit 1
fi

if [[ -n "${TOKEN_FILE}" ]]; then
  if [[ ! -f "${TOKEN_FILE}" ]]; then
    echo "TOKEN_FILE does not exist: ${TOKEN_FILE}" >&2
    exit 1
  fi
  TOKEN="$(<"${TOKEN_FILE}")"
fi

if [[ -z "${TOKEN}" ]]; then
  echo "Set TOKEN (raw token, without Bearer prefix) or TOKEN_FILE (path to file containing the token)." >&2
  exit 1
fi

echo "Starting sandbox..."
docker compose -f docker-sandbox/docker-compose.yml up -d --build

echo "Building pam-weblogin..."
docker exec sandbox bash -lc "cd /home/worker/work && make clean && make"

echo "Writing /etc/security/pam-weblogin.conf (without echoing token)..."
docker exec \
  -e "URL=${URL}" \
  -e "TOKEN=${TOKEN}" \
  -e "ATTRIBUTE=${ATTRIBUTE}" \
  -e "RETRIES=${RETRIES}" \
  -e "CACHE_DURATION=${CACHE_DURATION}" \
  sandbox \
  bash -lc "sudo bash -lc 'umask 077; cat > /etc/security/pam-weblogin.conf <<EOF
url=${URL}
token = Bearer ${TOKEN}
retries = ${RETRIES}
attribute=${ATTRIBUTE}
cache_duration=${CACHE_DURATION}
EOF
chown root:root /etc/security/pam-weblogin.conf
chmod 600 /etc/security/pam-weblogin.conf'"

echo "Installing pam-weblogin module..."
docker exec sandbox bash -lc "cd /home/worker/work && sudo make install URL='${URL}' ATTRIBUTE='${ATTRIBUTE}' RETRIES='${RETRIES}' CACHE_DURATION='${CACHE_DURATION}'"

echo "Ensuring local user exists: ${USERNAME}"
docker exec sandbox bash -lc "id -u '${USERNAME}' >/dev/null 2>&1 || sudo useradd --create-home --shell /bin/bash '${USERNAME}'"

echo "Configuring sshd for keyboard-interactive PAM..."
docker exec sandbox bash -lc "sudo bash -lc 'cat > /etc/ssh/sshd_config.d/40-pamweblogin.conf <<EOF
KbdInteractiveAuthentication yes
ChallengeResponseAuthentication yes
UsePAM yes
PasswordAuthentication no
EOF
chmod 644 /etc/ssh/sshd_config.d/40-pamweblogin.conf
rm -f /etc/ssh/sshd_config.d/90-pamweblogin.conf'"
docker exec sandbox bash -lc "sudo python3 - <<'PY'
from pathlib import Path
p = Path('/etc/pam.d/sshd')
line = 'auth [success=done ignore=ignore default=die] /usr/local/lib/security/pam_weblogin.so /etc/security/pam-weblogin.conf\\n'
text = p.read_text()
if 'pam_weblogin.so' not in text:
    marker = '@include common-auth\\n'
    if marker in text:
        text = text.replace(marker, line + marker, 1)
    else:
        text = line + text
    p.write_text(text)
PY"

docker exec sandbox bash -lc "pid=''; \
if [ -r /run/sshd.pid ]; then pid=\$(sudo sed -n '1p' /run/sshd.pid); \
elif [ -r /var/run/sshd.pid ]; then pid=\$(sudo sed -n '1p' /var/run/sshd.pid); fi; \
if [ -n \"\$pid\" ]; then sudo kill -HUP \"\$pid\" || true; fi; \
if command -v service >/dev/null 2>&1; then \
  sudo service ssh restart || sudo service sshd restart || true; \
elif command -v systemctl >/dev/null 2>&1 && [ \"\$(cat /proc/1/comm 2>/dev/null)\" = \"systemd\" ]; then \
  sudo systemctl restart sshd || true; \
fi; \
sudo /usr/sbin/sshd -t"

echo
echo "Done. Test with:"
echo "ssh -p 2222 -o PubkeyAuthentication=no ${USERNAME}@localhost"
