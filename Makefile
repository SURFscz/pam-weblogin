#!make

-include .env

URL ?= "http://localhost:5001"

module:
	gcc -fPIC -fno-stack-protector -c *.c
	gcc -shared -o pam_websso.so *.o -lcurl -lpam -lm
	chmod 644 pam_websso.so

install: module
	sudo cp pam_websso.so /lib/x86_64-linux-gnu/security/

clean:
	-rm -f *.o
	-rm -f *.so

test: install
	echo "auth required pam_websso.so /etc/pam-websso.conf" | sudo tee "/etc/pam.d/websso"
	echo "url=${URL}" | sudo tee "/etc/pam-websso.conf"
	pamtester websso $(id -u -n) websso authenticate
