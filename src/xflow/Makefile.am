# Makefile for compile xflow displayer.
# (c) 2002-2006 D.Béréziat
# $Id: Makefile.am,v 1.7 2009/06/02 20:34:30 bereziat Exp $

# @STUFF@ are autoconf macros: they are evaluated by configure
#         to produce Makefile.in
AM_CPPFLAGS = -I.. @PACKAGE_CFLAGS@ \
	      -DPACKAGE_DATA_DIR=\""$(datadir)"\"

bin_PROGRAMS = xflow
xflow_SOURCES = xflow.glade main.c cb_mainwindow.c lecscrn.c lecscrn.h \
		data.c data.h utils.c utils.h api.c api.h \
		middlebury.c middlebury.h cb_export.c
#                cb_prefs.c cb_export.c cb_about.c 
#		trajs.c (pas encore)

xflow_CFLAGS = -g
xflow_LDADD = @PACKAGE_LIBS@ ../lib/libxflow.a
xflow_LDFLAGS = 

# installer les pixmaps dans $prefix/share/PACKAGE/pixmaps
nobase_pkgdata_DATA = pixmaps/arrow0.png pixmaps/arrow1.png pixmaps/arrow2.png \
		      pixmaps/arrow3.png pixmaps/arrow4.png pixmaps/arrow5.png pixmaps/arrow6.png
EXTRA_DIST = arrow.sh

pixmaps:; mkdir $@
$(nobase_pkgdata_DATA): pixmaps arrow.sh
	cd pixmaps && ../arrow.sh

glade:
	glade-3 xflow.glade


clean-local:
	$(RM) -r pixmaps

