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


def pop_cached(user_id):
    logging.debug(f"pop cached {user_id}")
    cached.pop(user_id, None)


def session_id(length=8):
    return ''.join([str(random.choice(chars)) for i in range(length)])


def code(length=4):
    return ''.join([str(random.choice(numbers)) for i in range(length)])


def authorized(headers):
    auth = headers.get("Authorization")
    if "Bearer" in auth:
        return True
    else:
        return False


@app.route('/pam-weblogin/start', methods=['POST'])
def req():
    if not authorized(request.headers):
        return Response(response="Unauthorized", status=401)

    data = json.loads(request.data)

    user_id = data.get('user_id')
    attribute = data.get('attribute')
    cache_duration = data.get('cache_duration')
    new_session_id = session_id()
    url = os.environ.get("URL", "http://localhost:5001")
    cache = cached.get(user_id, False)
    auths[new_session_id] = {
        'session_id': new_session_id,
        'challenge': f'Hello {user_id}. To continue, '
                     f'visit {url}/pam-weblogin/login/{new_session_id} and enter verification code',
        'cached': cache,
        'info': 'Login was cached' if cache else 'Sign in'
    }

    response = Response(status=201)
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(auths[new_session_id])

    new_code = code()
    auths[new_session_id]['user_id'] = user_id
    auths[new_session_id]['attribute'] = attribute
    auths[new_session_id]['code'] = new_code
    auths[new_session_id]['cache_duration'] = cache_duration
    Timer(TIMEOUT, pop_auth, [new_session_id]).start()

    logging.debug(f'/pam-weblogin/start <- {data}\n'
                  f' -> {response.data.decode()}\n'
                  f'  code: {new_code}')

    return response


@app.route('/pam-weblogin/check-pin', methods=['POST'])
def auth():
    if not authorized(request.headers):
        return Response(response="Unauthorized", status=401)

    data = json.loads(request.data)
    session_id = data.get('session_id')
    rcode = data.get('pin')

    this_auth = auths.get(session_id)
    if this_auth:
        user_id = this_auth.get('user_id')
        attribute = this_auth.get('attribute')
        code = this_auth.get('code')
        cache_duration = this_auth.get('cache_duration')
        if rcode == code:
            reply = {
                'result': 'SUCCESS',
                'info': f'Authenticated on attribute {attribute}'
            }
            cached[user_id] = True
            pop_auth(session_id)
            Timer(int(cache_duration), pop_cached, [user_id]).start()
        else:
            reply = {
                'result': 'FAIL',
                'info': 'Verification failed'
            }
    else:
        reply = {
            'result': 'TIMEOUT',
            'info': 'Authentication failed'
        }

    response = Response(status=201)
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(reply)

    logging.debug(f'/pam-weblogin/check-pin <- {data}\n -> {response.data.decode()}')

    return response


@app.route('/pam-weblogin/login/<session_id>', methods=['GET', 'POST'])
def login(session_id):
    logging.info(f'/pam-weblogin/login {session_id}')

    this_auth = auths.get(session_id)
    if this_auth:
        if request.method == 'GET':
            user_id = this_auth.get('user_id')
            content = "<html>\n<body>\n<form method=POST>\n"
            content += f"Please authorize SSH login for user {user_id}<br />\n"
            content += "<input name=action type=submit value=login>\n"
            content += "</body>\n</html>\n"
        else:
            request.data
            user_id = this_auth['user_id']
            code = this_auth['code']
            content = "<html>\n<body>\n"
            content += f"{session_id}/{user_id} successfully authenticated<br />\n"
            content += f"Verification code: {code}<br />\n"
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
