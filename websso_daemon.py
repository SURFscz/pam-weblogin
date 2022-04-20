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

auths = {}
cached = {}

chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890'
numbers = '1234567890'

def pop_auth(nonce):
    logging.debug(f"pop auth {nonce}")
    auths.pop(nonce, None)

def pop_cached(user):
    logging.debug(f"pop cached {user}")
    cached.pop(user, None)

def nonce(length=8):
    return ''.join([str(random.choice(chars)) for i in range(length)])

def pin(length=4):
    return ''.join([str(random.choice(numbers)) for i in range(length)])

def authorized(headers):
    auth = headers.get("Authorization")
    if "client:verysecret" in auth:
        return True
    else:
        return False

@app.route('/req', methods=['POST'])
def req():
    if not authorized(request.headers):
        return Response(response="Unauthorized", status=401)

    data = json.loads(request.data)

    user = data.get('user')
    new_nonce = nonce()
    url = os.environ.get("URL", "http://localhost:5001")
    auths[new_nonce] = {
      'nonce': new_nonce,
      'challenge': f"Hello {user}. To continue, visit {url}/login/{new_nonce} and enter pin",
      'cached': cached.get(user, False)
    }

    response = Response()
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(auths[new_nonce])

    new_pin = pin()
    auths[new_nonce]['user'] = user
    auths[new_nonce]['pin'] = new_pin
    Timer(60, pop_auth, [new_nonce]).start()

    logging.debug(f'/req <- {data}\n'
                  f' -> {response.data.decode()}\n'
                  f'  pin: {new_pin}')

    return response

@app.route('/auth', methods=['POST'])
def auth():
    if not authorized(request.headers):
        return Response(response="Unauthorized", status=401)

    data = json.loads(request.data)
    nonce = data.get('nonce')
    rpin = data.get('rpin')

    this_auth = auths.get(nonce)
    if this_auth:
        user = this_auth.get('user')
        pin = this_auth.get('pin')
        if rpin == pin:
            reply = {
                'result': 'SUCCESS',
                'msg': 'Authenticated'
            }
            cached[user] = True;
            pop_auth(nonce)
            Timer(60, pop_cached, [user]).start()
        else:
            reply = {
                'result': 'FAIL',
                'msg': 'Pin failed'
            }
    else:
        reply = {
            'result': None,
            'msg': 'Authentication failed'
        }


    response = Response()
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(reply)

    logging.debug(f'/auth <- {data}\n -> {response.data.decode()}')

    return response

@app.route('/login/<nonce>', methods=['GET', 'POST'])
def login(nonce):
    logging.info(f'/login {nonce}')

    this_auth = auths.get(nonce)
    if this_auth:
        if request.method == 'GET':
            user = this_auth.get('user')
            content =  "<html>\n<body>\n<form method=POST>\n"
            content += f"Please authorize SSH login for user {user}<br />\n"
            content += "<input name=action type=submit value=login>\n"
            content += "</body>\n</html>\n"
        else:
            data = request.data
            user = this_auth['user']
            pin = this_auth['pin']
            content =  "<html>\n<body>\n"
            content += f"{nonce}/{user} successfully authenticated<br />\n"
            content += f"PIN: {pin}<br />\n"
            content += "This window may be closed\n"
            content += "</body>\n</html>\n"
    else:
        content = "<html>\n<body>\n"
        content += "nonce not found\n"
        content += "</body>\n</html>\n"

    response = Response()
    response.data = content

    return response

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5001, debug=False)
