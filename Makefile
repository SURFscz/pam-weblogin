SUBDIRS := json-parser src

all: json-parser src/pam-websso.so

pam-websso: src/pam-websso.so

# Make PAM-WEBSSO module
src/pam-websso.so: json-parser src/Makefile
	$(MAKE) -C src/
src/Makefile: src/Makefile.am src/configure.ac
	cd src && ./autogen.sh && ./configure

# Make & Install json-parser package
.PHONY: json-parser
json-parser: json-parser/Makefile
	$(MAKE) -C json-parser/
	sudo  $(MAKE) -C json-parser/ install-static
json-parser/Makefile: json-parser/configure
	cd json-parser && ./configure
json-parser/configure:
	git submodule init
	git submodule update

# Cleanup...
.PHONY: clean
clean: clean-module clean-json-parser
clean-json-parser:
	@-$(MAKE) -C json-parser clean
clean-module:
	@-$(MAKE) -C src distclean

# Install...
.PHONY: install
install: src/pam-websso.so
	$(MAKE) -C src install

# Test...
.PHONY: test
test:
	$(MAKE) -C src test
