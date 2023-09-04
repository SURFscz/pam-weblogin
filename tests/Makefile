CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c11
CFLAGS += -fPIC
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2
CFLAGS += -Werror -Wno-gnu-zero-variadic-macro-arguments

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

run_unittests: $(OBJS) ../src/pam_weblogin.a
	$(CC) -o $@ $^ $(LDFLAGS)

unittest: run_unittests
	./run_unittests

.PHONY: coverage
coverage:
	gcov -pb *.c

.PHONY: clean
clean:
	-rm -f $(OBJS) $(GCOVS) run_unittests core

