#!make

-include .env

URL ?= "http://localhost:5001"

# override default variable, bt leave it alone if set in environment
ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c11
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2
CFLAGS += -Werror
CFLAGS += -fstack-protector-strong

# gcc only
ifneq "$(findstring gcc,$(CC))" ""
  GCC_MAJOR=$(shell gcc -dumpversion | cut -f1 -d.)
  ifeq "$(shell expr '$(GCC_MAJOR)' '>=' '10')" "1"
    CFLAGS += -ftree-vrp -fanalyzer
  endif

  CFLAGS += -Wformat-overflow=2 -Wformat-truncation=2
  CFLAGS += -Wno-unknown-pragmas
endif
#
# clang only
ifneq "$(findstring clang,$(CC))" ""
  CLANG_MAJOR=$(shell gcc -dumpversion | cut -f1 -d.)
  ifeq "$(shell expr '$(CLANG_MAJOR)' '>=' '14')" "1"
    CFLAGS += -Wno-reserved-identifier
  endif

  CFLAGS += -Weverything
  CFLAGS += -Wno-declaration-after-statement -Wno-disabled-macro-expansion
  CFLAGS += -Wno-reserved-id-macro -Wno-padded
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
	$(CC) $(CFLAGS) -c -o $@ $<

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

