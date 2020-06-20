#
# $Id: $
#
# Description: Makefile for zhw
#
# $Log: $
#

PACKAGE = zhw-0.5.0

.PHONY: all
all:
	cd test && $(MAKE) $@

.PHONY: dist
dist: distclean
	tar --transform 's,^,$(PACKAGE)/,' -czf ../$(PACKAGE).tgz --exclude-vcs *

.PHONY: distclean
distclean:
	$(RM) ../$(PACKAGE).tgz
	cd test && $(MAKE) clean
