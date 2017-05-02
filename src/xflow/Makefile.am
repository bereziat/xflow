# Makefile for compile xflow displayer.
# (c) 2013 D.Béréziat

AM_CPPFLAGS = -I.. @GTK2_CFLAGS@ \
	      -DPACKAGE_DATA_DIR=\""$(datadir)"\"

bin_PROGRAMS = xflow
xflow_SOURCES = main.c api.c api.h \
		cb_export.c cb_about.c cb_mainwindow.c \
		data.c data.h utils.c utils.h \
		middlebury.c middlebury.h  \
		lecscrn.c lecscrn.h \
		trajs.c trajs.h

xflow_CFLAGS = -O2
xflow_LDADD = @GTK2_LIBS@ ../lib/libxflow.a
if LINUX
# ce flag permet à gmodule de lire la table des symboles de l'application
# et  d'associer les callbacks aux bonnes routines (gtk_builder_connect_signals()).
# Déjà activé par défaut sur MacOS, ce n'est pas le cas sur Linux.
xflow_LDFLAGS = -Wl,--export-dynamic
endif

# installer les pixmaps dans $prefix/share/PACKAGE/pixmaps
nobase_pkgdata_DATA = pixmaps/arrow0.png pixmaps/arrow1.png pixmaps/arrow2.png \
		      pixmaps/arrow3.png pixmaps/arrow4.png pixmaps/arrow5.png \
		      pixmaps/arrow6.png pixmaps/middlebury.png

pkgdata_DATA = xflow.glade export.glade about.glade

EXTRA_DIST = arrow.sh makewheel.c xflow.glade export.glade about.glade

$(nobase_pkgdata_DATA): arrow.sh makewheel
	mkdir -p pixmaps
	cd pixmaps && ../arrow.sh
	./makewheel -x 120 -y 120 | ../utils/vel2col | inr2png > pixmaps/middlebury.png

makewheel: makewheel.c
	$(CC) -o $@ $^ `inrinfo --cflags --libs`

glade:
	glade-3 xflow.glade export.glade about.glade


clean-local:
	$(RM) -r pixmaps makewheel


