# pam-weblogin test server
The test server to test the module stand-alone.

## Installation
Create a virtualenv and ```pip install -r requirements.txt```

## Configuring PAM module
Configure pam-weblogin to connect to http://localhost:8080/pam-weblogin and use a placeholder Bearer token:
```
url=http://localhost:8080/pam-weblogin
token = Bearer TestToken
retries = 3
attribute=email
cache_duration = 600
```

## Configuring the daemon
The daemon can be configured with and without a real OIDC interaction by toggling oidc.enable config parameter:
```
---
host: '0.0.0.0'
port: 8080
url: 'http://localhost:8080'
timeout: 60
oidc:
  enabled: True
  issuer: '<OIDC Provider base url>'
  client_id: '<client>'
  client_secret: '<secret>'
  redirect_uri: 'https://<server>>/redirect_uri
```

When oidc is disabled, the login endpoint will show a login button that immediately logs in the user and shows the verification code.
When oidc is enabled, a real roundtrip to the oidc provider is executed and the real required attribute is used to check the SSH principal.

## Quick test
The daemon will print the verification code in the logs, so you don't have to visit the login URL to test verification code success flow.
