SUBDIRS := json-parser src

all: module
	$(MAKE) -C $@

.PHONY: module
module: json-parser
	$(MAKE) -C src/

.PHONY: json-parser
json-parser: json-parser/libjsonparser.a
json-parser/libjsonparser.a: json-parser/Makefile
	$(MAKE) -C json-parser/
json-parser/Makefile: json-parser/configure
	cd json-parser && ./configure
json-parser/configure:
	git submodule init
	git submodule update

.PHONY: clean clean-src clean-json-parser
clean: clean-src clean-json-parser
clean-json-parser: json-parser/Makefile
clean-json-parser clean-src: clean-%:
	$(MAKE) -C $* clean

.PHONY: install
install: module
	$(MAKE) -C src install

.PHONY: test
test: module
	$(MAKE) -C src test
