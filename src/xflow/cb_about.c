#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>
#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "data.h"

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget, *label;
  char text[256];
  

  widget = create_xflow_info();
  label = lookup_widget( widget, "xflow_info_label");
  sprintf( text,
	   "XFLOW\nversion %d.%d.%d %s\n\n compiled with libxflow %d.%d.%d \n(c) 2004-2010 Dominique Bereziat",
	   XFLOW_API_MAJOR, XFLOW_API_MINOR, XFLOW_API_RELEASE, XFLOW_API_BETA?"beta":"",
	   XFLOW_MAJOR, XFLOW_MINOR, XFLOW_RELEASE
	   );
  
  gtk_label_set_text( GTK_LABEL(label), text);
  gtk_widget_show( widget);
}


void
on_xflow_info_ok_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(button), "xflow_info");
  gtk_widget_destroy( widget);
}

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  char cmd[256];
  char *p = getenv("BROWSER");
  
  sprintf( cmd, "%s %s/%s/doc/index.html &", p?p:"mozilla", PACKAGE_DATA_DIR, PACKAGE);
  puts( cmd);
  system( cmd);
}

