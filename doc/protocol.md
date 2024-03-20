# PAM-WebLogin protocol

## Functional

The Pam-WebLogin system is meant for the case in which a server admin wants to allow users to log in to their server based on authentication in a web browser.  This can be used in place of or in addition to regular means of terminal-based authentication, such as ssh public keys or username/password.  It can also be used to enable use of advanced multi-factor authentication methods which would otherwise be unavailable or hard to use on terminal-based systems.

Functionally, the user connects to the system in the regular fashion.  The system then shows a url that the user needs to visit in their web browser; this web site can implement any form of authentication desired.  On successful authentication, a pin code is presented to the user, the user finalizes the login procedure by entering this pin code at the terminal prompt.

## Overview
The picture below gives a schematic overview of the PAM-WebLogin protocol:

![Flow overview](flow.svg "Technical design")

The flow is as follows (more details below):
1. User logs in to a terminal application (e.g. using ssh)
1. SSH starts PAM-WebLogin module
1. Pam-WebLogin initiates a backend call to the WebLogin-server (e.g., `https://weblogin.server/pam-weblogin/start` endpoint).  It sends the incoming `user_id` and receives a `session_id`, a `challenge` and a `cached` indication.
1. If the login was cached, the module immediately returns success.
1. The pam module presents the `challenge` to the user and prompts for a pin
1. The user copies the `challenge_url` (e.g. `https://weblogin.server/weblogin/cloud/57d6ca67-02a1-4235-a5a3-d0cb9df1789a`) to their browser or clicks the link the terminal (it depends on the terminal how links can be activated)
1. The WebLogin-server asks the user to log in (using local accounts, OpenID Connect, etc).
1. If the user has successfully authenticated, the WebLogin server checks if the known (registered) username of this user matches the username that was provided to the `/start` endpoint.
1. If everything checks out, the WebLogin-server generates a pin and presents it to the user.
1. The user enters the pin in the terminal
1. The pam module initiates a new backend call to the WebLogin server; it send the users `sessions` and their entered `pin`; the WebLogin server verifies the pin and responds with a status code and a list of groups the user belongs to in the context of the service.
1. Based on the status code, the pam module allows the user to continue, or asks the user to retry the pin.
1. If there is more than one group, the PAM module will ask the user to choose a group.
1. When the group is chosen, the user enters the Shell.

See also the [sequence diagram](pam-weblogin.plantuml)

![sequence diagram](https://www.plantuml.com/plantuml/proxy?src=https://github.com/SURFscz/pam-weblogin/raw/main/doc/pam-weblogin.plantuml?v=1)

## API description
The WebLogin server needs to implement two API endpoints and a user interface: the start endpoint, the check-pin endpoint and the authentication UI.

Authentication to the API is managed via a Bearer token.

The full API specification is available as an [OpenAPI file](weblogin-api.yml) which can be viewed using Swagger's [Online editor](https://editor-next.swagger.io/)

### Request start endpoint
To initiate a request, send a request to the `/start` endpoint with a json body.  The json object should have three members:
  - `user_id`: the identifier of the user who is trying to log in
  - `attribute`: the attribute or claim from the web authentication process to which the `user_id` should be matched (e.g., `email`, `sub` (for OpenID Connect) or `urn:oasis:names:tc:SAML:attribute:subject-id` for SAML2)
  - `cache_duration`: the duration (in seconds) during which a previous login of the same user should be considered valid, and during which the user does not need to authenticate in the web browser again.

The response can be 401 with a body explaining what went wrond or be a 200 and consist of a json object with the following members:
  - `result`: should always be "OK"
  - `session_id`: identifier to refer to this login sessions later on (specifically, when querying the `/check-pin` API)
  - `challenge`: challenge to show to the user; this includes a URL which the user should visit to authenticate
  - `cached`: whether or not the user has already successfully authenticated in the previous `cache_duration` seconds

Example:
```json
{
  "user_id": "jan.klaassen@uni-harderwijk.nl",
  "attribute": "email",
  "cache_duration": 60
}
```
Reponse:
```json
{
  "result": "OK",
  "session_id": "39277014-1824-4F7A-93EE-8E2FBAA1E816",
  "challenge": "To continue, please visit https://localhost:3000/pam-websso/login/39277014-1824-4F7A-93EE-8E2FBAA1E816 and enter pin below",
  "cached": false
}
```

### Authentication UI
The authentication UI corresponds to the challenge url in the response above.
It should allow the user to log in, on successful authentication, show a PIN number that the user needs to enter in their terminal.

How authentication is managed, is implementation dependent. The server could simply have a local database, could implement social logins (Google, Apple) or use external authentication from an identity federation based on OpenID Connect or SAML2.

### Check pin endpoint
To check an entered pin, send a request to the `/check-pin` endpoint with the following members:
  - `session_id`: id of the authentication session, as returned by the `/start` endpoint
  - `pin`: pin as entered by the user

The response is a json with the following members:
  - `result`: either `SUCCESS` if the pin was correct and the user has authenticated successfully, `TIMEOUT` if the user failed to authenticate in a reasonable time frame, or `FAIL` if the pin is incorrect or the authenticated user didn't match the pam user.
  - `info`: message explaining what has happened.  Not meant to show to the end user.
  - `group`: list containing group objects, consisting of a name and short_name

Example:
```json
{
  "session_id": "39277014-1824-4F7A-93EE-8E2FBAA1E816",
  "pin": "1234"
}
```

Response:
```json
{
  "result": "SUCCESS",
  "info": "Authenticated on attribute 'email'",
  "groups": [
	{ "name": "Example collaboration",
      "short_name": "example_co"
    },
    {
      "name": "HPC CLI demo",
      "short_name": "hpc_cli_demo"
    }
  ]
}
```

### SSH keys endpoint
A service can get list of valid SSH keys of members for this service using the `/ssh_keys` endpoint. No parameters are required, so this is a GET request.
Returned is a list of all valid SSH keys, that can be used to create an `authorized_keys` file e.g.
A 401 is returned with an error message if anything went wrong.

Example:
```json
[
  "ssh-rsa AAAAB3NzaC1yc2EAAA...",
  "ssh-rsa AAAAB3NzaEAAAC1yc2..."
]
```

### Service session endpoint
A Service can request the status of the session using the `/{service_shortname}/{session_id}` endpoint. No parameters are required, so this is a GET request.
Returned is an object containing a `service` key, that contains the service object of this session.
A 401 is returned with an error message if anything went wrong.

Example:
```json
{
  "service": {
    "...": "..."
  }
}
```