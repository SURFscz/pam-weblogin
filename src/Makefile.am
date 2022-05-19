test: install
	echo "auth required pam_websso.so /etc/pam-websso.conf" | sudo tee "/etc/pam.d/websso"
	echo -e "url=${URL}\ntoken = Bearer ${TOKEN}\nretries = ${RETRIES}\nattribute = ${ATTRIBUTE}\n" | sudo tee "/etc/pam-websso.conf"
	pamtester websso $${USERNAME} authenticate

INSTALL = ./install-sh
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = pam_websso.so
pam_websso_so_SOURCES = pam_websso.c http.c config.c utils.c tty.c
pam_websso_so_CFLAGS= -fPIC
pam_websso_so_LDFLAGS= -shared -fPIC
