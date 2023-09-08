# PAM Weblogin via smart shell
sshd subprocess shell test

Given you have checked out pam-weblogin in /opt/pam-weblogin

- Add functional user account 'weblogin' and set shell to /opt/pam-weblogin/shell/weblogin.py

```weblogin:x:1003:1003:Weblogin,,,:/home/weblogin:/usr/bin/weblogin.py```

- Allow weblogin to sudo (/usr/bin/bash) without password:

```weblogin ALL=(ALL:ALL) NOPASSWD:/usr/bin/bash,/usr/sbin/adduser```

Possibly
- Add sshd AuthorizedKeysCommand config

```
AuthorizedKeysCommand /opt/pam-weblogin/shell/authorized_keys.py
AuthorizedKeysCommandUser root
```
Or, allow weblogin user to pass sshd in PAM:
```
auth sufficient pam_succeed_if.so quiet uid eq 1003
```
- Change sshd authentication to:
```
AuthenticationMethods keyboard-interactive:pam
PasswordAuthentication no
UsePAM yes
```

If you don't use the JIT adduser command
- Add target account myshell (shell /usr/bin/bash)

```
# adduser myshell
```

Create /etc/pam-weblogin.conf and make it readable to the ```weblogin``` user:
```
url=https://sram.surf.nl/pam-weblogin
token = Bearer <token>
retries = 3
attribute=username
cache_duration = 60
```
```
$ ssh weblogin@weblogin
Linux weblogin 5.10.0-15-amd64 #1 SMP Debian 5.10.120-1 (2022-06-09) x86_64

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc/*/copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
Last login: Tue Oct 18 14:48:52 2022 from 192.168.56.1
Hello myshell. To continue, visit http://localhost:5001/pam-weblogin/login/k4vI3DOT and enter verification code
code:
myshell@weblogin:~$ id
uid=1002(myshell) gid=1002(myshell) groups=1002(myshell)
myshell@weblogin:~$
logout
Connection to weblogin closed.
```
