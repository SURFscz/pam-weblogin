#!/usr/bin/env python
import subprocess

# Debug info
# This is printed before activating the shell
print("before bash")
subprocess.call(['sudo', '-iu', 'myshell'])

# This is printed after exiting the shell
print("after bash")
