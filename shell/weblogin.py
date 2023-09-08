#!/usr/bin/env python3
import os
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
        colist = answer.get('colist', None)
        result = answer.get('result')
        username = answer.get('username')
        if result == 'SUCCESS' and username:
            selected_co = 0
            selected = False
            print()
            if len(colist.keys()) > 1:
                while not selected:
                    i = 0
                    print('What CO are you operating for?')
                    for co, co_name in colist.items():
                        i += 1
                        print(f"  [{i}] {co_name}")
                    try:
                        selected_co = int(input("\nSelect CO: "))
                    except Exception:
                        print("\nPlease use a number to select the CO")
                        continue
                    if selected_co > 0 and selected_co <= len(colist):
                        selected = True
                    else:
                        print(f"\nNumber not in valid range (1 - {len(colist)})")
            else:
                selected_co = 1

            print("\nYou chose wisely...")
            my_co = list(colist.keys())[selected_co - 1]
            # You need the following sudoers line to make this work:
            # weblogin ALL=(ALL:ALL) NOPASSWD:/usr/bin/bash,/usr/sbin/adduser
            subprocess.call(['/usr/bin/sudo', '/usr/sbin/adduser', '--disabled-password', '--gecos', '""',
                             username + "_" + my_co], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            subprocess.call(['/usr/bin/sudo', '-iu', username + "_" + my_co])
            break


if __name__ == "__main__":
    main()
