#!/usr/bin/env python
import sys
import os
import json
import random
import logging
import yaml
from threading import Timer
from datetime import timedelta

from flask import Flask, Response, request, Markup, session, render_template
from flask_pyoidc import OIDCAuthentication
from flask_pyoidc.provider_configuration import ProviderConfiguration, ClientMetadata
from flask_pyoidc.user_session import UserSession
from flask_session import Session

logging.getLogger().setLevel(logging.DEBUG)
logging.getLogger('flask_pyoidc').setLevel(logging.ERROR)
logging.getLogger('oic').setLevel(logging.ERROR)
logging.getLogger('jwkest').setLevel(logging.ERROR)
logging.getLogger('urllib3').setLevel(logging.ERROR)
logging.getLogger('werkzeug').setLevel(logging.ERROR)

app = Flask(__name__, template_folder='templates', static_folder='static')

with open(sys.argv[1]) as f:
    config = yaml.safe_load(f)

appConfig = {
    "OIDC_REDIRECT_URI": config['oidc']['redirect_uri'],
    "SESSION_PERMANENT": False,
    "SESSION_TYPE": "filesystem",
    "PERMANENT_SESSION_LIFETIME": timedelta(hours=8),
    "SESSION_COOKIE_SAMESITE": "Lax",
}

app.config.from_mapping(appConfig)
Session(app)

oidc_enabled = config['oidc']['enabled']

if oidc_enabled:
    client_metadata = ClientMetadata(
        client_id=config['oidc']['client_id'],
        client_secret=config['oidc']['client_secret'])

    provider_config = ProviderConfiguration(
        issuer=config['oidc']['issuer'],
        client_metadata=client_metadata)

    authzn = OIDCAuthentication({'pam-weblogin': provider_config}, app)
else:
    authzn = None

timeout = config['timeout']

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
    url = os.environ.get("URL", config['url']).rstrip('/')
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
    Timer(timeout, pop_auth, [new_session_id]).start()

    logging.debug(f'/pam-weblogin/start <- {data}\n'
                  f' -> {response.data.decode()}\n'
                  f'  code: {new_code}')

    return response


@app.route('/pam-weblogin/check-pin', methods=['POST'])
def check_pin():
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


def __login(session_id):
    logging.info(f'/pam-weblogin/login/{session_id}')

    try:
        user_session = UserSession(session)
        userinfo = user_session.userinfo
    except Exception:
        userinfo = {}

    this_auth = auths.get(session_id)
    if this_auth:
        request.data
        user_id = this_auth.get('user_id')
        attribute_id = userinfo.get(this_auth.get('attribute'))
        logging.info(f"user_id: {user_id}, attribute_id: {attribute_id}")
        if (user_id
            and attribute_id
            and user_id in attribute_id
            or not oidc_enabled):
            user_id = Markup.escape(user_id)
            attribute_id = Markup.escape(attribute_id)
            code = this_auth['code']
            code = Markup.escape(code)
            message = f"<h1>SSH request</h1>\n"
            message += f"for session {session_id}/{user_id}<br>\n"
            message += f"{attribute_id} successfully authenticated<br>\n"
            message += f"Verification code: {code}<br><br>\n"
            message += "<i>This window may be closed</i>\n"
        else:
            message = f"user_id {user_id} not found\n"
    else:
        message = "session_id not found\n"

    response = render_template('login.j2', message=message)

    return response


if isinstance(authzn, OIDCAuthentication):
    __login = authzn.oidc_auth('pam-weblogin')(__login)


@app.route('/pam-weblogin/login/<session_id>', methods=['GET'])
def login(session_id):
    return __login(session_id)


if __name__ == "__main__":
    app.run(host=config['host'], port=config['port'])
