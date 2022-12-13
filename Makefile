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
	git submodule init
	git submodule update

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
