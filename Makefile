#
# $Id: $
#
# Description: Makefile for zhw
#
# $Log: $
#

PACKAGE = zhw-0.6.0

.PHONY: all
all:
	cd test/decode_test && $(MAKE) $@
	cd ../encode_test && $(MAKE) $@

.PHONY: dist
dist: distclean
	tar --transform 's,^,$(PACKAGE)/,' -czf ../$(PACKAGE).tgz --exclude-vcs *

.PHONY: distclean
distclean:
	$(RM) ../$(PACKAGE).tgz
	cd test/decode_test && $(MAKE) clean
	cd ../encode_test && $(MAKE) clean
