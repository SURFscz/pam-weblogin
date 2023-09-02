SUBDIRS := json-parser src

all: module
ifdef NOVERIFY
FLAGS := NOVERIFY=1
endif

.PHONY: module
module: json-parser
	$(MAKE) -C src/ $(FLAGS)

.PHONY: json-parser
json-parser: json-parser-build/libjsonparser.a
json-parser-build/libjsonparser.a: json-parser-build/Makefile
	$(MAKE) -C json-parser-build/
json-parser-build/Makefile: json-parser/configure
	mkdir -p json-parser-build
	cd json-parser-build && ../json-parser/configure
json-parser/configure:
	git clone https://github.com/json-parser/json-parser.git json-parser/
	git -c advice.detachedHead=false -C json-parser/ checkout 531a49062975d6d2cd5d69b75ad5481a8c0e18c5

.PHONY: clean clean-src clean-json-parser
clean: clean-src clean-json-parser
clean-json-parser:
	rm -rf json-parser-build/
clean-src: clean-%:
	$(MAKE) -C $* clean

.PHONY: install
install: module
	$(MAKE) -C src install

.PHONY: test
test: module
	$(MAKE) -C src test

.PHONY: unittest
unittest: module
	$(MAKE) -C tests unittest
