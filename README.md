# pam-weblogin
PAM WebLogin module in C

## Installation
Dependancies: libpam and libcurl. Install the dev packages for these libraries, then

```
$ make
$ make install
```
This copies the pam module to /usr/local/lib/security, creates an example configuration file in /etc/pam-weblogin.conf and a pam example configuration in ```/etc/pam.d/weblogin```:

## Testing
Change into the server directory.

Create python virtualenv, ```pip install -r requirements.txt``` and run weblogin_daemon.py as a stub server on localhost:5001

Install pamtester to test the module (see above under Installation)
```
$ pamtester weblogin [username] authenticate
Hello mrvanes. To continue, visit http://localhost:5001/pam-weblogin/login/yqxvIDZV and enter pin
Pin:
Authenticated on attribute username
pamtester: successfully authenticated
```

## Production (example)
This example enables PAM WebLogin *and* requires either publickey **or** password authentication:

Edit /etc/pam.d/sshd as follows (add the line above @include common-auth)
```
# PAM configuration for the Secure Shell service

auth sufficient /usr/local/lib/security/pam_weblogin.so /etc/pam-weblogin.conf
# Standard Un*x authentication.
@include common-auth
```

Set the following configurations in ```/etc/ssh/sshd_config``` and restart sshd
```
PubkeyAuthentication yes
PasswordAuthentication yes
AuthenticationMethods publickey,keyboard-interactive password,keyboard-interactive
ChallengeResponseAuthentication yes
UsePAM yes
```

Mind that the line with AuthenticationMethods signifies the option of authenticating either via (publickey and keyboard-interactive (pam)) *or* (password and keyboard-interactive), depending on the succes of publickey.