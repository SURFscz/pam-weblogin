#!/usr/bin/env python
import json
from flask import Flask, Response, request
from threading import Timer

app = Flask(__name__)
hots = {}

def pop_hot(user):
    print("pop hot {}".format(user))
    hots.pop(user, None)

@app.route('/req', methods=['POST'])
def req():
    data = json.loads(request.data)
    response = Response()
    response.headers['Content-Type'] = "application/json"
    user = data['user']
    print(f'/req {user}')

    reply = {
      'nonce': '1234',
      'pin': '5678',
      'challenge': f'Hello {user}. To continue, visit http://example.com/login/1234 and enter pin:',
      'hot': hots.get(user, False)
    }
    response.data = json.dumps(reply)

    hots[user] = True;
    Timer(60, pop_hot, [user]).start()

    return response

@app.route('/auth', methods=['POST'])
def auth():
    data = json.loads(request.data)
    response = Response()
    response.headers['Content-Type'] = "application/json"
    nonce = data['nonce']
    print(f'/auth {nonce}')

    reply = {
      'uid': 'user',
      'result': 'SUCCESS'
    }
    response.data = json.dumps(reply)

    return response


if __name__ == "__main__":
    app.run(host='127.0.0.1', port=5001, debug=False)
