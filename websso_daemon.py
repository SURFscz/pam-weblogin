#!/usr/bin/env python
import json
from flask import Flask, Response, request

app = Flask(__name__)

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
      'challenge': f'Hello {user}',
      'hot': False
    }
    response.data = json.dumps(reply)

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
