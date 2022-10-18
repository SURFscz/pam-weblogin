# PAM Weblogin via smart shell
sshd subprocess shell test

- Copy weblogin.py to /usr/bin/weblogin.py and chmod +x

```4 -rwxr-xr-x 1 root root 129 Oct 18 11:21 /usr/bin/weblogin.py```

- Add functional user account 'weblogin' and set shell to /usr/bin/weblogin.py

```weblogin:x:1003:1003:Weblogin,,,:/home/weblogin:/usr/bin/weblogin.py```

- Allow weblogin to sudo (/usr/bin/bash) without password:

```weblogin ALL=(ALL:ALL) NOPASSWD:/usr/bin/bash```

- Add target account myshell (shell /usr/bin/bash)

```
$ ssh weblogin@weblogin
Linux weblogin 5.10.0-15-amd64 #1 SMP Debian 5.10.120-1 (2022-06-09) x86_64

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc/*/copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
Last login: Tue Oct 18 14:48:52 2022 from 192.168.56.1
before bash
Hello myshell. To continue, visit http://localhost:5001/pam-weblogin/login/k4vI3DOT and enter verification code
code:
myshell@weblogin:~$ id
uid=1002(myshell) gid=1002(myshell) groups=1002(myshell)
myshell@weblogin:~$
logout
after bash
Connection to weblogin closed.
```
