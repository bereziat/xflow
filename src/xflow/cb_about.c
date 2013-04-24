#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdlib.h>
#include "data.h"
#include "utils.h"

void
on_about_activate                     (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  GError *error;
  GtkWidget *widget, *entry;
  char text[256];

  if( api->about) {
    gtk_window_present(GTK_WINDOW(lookup_widget(api->about, "about")));
    return;
  }

  api->about = gtk_builder_new();
  if( gtk_builder_add_from_file (api->about, "about.glade", NULL) == 0 &&
      gtk_builder_add_from_file (api->about,
				 PACKAGE_DATA_DIR "/" PACKAGE "/about.glade", 
				 &error) == 0 ) {
    printf("gtk_builder:fatal error:%d:%s\n", error->code, error->message);
    return;
  }
    
  gtk_builder_connect_signals (api->about, api);
  widget = lookup_widget( api->about, "about");

  entry = lookup_widget( api->about, "about_xflow");
  sprintf( text, "%d.%d.%d %s", XFLOW_API_MAJOR, XFLOW_API_MINOR, 
	   XFLOW_API_RELEASE, XFLOW_API_BETA?"beta":"");
  gtk_entry_set_text( GTK_ENTRY(entry), text);
  
  entry = lookup_widget( api->about, "about_libxflow");
  sprintf( text, "%d.%d.%d", XFLOW_MAJOR, XFLOW_MINOR, 
	   XFLOW_RELEASE);
  gtk_entry_set_text( GTK_ENTRY(entry), text);
  

  entry = lookup_widget( api->about, "about_doc");
  sprintf( text, "%s/%s/doc/index.html", PACKAGE_DATA_DIR, PACKAGE);
  gtk_link_button_set_uri ( GTK_LINK_BUTTON(entry), text);

  gtk_widget_show( widget);
}


void
on_about_close_clicked               (GtkButton       *button,
				      gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*)user_data;
  gtk_widget_destroy( lookup_widget( api->about, "about"));  
  g_object_unref( api->about);
  api->about = FALSE; 
}


void
on_about_doc_clicked               (GtkButton       *button,
				    gpointer         user_data)
{
  char cmd[256];
  char *p = getenv("BROWSER");
 
  sprintf( cmd, "url=%s/%s/doc/index.html; if which xdg-open; then xdg-open $url; else open $url; fi &", PACKAGE_DATA_DIR, PACKAGE);
  // puts( cmd);
  system( cmd);
}


