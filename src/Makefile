#!make

-include .env

URL ?= http://localhost:5001/pam-weblogin
TOKEN ?= Bearer 1234
RETRIES ?= 3
CACHE_DURATION ?= 60
ATTRIBUTE ?= username
prefix ?= /usr/local

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

ifdef NOVERIFY
CFLAGS += -DNOVERIFY
endif

# gcc only
ifneq "$(shell $(CC) --version | grep gcc)" ""
  ifeq "$(shell expr '$(CC_MAJOR)' '>=' '5')" "1"
    CFLAGS += -fstack-protector-strong
  endif
  ifeq "$(shell expr '$(CC_MAJOR)' '>=' '10')" "1"
    CFLAGS += -Wformat-overflow=2 -Wformat-truncation=2
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
  ifeq "$(shell expr '$(CC_MAJOR)' '>=' '16')" "1"
    CFLAGS += -Wno-unsafe-buffer-usage
  endif
endif

SOURCES := config.c http.c utils.c tty.c pam_weblogin.c
OBJS := $(SOURCES:%.c=%.o)

CFLAGS += -I..

GIT_COMMIT = "$(shell git describe --abbrev=6 --always 2>/dev/null || echo 'Error')"
JSONPARSER_GIT_COMMIT = "$(shell git -C json-parser describe --abbrev=6 --always 2>/dev/null || echo 'Error')"
CFLAGS += -DGIT_COMMIT=$(GIT_COMMIT)
CFLAGS += -DJSONPARSER_GIT_COMMIT=$(JSONPARSER_GIT_COMMIT)

# default target
.PHONY: all
all: pam_weblogin.so

# dependencies
config.o: config.c config.h tty.h defs.h
http.o: http.c http.h tty.h defs.h
utils.o: utils.c utils.h tty.h defs.h
tty.o: tty.c utils.h defs.h
pam_weblogin.o: pam_weblogin.c config.h http.h utils.h tty.h defs.h

pam_weblogin.so: $(OBJS) ../json-parser-build/libjsonparser.a
	$(CC) $(LDFLAGS) -shared -o $@ $^ -lcurl -lpam -lm
	chmod 644 $@

.PHONY: install
install: pam_weblogin.so
	install -d $(DESTDIR)$(prefix)/lib/security/
	install pam_weblogin.so $(DESTDIR)$(prefix)/lib/security/
	@if [ ! -d $(DESTDIR)/etc/security ]; then\
		mkdir -p $(DESTDIR)/etc/security;\
	fi
	@if [ ! -f $(DESTDIR)/etc/security/pam-weblogin.conf ]; then\
		printf "url=${URL}\ntoken = Bearer ${TOKEN}\nretries = ${RETRIES}\nattribute=${ATTRIBUTE}\ncache_duration=${CACHE_DURATION}\n" | tee "$(DESTDIR)/etc/security/pam-weblogin.conf";\
		chmod 600 $(DESTDIR)/etc/security/pam-weblogin.conf;\
	fi
	@if [ ! -f $(DESTDIR)/etc/pam.d/weblogin ]; then\
		echo "auth required $(prefix)/lib/security/pam_weblogin.so /etc/security/pam-weblogin.conf" | tee "$(DESTDIR)/etc/pam.d/weblogin";\
	fi

.PHONY: clean
clean:
	-rm -f $(OBJS) pam_weblogin.so

.PHONY: test
test:
	pamtester weblogin "$${USERNAME:-unspecified}" authenticate

