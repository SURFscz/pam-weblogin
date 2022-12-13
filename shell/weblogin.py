#!/usr/bin/env python3
import os
import sys
import subprocess
import requests
import json
import getpass

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


def main():
    config_file = '/etc/pam-weblogin.conf'
    with open(config_file) as f:
        config = read_conf(f)

    token = config['token']
    url = config['url'].rstrip('/')
    attribute = config.get('attribute')
    retries = int(config.get('retries', 3))
    verify = config.get('verify')
    path = os.path.dirname(__file__)
    try:
        git_commit = subprocess.check_output(['git', '-C', path, 'describe', '--abbrev=6', '--always']).decode().strip()
    except Exception:
        git_commit = 'Unknown'

    auth = {
        'Authorization': token,
        'Content-Type': 'application/json'
    }

    start = {
        'attribute': attribute,
        'GIT_COMMIT': git_commit
    }

    response = requests.post(f"{url}/start", data=json.dumps(start), headers=auth,
                             verify=verify)
    #print(response.text)
    answer = response.json()
    print(answer['challenge'])

    session_id = answer['session_id']
    check = {'session_id': session_id}

    counter = 0
    while True:
        counter += 1
        if counter > retries:
            break
        code = getpass.getpass("code: ")
        check['pin'] = code
        response = requests.post(f"{url}/check-pin", data=json.dumps(check), headers=auth,
                                 verify=verify)
        #print(response.text)
        answer = response.json()
        result = answer.get('result')
        username = answer.get('username')
        if result == 'SUCCESS' and username:
            subprocess.call(['sudo', '-iu', username])
            break


if __name__ == "__main__":
    main()

