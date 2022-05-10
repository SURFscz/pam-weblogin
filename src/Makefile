#!make

-include .env

URL ?= "http://localhost:5001"

# override default variable, bt leave it alone if set in environment
ifeq ($(origin CC),default)
CC = gcc
endif
CC_MAJOR:=$(shell $(CC) -dumpversion | cut -f1 -d.)

CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c11
CFLAGS += -fPIC
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2
CFLAGS += -Werror
CFLAGS += -fstack-protector-strong

# gcc only
ifneq "$(shell $(CC) --version | grep gcc)" ""
  CFLAGS += -Wformat-overflow=2 -Wformat-truncation=2
  CFLAGS += -Wno-unknown-pragmas

  ifeq "$(shell expr '$(CC_MAJOR)' '>=' '10')" "1"
    CFLAGS += -ftree-vrp -fanalyzer
  endif
endif

# clang only
ifneq "$(shell $(CC) --version | grep clang)" ""
  CFLAGS += -Weverything
  CFLAGS += -Wno-declaration-after-statement -Wno-disabled-macro-expansion
  CFLAGS += -Wno-reserved-id-macro -Wno-reserved-identifier
  CFLAGS += -Wno-padded -Wno-poison-system-directories
  ifeq "$(shell expr '$(CC_MAJOR)' '<' '14')" "1"
    CFLAGS += -Wno-unknown-warning-option
  endif
endif

SOURCES := config.c http.c utils.c pam_websso.c
OBJS := $(SOURCES:%.c=%.o)

CFLAGS += -I../json-parser

# default target
.PHONY: all
all: pam_websso.so

# dependencies
config.o: config.c config.h utils.h defs.h
http.o: http.c http.h utils.h defs.h
utils.o: utils.c utils.h defs.h
pam_websso.o: pam_websso.c config.h http.h utils.h defs.h

pam_websso.so: $(OBJS) ../json-parser/libjsonparser.a
	$(CC) $(LDFLAGS) -shared -o $@ $^ -lcurl -lpam -lm
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

