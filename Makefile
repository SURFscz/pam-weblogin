SUBDIRS := src

.PHONY: all $(SUBDIRS)
all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

CLEANSUBDIRS = $(addprefix clean-,$(SUBDIRS))

.PHONY: clean $(CLEANSUBDIRS)
clean: $(CLEANSUBDIRS)
$(CLEANSUBDIRS): clean-%:
	$(MAKE) -C $* clean



