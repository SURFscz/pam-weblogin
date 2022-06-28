# pam-weblogin
PAM WebLogin module in C

## Installation
Dependancies: libpam and libcurl. Install the dev packages for these libraries, then

```
$ make
$ make install
```

Add this file to ```/etc/pam.d/weblogin```

```
auth required pam_weblogin.so /etc/pam-weblogin.conf
```

Add pam-weblogin.conf configuration file to ```/etc/pam-weblogin.conf```

```
url = http://localhost:5001
token = Bearer client:verysecret
retries = 3
```

## Testing
Create python virtualenv, pip install Flask and run weblogin-daemon.py as a stub server on localhost:5001

Install pamtester to test the module

```
$ pamtester weblogin martin authenticate
Authenticate
config: /etc/pam-weblogin.conf
config 'url' -> 'http://localhost:5001'
cfg->url: http://localhost:5001
http URL: http://localhost:5001/req
http data: {"user":"martin"}
req: {"nonce": "ji1bI8RJ", "pin": "6333", "challenge": "Hello martin. To continue, visit http://localhost:5001/login/ji1bI8RJ and enter pin:", "hot": false}
Hello martin. To continue, visit http://localhost:5001/login/ji1bI8RJ and enter pin:
Pin: 6333
Pin matched!
http URL: http://localhost:5001/auth
http data: {"nonce":"ji1bI8RJ"}
auth: {"uid": "martin", "result": "SUCCESS"}
user: martin
auth_result: SUCCESS
pamtester: successfully authenticated
```

## Production
Insert the pam-weblogin module below common-auth
```
# Standard Un*x authentication.
@include common-auth
auth required /usr/local/lib/security/pam_weblogin.so /etc/pam-weblogin.conf
```
Set sshd ```ChallengeResponseAuthentication``` and ```UsePAM``` to ```yes``` and restart sshd
```
# Change to yes to enable challenge-response passwords (beware issues with
# some PAM modules and threads)
ChallengeResponseAuthentication yes
...
UsePAM yes
```