CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c11
CFLAGS += -fPIC
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2
CFLAGS += -Werror -Wno-gnu-zero-variadic-macro-arguments

CHECK_CFLAGS := $(shell pkg-config --cflags check)
CHECK_LDFLAGS := $(shell pkg-config --libs check)

CPPFLAGS += -I../ $(CHECK_CFLAGS)
LDFLAGS += $(CHECK_LDFLAGS) -lpam

SOURCES := $(wildcard *.c)
OBJS := $(SOURCES:%.c=%.o)

.PHONY: all
all: run_unittests

run_unittests: $(OBJS) ../src/pam_weblogin.a
	$(CC) -o $@ $^ $(LDFLAGS)

unittest: run_unittests
	./run_unittests

.PHONY: clean
clean:
	-rm -f $(OBJS) run_unittests core

