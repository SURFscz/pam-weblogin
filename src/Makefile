#!make

-include .env

URL ?= "http://localhost:5001"

# override default variable, bt leave it alone if set in environment
ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c17
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2 -Wno-unknown-pragmas
#CFLAGS += -Werror
CFLAGS += -fstack-protector-strong

# gcc only
ifneq "$(findstring gcc,$(CC))" ""
CFLAGS += -Wformat-overflow=2 -Wformat-truncation=2
CFLAGS += -ftree-vrp -fanalyzer
endif
#
# clang only
ifneq "$(findstring clang,$(CC))" ""
CFLAGS += -Weverything
CFLAGS += -Wno-declaration-after-statement -Wno-disabled-macro-expansion
endif

SOURCES := config.c http.c utils.c pam_websso.c
OBJS := $(SOURCES:%.c=%.o)

CFLAGS += -I../json-parser
LDFLAGS += -L../json-parser

# default target
.PHONY: all
all: pam_websso.so

# dependencies
config.c: config.h utils.h
http.c: http.h utils.h
utils.c: utils.h
pam_websso.c: config.h http.h utils.h

%.o: %.c %.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

pam_websso.so: $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ *.o -lcurl -lpam -lm -ljsonparser
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

