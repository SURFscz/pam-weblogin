[![Build status](https://github.com/SURFscz/pam-weblogin/actions/workflows/build.yml/badge.svg)](https://github.com/SURFscz/pam-weblogin/actions)

# pam-weblogin

PAM WebLogin module in C

## Wiki

Accompanying wiki page for this project https://edu.nl/hjhqc

## Installation

Dependancies: libpam and libcurl. Install the dev packages for these libraries, then

```
$ make
$ make install
```

This copies the pam module to /usr/local/lib/security, creates an example configuration file in /etc/pam-weblogin.conf and a pam example configuration in `/etc/pam.d/weblogin`:

## Testing

Change into the server directory.

Create python virtualenv, `pip install -r requirements.txt` and run weblogin_daemon.py as a stub server on localhost:5001

Install pamtester to test the module (see above under Installation)

```
$ pamtester weblogin [username] authenticate
Hello mrvanes. To continue, visit http://localhost:5001/pam-weblogin/login/yqxvIDZV and enter pin
Pin:
Authenticated on attribute username
pamtester: successfully authenticated
```

## Production (example)

This example enables PAM WebLogin _and_ requires either publickey **or** password authentication:

Edit `sshd` as follows (add the line above @include common-auth)

### /etc/pam.d/sshd

```
# PAM configuration for the Secure Shell service

auth [success=done ignore=ignore default=die] /usr/local/lib/security/pam_weblogin.so /etc/pam-weblogin.conf
# Standard Un*x authentication.
@include common-auth
```

If you put these lines _above_ the `auth required` line:

```
auth [success=4 default=ignore] pam_access.so accessfile=/etc/security/exclude_iprange.conf
auth [success=3 default=ignore] pam_succeed_if.so quiet user ingroup admin
auth [success=2 default=ignore] pam_succeed_if.so quiet uid < 70000000
auth [success=1 default=ignore] pam_succeed_if.so quiet uid > 72999999
```

Logins coming from exclude range, users in the admin group and uids between 70000000 and 72999999 are allowed to continue without pam-weblogin.

Set the following configurations in `sshd_config` and restart sshd

### /etc/ssh/sshd_config

```
PubkeyAuthentication yes
PasswordAuthentication yes
AuthenticationMethods publickey,keyboard-interactive password,keyboard-interactive
ChallengeResponseAuthentication yes
UsePAM yes
```

Mind that in this example the line AuthenticationMethods signifies the option of authenticating either via (publickey and keyboard-interactive (pam)) _or_ (password and keyboard-interactive), depending on the success of publickey.

Add the `pam-weblogin.conf` and make it readable only for root (`chmod 600 /etc/pam-weblogin.conf, chown root.root /etc/pam-weblogin.conf`).

### /etc/pam-weblogin.conf

```
url = https://sram.surf.nl/pam-weblogin
token = Bearer <replace with SRAM API TOKEN for your service>
retries = 3
attribute = email
cache_duration = 30
verify = /etc/ssl/ca.crt
```

-   `url` is the pam-weblogin endpoint of the weblogin server
-   `token` is the complete HTTP `Authorization` header, including `Bearer`
-   `retries` is the number of verification code retries allowed
-   `cache_duration` is the time the server should respond with a cached answer instead of reauthenticating the user, in seconds
-   `verify` alternative SSL CA, for debug purposes

## Locking yourself out

Please make sure to create a way of accessing the machine in case you create a configuration that effectively locks you out of the machine. Practice on a local VM first.

## Standalone server

Whereas this pam module was developed for use with [SRAM](https://wiki.surfnet.nl/display/SRAM) [SBS](https://github.com/SURFscz/SBS), it is also easlity possible to integrate it in your own infrastructure. To that ebd, we provide a fully fuctional `pam-weblogin` server which can authenticate users by acting as an OIDC RP in an existing infratructure. See the Readme file in `server/` for more info.
