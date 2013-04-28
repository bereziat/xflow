/**
 * Fonctions pour l'API xflow
 */
#include <config.h>  /* Pour la macro PACKAGE */
#include <stdlib.h>
#include <math.h>
#include "data.h"
#include "utils.h"
#include "api.h"

/**
 * Initialise une api
 */

XFLOW_API *xflow_api_new( void) {
  XFLOW_API *api = NEW( XFLOW_API, 1);

  api -> wimg = api -> himg = 0;  
  api -> zmax = 0;

  api -> data = NULL;
  api -> zpos = 0;
  api -> gc = NULL;
  api -> scale = 1.;
  api -> thresh = 0.;
  api -> thresh_high = 1000.;
  api -> sample = 4;
  
  api -> mainwindow = NULL;
  api -> about = NULL;
  api -> export = NULL;

  api -> background = NULL;
  api -> active = NULL;
  api -> trajs = NULL;
  api -> zoom = 1;
   
  return api;
}

void xflow_api_delete( XFLOW_API *api) {
  if( api->data) data_free(api);
  if( api->mainwindow) 
    g_object_unref (api->mainwindow);
  DELETE(api);
}

void xflow_api_set_global( XFLOW_API *api, int mode, float val) {
  switch( mode) {
  case API_SAMPLE: api->sample = val; break;
  case API_SCALE: api->scale = val; break;
  case API_TLOW: api->thresh = val; break;
  case API_THIGH: api->thresh_high = val; break;
  }
}

void xflow_api_show_window( XFLOW_API *api) {
  XFLOW_DATA *data;
  char name[256];
  GtkWidget *window;
  GError *error = NULL;

  api->mainwindow = gtk_builder_new();
  
  if( gtk_builder_add_from_file (api->mainwindow, "xflow.glade", NULL) == 0 &&
      gtk_builder_add_from_file (api->mainwindow, 
				 PACKAGE_DATA_DIR "/" PACKAGE "/xflow.glade", 
				 &error) == 0 ) {
    g_warning("gtk_builder, fatal error num %d:\n** => %s\n", error->code, error->message);
    exit(error->code);
  }
  gtk_builder_connect_signals (api->mainwindow, api);
  window = GTK_WIDGET( gtk_builder_get_object (api->mainwindow, "xflow_main"));
  
  /* main window title: the first available velocity filename or the first 
     image otherwise */
  for( data = api->data; data; data=data->next) {
    if( data -> type == DATA_IMAGE)
      sprintf( name, "%s", data->data.image.file->nom);
    if( data -> type == DATA_XFLOW) {
      sprintf( name, "%s", data->data.xflow.file->iuv->nom);
      break;
    }
  }
  gtk_window_set_title( GTK_WINDOW(window), name);  
  gtk_widget_show( GTK_WIDGET(window));

  /* background image */
  for( data = api->data; data; data=data->next)
    if( data -> type == DATA_IMAGE) {
      api->background = data;
      break;
    }

  /* active field */
  for( data = api->data; data; data=data->next)
    if( data -> type == DATA_XFLOW) {
      api->active = data;
      break;
    }
  
  /* Build the menu Select */
  GtkWidget *menu = lookup_widget( api->mainwindow, "xflow_main_menu_select");
  GSList *group = NULL;
  for( data=api->data; data; data=data->next) {
    if( data->type == DATA_IMAGE) {
      char *p = g_path_get_basename(data->data.image.file->nom);
      GtkWidget *menusubitem = gtk_radio_menu_item_new_with_label( group, p); 
      gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menusubitem), group?FALSE:TRUE);
      group =  gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menusubitem));
      gtk_widget_show( menusubitem);
      gtk_container_add( GTK_CONTAINER(menu), menusubitem);
      g_signal_connect ((gpointer) menusubitem, "activate",
			G_CALLBACK (on_xflow_main_menu_activebg_activate),
			api);
    }
  }

  if(api->active) {
    int first = 1;
    if( group) {
      GtkWidget *sep = gtk_separator_menu_item_new();
      gtk_widget_show( sep);
      gtk_container_add( GTK_CONTAINER(menu), sep);      
    }
    group = NULL;
    for( data = api->data; data; data=data->next) {
      if( data->type == DATA_XFLOW) {
	char *p = g_path_get_basename(data->data.xflow.file->iuv->nom);
	GtkWidget *menusubitem = gtk_radio_menu_item_new_with_label( group, p);
	group =  gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menusubitem));
	gtk_widget_show( menusubitem);
	gtk_container_add( GTK_CONTAINER(menu), menusubitem);
	g_signal_connect ((gpointer) menusubitem, "activate",
			  G_CALLBACK (on_xflow_main_menu_activefield_activate),
			  api);
	if( first) {
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menusubitem), TRUE);
	  first = 0;
	} else
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menusubitem), FALSE);
      }	
    }
  }


  /* list of vector fields in the Vectors Notebook */
  for( data=api->data; data; data=data->next) {
    if( data->type == DATA_XFLOW) {
      char *g = g_path_get_basename(data->data.xflow.file->iuv->nom);
      GtkWidget *but = gtk_check_button_new_with_label( g);
      gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(but), TRUE);

      GdkColor color;
      gdk_color_parse ( color_name(data->data.xflow.arrowcolor), &color);
      gtk_widget_modify_fg (GTK_WIDGET(but), GTK_STATE_NORMAL, &color);
      
      gtk_widget_show( but);
      gtk_container_add( GTK_CONTAINER(lookup_widget(api->mainwindow, "xflow_main_vectors_checks")),
			 but);
      g_signal_connect ((gpointer) but, "toggled",
			G_CALLBACK (on_xflow_main_vectors_checks_toggled), api);
      data->data.xflow.check = but;
    }
  }

  /* Paned positions */
  {
    GtkWidget *paned = lookup_widget( api->mainwindow, "xflow_main_vectors_paned");
    GtkAllocation alloc;
    
    gtk_widget_get_allocation (paned, &alloc);
    gtk_paned_set_position( GTK_PANED(paned), 0);
    api->paned = 0;

    /* Ne fonctionne pas
    paned = gtk_paned_get_child1(GTK_PANED(paned));
    g_object_set( paned, "shrink", FALSE);
    g_object_set( paned, "resize", FALSE);
    */

  }


  /* Création d'un contexte graphique pour dessiner dans les drawing areas */
  {
    GdkColor red = { 0, 1000, 0, 0 };
    GtkWidget* widget = lookup_widget( api->mainwindow, "xflow_main_vectors_draw");
    api -> gc = gdk_gc_new( widget->window);
    gdk_gc_set_rgb_fg_color (api->gc, &red);
  }
  /* Initialisation des couleurs Gdk */
  color_init( api);
  /* Init tableaux statiques internes */
  utils_init();

  utils_mesag( api, "sequence with %d frames", api->zmax);
}

