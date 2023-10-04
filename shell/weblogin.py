#!/usr/bin/env python3
import os
import sys
from subprocess import run


def read_conf(f):
    c = {}
    while True:
        line = f.readline().strip()
        if not line:
            break
        if line[0:1] == '#':
            continue
        if line.find('=') > 0:
            key, value = [v.strip() for v in line.split('=', 1)]
            c[key] = value

    return c


def main():
    f = open('/tmp/shell.log', 'a')
    pam_user = os.environ.get('PAM_USER', None)
    pam_group = os.environ.get('PAM_GROUP', None)
    f.write(f"args: {sys.argv}\n")
    f.write(f"user: {pam_user}\n")
    f.write(f"group: {pam_group}\n")

    if pam_user is not None:
        username = pam_user
        if pam_group is not None:
            username += "_" + pam_group

        # You need the following sudoers line to make this work:
        # weblogin ALL=(ALL:ALL) NOPASSWD:/usr/bin/bash,/usr/sbin/adduser
        run(['/usr/bin/sudo', '/usr/sbin/adduser', '--disabled-password', '--gecos', '""',
            username])

        command = None
        stop = False
        for arg in sys.argv:
            if stop:
                command = arg
                break
            if arg == '-c':
                stop = True

        f.write(f"command: {command}\n")
        if command is not None:
            f.write(f"Calling {command} for {username}\n")
            f.flush()
            run(['/usr/bin/sudo', '-Hiu', username, command])
            f.write(f"{command} for {username} ended\n")
        else:
            f.write(f"Dropping into shell for {username}\n")
            f.flush()
            run(['/usr/bin/sudo', '-Hiu', username])
            f.write(f"Closing shell for {username}\n")

    f.close()


if __name__ == "__main__":
    main()
