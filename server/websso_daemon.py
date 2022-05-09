#!/usr/bin/env python

import os
import json
import random
import logging
from flask import Flask, Response, request
from threading import Timer

logging.getLogger().setLevel(logging.DEBUG)
logging.getLogger('werkzeug').setLevel(logging.ERROR)

app = Flask(__name__)

TIMEOUT = 60

auths = {}
cached = {}

chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890'
numbers = '1234567890'


def pop_auth(session_id):
    logging.debug(f"pop auth {session_id}")
    auths.pop(session_id, None)


def pop_cached(user):
    logging.debug(f"pop cached {user}")
    cached.pop(user, None)


def session_id(length=8):
    return ''.join([str(random.choice(chars)) for i in range(length)])


def pin(length=4):
    return ''.join([str(random.choice(numbers)) for i in range(length)])


def authorized(headers):
    auth = headers.get("Authorization")
    if "client:verysecret" in auth:
        return True
    else:
        return False


@app.route('/pam-websso/start', methods=['POST'])
def req():
    if not authorized(request.headers):
        return Response(response="Unauthorized", status=401)

    data = json.loads(request.data)

    user = data.get('user')
    attribute = data.get('attribute')
    cache_duration = data.get('cache_duration')
    new_session_id = session_id()
    url = os.environ.get("URL", "http://localhost:5001")
    auths[new_session_id] = {
        'session_id': new_session_id,
        'challenge': f'Hello {user}. To continue, '
                     f'visit {url}/pam-websso/login/{new_session_id} and enter pin',
        'cached': cached.get(user, False)
    }

    response = Response()
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(auths[new_session_id])

    new_pin = pin()
    auths[new_session_id]['user'] = user
    auths[new_session_id]['attribute'] = attribute
    auths[new_session_id]['pin'] = new_pin
    auths[new_session_id]['cache_duration'] = cache_duration
    Timer(TIMEOUT, pop_auth, [new_session_id]).start()

    logging.debug(f'/pam-websso/start <- {data}\n'
                  f' -> {response.data.decode()}\n'
                  f'  pin: {new_pin}')

    return response


@app.route('/pam-websso/check-pin', methods=['POST'])
def auth():
    if not authorized(request.headers):
        return Response(response="Unauthorized", status=401)

    data = json.loads(request.data)
    session_id = data.get('session_id')
    rpin = data.get('rpin')

    this_auth = auths.get(session_id)
    if this_auth:
        user = this_auth.get('user')
        attribute = this_auth.get('attribute')
        pin = this_auth.get('pin')
        cache_duration = this_auth.get('cache_duration')
        if rpin == pin:
            reply = {
                'result': 'SUCCESS',
                'msg': f'Authenticated on attribute {attribute}'
            }
            cached[user] = True
            pop_auth(session_id)
            Timer(int(cache_duration), pop_cached, [user]).start()
        else:
            reply = {
                'result': 'FAIL',
                'msg': 'Pin failed'
            }
    else:
        reply = {
            'result': 'TIMEOUT',
            'msg': 'Authentication failed'
        }

    response = Response()
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(reply)

    logging.debug(f'/pam-websso/check-pin <- {data}\n -> {response.data.decode()}')

    return response


@app.route('/pam-websso/login/<session_id>', methods=['GET', 'POST'])
def login(session_id):
    logging.info(f'/pam-websso/login {session_id}')

    this_auth = auths.get(session_id)
    if this_auth:
        if request.method == 'GET':
            user = this_auth.get('user')
            content = "<html>\n<body>\n<form method=POST>\n"
            content += f"Please authorize SSH login for user {user}<br />\n"
            content += "<input name=action type=submit value=login>\n"
            content += "</body>\n</html>\n"
        else:
            request.data
            user = this_auth['user']
            pin = this_auth['pin']
            content = "<html>\n<body>\n"
            content += f"{session_id}/{user} successfully authenticated<br />\n"
            content += f"PIN: {pin}<br />\n"
            content += "This window may be closed\n"
            content += "</body>\n</html>\n"
    else:
        content = "<html>\n<body>\n"
        content += "session_id not found\n"
        content += "</body>\n</html>\n"

    response = Response()
    response.data = content

    return response


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5001, debug=False)
