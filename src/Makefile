#!make

-include .env

URL ?= "http://localhost:5001"

# override default variable, bt leave it alone if set in environment
ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c17
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2
#CFLAGS += -Werror
CFLAGS += -fstack-protector-strong

# gcc only
ifeq "$(findstring gcc,$(CC))" "gcc"
CFLAGS += -Wformat-overflow=2 -Wformat-truncation=2
CFLAGS += -ftree-vrp -fanalyzer
endif
#
# clang only
ifeq "$(findstring clang,$(CC))" "clang"
CFLAGS += -Weverything
endif

SOURCES := config.c http.c json.c utils.c pam_websso.c
OBJS := $(SOURCES:%.c=%.o)

# default target
.PHONY: all
all: pam_websso.so

# dependencies
config.c: config.h utils.h
http.c: http.h utils.h
json.c: json.h
utils.c: utils.h json.h
pam_websso.c: config.h http.h json.h utils.h

%.o: %.c %.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

pam_websso.so: $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ *.o -lcurl -lpam -lm
	chmod 644 $@

.PHONY: install
install: module
	cp pam_websso.so /lib/x86_64-linux-gnu/security/

.PHONY: clean
clean:
	-rm -f $(OBJS) pam_websso.so

.PHONY: test
test: install
	echo "auth required pam_websso.so /etc/pam-websso.conf" | sudo tee "/etc/pam.d/websso"
	echo "url=${URL}\ntoken = Bearer client:verysecret\nretries = 2\nattribute=uid\n" | sudo tee "/etc/pam-websso.conf"
	pamtester websso $(id -u -n) websso authenticate

