#!/usr/bin/env python3
import requests

def read_conf(f):
    c = {}
    while True:
        line = f.readline().strip()
        if not line:
            break
        if line[0:1] == '#':
            continue
        if line.find('=') > 0:
            key, value = [v.strip() for v in line.split('=', 1)]
            c[key] = value

    return c

def get_authorized_keys(url, auth, verify):
    keys = []
    response = requests.get(f"{url}/ssh_keys", headers=auth, verify=verify)
    answer = response.json()
    return answer

def main():
    config_file = '/etc/pam-weblogin.conf'
    with open(config_file) as f:
        config = read_conf(f)

    token = config.get('token')
    url = config['url'].rstrip('/')
    verify = config.get('verify')
    auth = {
        'Authorization': token
    }
    keys = get_authorized_keys(url, auth, verify)

    for key in keys:
        print(key)

if __name__ == "__main__":
    main()

