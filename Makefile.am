# Makefile for compile xflow and its utilities
# $Id: Makefile.am,v 1.5 2010/07/21 12:41:57 bereziat Exp $
# (c) 2002-2006 D.Bereziat

SUBDIRS    = src scripts
EXTRA_DIST = debian/changelog  debian/compat debian/control  \
             debian/copyright  debian/dirs debian/docs  \
             debian/rules

pkgdata_DATA = AUTHORS README TODO NEWS COPYING INSTALL ChangeLog

# target reached by 'make install'
install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) NEWS $(DESTDIR)$(docdir)
	$(INSTALL_DATA) ChangeLog $(DESTDIR)$(docdir)

# target reached by 'make dist'
dist-hook:


