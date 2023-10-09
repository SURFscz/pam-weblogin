# PAM Weblogin via smart shell
This file describes an optional way to use pam-weblogin to allow users to log in with a generic username, which will then be changed dynamically to the user's real usename on the system.

Note that this is still beta-functionality and has not been thoroughly tested.

### sshd subprocess shell test

Given you have checked out pam-weblogin in /opt/pam-weblogin

Add functional user account 'weblogin'
```
# adduser weblogin --gecos "" --disabled-password
```
and set shell to /opt/pam-weblogin/shell/weblogin.py (/etc/passwd)
```
weblogin:x:1003:1003:Weblogin,,,:/home/weblogin:/opt/pam-weblogin/shell/weblogin.py
```

- Allow weblogin to sudo (bash and adduser) without password (/etc/sudoers.d/weblogin)

```weblogin ALL=(ALL:ALL) NOPASSWD:/usr/bin/bash,/usr/sbin/adduser```

Add pam-weblogin sshd config in PAM (/etc/pam.d/sshd)
```
auth sufficient /usr/local/lib/security/pam_weblogin.so /etc/security/pam-weblogin.conf
# Standard Un*x authentication.
@include common-auth
```
- Change sshd authentication to (/etc/ssh/sshd_config)
```
AuthenticationMethods publickey keyboard-interactive:pam
PubkeyAuthentication yes
KbdInteractiveAuthentication yes
UsePAM yes
```

Create /etc/security/pam-weblogin.conf and make it readable to the ```root``` user only:
```
url=https://sram.surf.nl/pam-weblogin
token = <token>
retries = 3
attribute=username
cache_duration = 60
pam_user
```
- A bare pam_user line (without '='!) signifies the overruling of the login user with web authenticated user.

### Login flow:
```
$ ssh weblogin@<server>
(weblogin@<server>) Please sign in to: https://sbs.scz-vm.net/weblogin/pamweblogin/....
...

Verification code:
(weblogin@<server>) User admin has authenticated successfully

What group are you operating for?
  [1] AI computing
  [2] UU

Select group: 2
Linux weblogin 6.1.0-12-amd64 #1 SMP PREEMPT_DYNAMIC Debian 6.1.52-1 (2023-09-07) x86_64

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc/*/copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
Last login: Wed Sep 20 16:32:54 2023 from 192.168.56.1
ascz_uu_short@<server>:~$
```
