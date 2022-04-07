#!/usr/bin/env python
import json
import random
from flask import Flask, Response, request
from threading import Timer

app = Flask(__name__)

auths = {}
hots = {}

chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890'
numbers = '1234567890'

def pop_auth(nonce):
    print(f"pop auth {nonce}")
    auths.pop(nonce, None)

def pop_hot(user):
    print(f"pop hot {user}")
    hots.pop(user, None)

def nonce(length=8):
    return ''.join([str(random.choice(chars)) for i in range(length)])

def pin(length=4):
    return ''.join([str(random.choice(numbers)) for i in range(length)])

@app.route('/req', methods=['POST'])
def req():
    data = json.loads(request.data)
    user = data.get('user')

    print(f'/req {user}')

    new_nonce = nonce()
    new_pin = pin()

    auths[new_nonce] = {
      'nonce': new_nonce,
      'pin': new_pin,
      'challenge': f"Hello {user}. To continue, visit http://localhost:5001/login/{new_nonce} and enter pin:",
      'hot': hots.get(user, False)
    }

    response = Response()
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(auths[new_nonce])

    auths[new_nonce]['user'] = user
    Timer(60, pop_auth, [new_nonce]).start()

    return response

@app.route('/auth', methods=['POST'])
def auth():
    data = json.loads(request.data)
    nonce = data.get('nonce')
    print(f'/auth {nonce}')

    this_auth = auths.get(nonce)
    if this_auth:
        user = this_auth.get('user')
        reply = {
            'uid': user,
            'result': 'SUCCESS'
        }
        hots[user] = True;
        Timer(5, pop_hot, [user]).start()
    else:
        reply = {
            'uid': None,
            'result': 'FAIL'
        }

    response = Response()
    response.headers['Content-Type'] = "application/json"
    response.data = json.dumps(reply)


    return response

@app.route('/login/<nonce>', methods=['GET', 'POST'])
def login(nonce):
    print(f'/login {nonce}')

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
    app.run(host='127.0.0.1', port=5001, debug=False)
