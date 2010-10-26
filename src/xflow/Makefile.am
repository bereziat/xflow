# Makefile for compile xflow displayer.
# (c) 2002-2006 D.Béréziat
# $Id: Makefile.am,v 1.7 2009/06/02 20:34:30 bereziat Exp $

# @STUFF@ are autoconf macros: they are evaluated by configure
#         to produce Makefile.in
INCLUDES = -I.. @PACKAGE_CFLAGS@ \
	   -DPACKAGE_DATA_DIR=\""$(datadir)"\"

bin_PROGRAMS = xflow
xflow_SOURCES = interface.c interface.h support.c support.h \
	        callbacks.h xflow.glade main.c cb_mainwindow.c \
                cb_prefs.c cb_export.c cb_about.c data.c data.h \
	        lecscrn.c lecscrn.h utils.c utils.h api.c api.h
#		trajs.c (pas encore)


xflow_LDADD = @PACKAGE_LIBS@ -lxflow
xflow_LDFLAGS = -L../lib/ 

# installer les pixmaps dans $prefix/share/PACKAGE/pixmaps
nobase_pkgdata_DATA = pixmaps/arrow0.png pixmaps/arrow1.png pixmaps/arrow2.png \
		      pixmaps/arrow3.png pixmaps/arrow4.png pixmaps/arrow5.png pixmaps/arrow6.png
EXTRA_DIST = arrow.sh

pixmaps:; mkdir $@
$(nobase_pkgdata_DATA): pixmaps arrow.sh
	cd pixmaps && ../arrow.sh

if GLADE
# genere uniquement les fichiers sources GTK a partir du fichier glade
# (ne sert que pour le developpeur)
interface.c interface.h support.c callbacks.h: xflow.glade
	mkdir -p tempo_glade && cp xflow.glade tempo_glade
	if test -d tempo_glade ; then \
	  cd tempo_glade; \
	  glade-2 --write-source $< ; \
	  cp src/interface.* src/support.* src/callbacks.* .. ; \
	fi
	rm -rf tempo_glade

glade:
	glade-2 xflow.glade
endif

clean-local:
	$(RM) -r pixmaps

