# override default variable, bt leave it alone if set in environment
ifeq ($(origin CC),default)
CC = gcc
endif
CC_MAJOR:=$(shell $(CC) -dumpversion | cut -f1 -d.)

GCOV ?= gcov

CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c11
CFLAGS += -fPIC
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2
CFLAGS += -Wno-gnu-zero-variadic-macro-arguments

CHECK_CFLAGS := $(shell pkg-config --cflags check)
CHECK_LDFLAGS := $(shell pkg-config --libs check)

CPPFLAGS += -I../ $(CHECK_CFLAGS)
LDFLAGS += $(CHECK_LDFLAGS) -lpam

ifdef COVERAGE
	CFLAGS += -fprofile-arcs -ftest-coverage
	LDFLAGS += -fprofile-arcs -ftest-coverage
endif

SOURCES := $(wildcard *.c)
OBJS := $(SOURCES:%.c=%.o)
GCOVS := $(SOURCES:%.c=%.gcov) $(SOURCES:%.c=%.gcno) $(SOURCES:%.c=%.gcda) $(SOURCES:%.c=%.c.gcov)

.PHONY: all
all: run_unittests

run_unittests: $(OBJS) ../src/pam_weblogin.a ../json-parser-build/libjsonparser.a
	$(CC) -o $@ $^ $(LDFLAGS)

unittest: run_unittests
	./run_unittests

.PHONY: coverage
coverage:
	$(GCOV) -pb $(wildcard *.gcda)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(GCOVS) run_unittests core \#*.gcov

