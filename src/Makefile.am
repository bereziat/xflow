# Makefile for generate xflow documentation
# $Id: Makefile.am,v 1.3 2007/09/24 11:43:34 bereziat Exp $
# (c) 2002-2006 D.Bereziat
SUBDIRS    = inrimage lib utils xflow img 
EXTRA_DIST = xflow.doxy doxydoc.c

nobase_include_HEADERS = inrimage/xflow.h

if BUILD_DOX
doc: xflow.doxy doxydoc.c img/main-window.png; doxygen xflow.doxy
img/main-window.png: img/main-window.fig; fig2dev -L png $< $@

# target reached by 'make install'
install-data-local: doc
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(docdir) 
	for file in $(srcdir)/doc/* ; do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(docdir) ;\
	done
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/examples/sh
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/examples/c
	for file in $(srcdir)/examples/sh/* ; do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(pkgdatadir)/examples/sh ;\
	done
	for file in $(srcdir)/examples/c/* ; do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(pkgdatadir)/examples/c ;\
	done 
	cd $(DESTDIR)$(pkgdatadir) && ln -sf $(docdir) doc
endif

# target reached by 'make dist'
dist-hook:
	mkdir -p $(distdir)/examples/c
	mkdir -p $(distdir)/examples/sh
	for file in examples/c/*.*; do \
	  cp -p $$file $(distdir)/examples/c; \
	done ;\
	for file in examples/sh/*.*; do \
	  cp -p $$file $(distdir)/examples/sh; \
	done 

clean-local:
	$(RM) -r doc img/main-window.png

.PHONY: doc
