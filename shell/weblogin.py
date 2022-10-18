#!/usr/bin/env python
import subprocess
import requests
import json
import getpass

authheader = {
    'Authorization': 'Bearer Token'
}

start = {
    'user_id': 'myshell',
    'attribute': 'mail',
    'cache_duration': 60
}

response = requests.post('http://localhost:8080/pam-weblogin/start', data=json.dumps(start), headers=authheader)
answer = response.json()
print(answer['challenge'])

session_id = answer['session_id']
check = {'session_id': session_id}

counter = 0
while True:
    counter += 1
    if counter > 3:
        break
    code = getpass.getpass("code: ")
    check['pin'] = code
    response = requests.post('http://localhost:8080/pam-weblogin/check-pin', data=json.dumps(check), headers=authheader)
    answer = response.json()
    result = answer['result']
    if result == 'SUCCESS':
        break

subprocess.call(['sudo', '-iu', 'myshell'])
