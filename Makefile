SUBDIRS := json-parser src

all: module

.PHONY: module
module: json-parser
	$(MAKE) -C src/

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
clean-json-parser: json-parser-build
	rm -rf json-parser-build/
clean-src: clean-%:
	$(MAKE) -C $* clean

.PHONY: install
install: module
	$(MAKE) -C src install

.PHONY: test
test: module
	$(MAKE) -C src test