/*
 * met a jour les widgets selon les paramètres de l'API.
 */

void xflow_api_update_widget( XFLOW_API *api) {
  GtkWidget *widget;
  GtkAdjustment *adj;
  float digit, normsup;
  XFLOW_DATA *pd;
  Fort_int lfmt[9];

  normsup = 0;
  for( pd=api->data; pd; pd=pd->next)
    if( pd->type == DATA_XFLOW && normsup < pd->data.xflow.normsup) {
      xflow_get_lfmt( pd->data.xflow.file, lfmt);
      normsup = MIN(pd->data.xflow.normsup, MAX(NDIMX,NDIMY));
    }

  /* sample */
  adj = gtk_range_get_adjustment( GTK_RANGE(lookup_widget(api->mainwindow,"xflow_main_sample")));
  adj -> value = api->sample;
  gtk_adjustment_changed( adj);

  /* scale */
  adj = gtk_range_get_adjustment( GTK_RANGE(lookup_widget(api->mainwindow,"xflow_main_scale")));
  adj -> value = api->scale;
  gtk_adjustment_changed( adj);

  /* seuillage bas */
  widget = lookup_widget( api->mainwindow, "xflow_main_thresh");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));

  adj -> upper = normsup+0.01; 
  adj -> lower = 0;
  adj -> step_increment = normsup / 100;
  adj -> page_increment = normsup / 10;
  adj -> value = api->thresh;
  if( adj->value < 0) adj->value = adj->lower;

  gtk_scale_set_digits (GTK_SCALE (widget), 3);
  gtk_adjustment_changed( adj);

  /* seuillage haut */
  widget = lookup_widget( api->mainwindow, "xflow_main_thresh_high");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));

  adj -> upper = normsup+0.01;
  adj -> lower = 0;
  adj -> step_increment = normsup / 100;
  adj -> page_increment = normsup / 10;
  adj -> value = api->thresh_high;
  if( adj->value > adj->upper ) adj->value = adj->upper;

  // printf( "%f\n", digit = log(api->normsup)/log(2.));
  gtk_scale_set_digits (GTK_SCALE (widget), 3);
  gtk_adjustment_changed( adj);

  /* zoom */  
  widget = lookup_widget( api->mainwindow, "xflow_main_zoom");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  adj -> value = api->zoom;
  gtk_adjustment_value_changed( adj);

  /* HSV saturation */
  if( api->active) {
    widget = lookup_widget( api->mainwindow, "xflow_main_hsv_saturation");
    adj = gtk_range_get_adjustment( GTK_RANGE(widget));
    adj->value = api->active->data.xflow.magmax;
    adj->upper = adj->value * 3;
    adj->lower = adj->value / 20;
    adj -> step_increment = (adj->upper - adj->lower) / 100;
    adj -> page_increment = (adj->upper - adj->lower) / 10;
  
    gtk_adjustment_value_changed( adj);
  }
  
  /* scroller vertical des plans */
  widget = lookup_widget( api->mainwindow, "xflow_main_zscroll");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  //  adj -> upper = api->zmax;
  //  gtk_adjustment_changed( adj);
  gtk_adjustment_set_upper (adj,api->zmax);
}
  


void xflow_api_refresh_drawing_areas( XFLOW_API *api) {
  XFLOW_DATA *pd;

  for( pd = api->data; pd; pd = pd->next) {
    if( pd->type == DATA_XFLOW) {
      GdkColor color;
      gdk_color_parse ( color_name(pd->data.xflow.arrowcolor), &color);
      gtk_widget_modify_fg (GTK_WIDGET(pd->data.xflow.check), GTK_STATE_NORMAL, &color); 
    }
  }
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_thresh"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_thresh_high"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_vectors_draw"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_mag_draw"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_mag_legend"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_div_draw"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_div_legend"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_curl_draw"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_curl_legend"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_main_hsv_draw"));
}
