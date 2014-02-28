/**
 * Fonctions pour l'API xflow
 */
#include <config.h>  /* Uniquement pour la macro PACKAGE */
#include <stdlib.h>
#include <string.h>
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

extern int with_trajs;

void xflow_api_show_window( XFLOW_API *api) {
  XFLOW_DATA *data;
  char name[256];
  GtkWidget *window;
  GError *error = NULL;

  api->mainwindow = gtk_builder_new();
  
  gtk_builder_add_from_file (api->mainwindow, "xflow.glade", &error);
  if( error) {
    if( debug) /* Pour la mise au point */
      g_warning("DEBUG: gtk_builder, error num %d\nDEBUG: => %s\n", 
		error->code, error->message);
    
    error = NULL;
    gtk_builder_add_from_file (api->mainwindow, 
			       PACKAGE_DATA_DIR "/" PACKAGE "/xflow.glade", 
			       &error);
    if( error) {
      g_warning("gtk_builder, fatal error num %d\n** => %s\n", 
		error->code, error->message);
      exit(error->code);
    }
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


  if( with_trajs)
    gtk_widget_show( lookup_widget( api->mainwindow, "xflow_main_menu_trajs"));
  
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
  GtkAccelGroup *accel = gtk_accel_group_new();
  int iter = 0;
  for( data=api->data; data; data=data->next) {
    if( data->type == DATA_IMAGE) {
      char *p = /*g_path_get_basename*/(data->data.image.file->nom);
      GtkWidget *menusubitem = gtk_radio_menu_item_new_with_label( group, p); 
      gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menusubitem), group?FALSE:TRUE);
      group =  gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menusubitem));
      gtk_widget_show( menusubitem);
      gtk_container_add( GTK_CONTAINER(menu), menusubitem);
      g_signal_connect ((gpointer) menusubitem, "activate",
			G_CALLBACK (on_xflow_main_menu_activebg_activate),
			api);
      gtk_widget_add_accelerator ( menusubitem, "activate", accel, 
				   gdk_keyval_from_name ("1") + iter ++,
				   (GdkModifierType) GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    }
  }

  if(api->active) {
    iter = 0;
    int first = 1;
    if( group) {
      GtkWidget *sep = gtk_separator_menu_item_new();
      gtk_widget_show( sep);
      gtk_container_add( GTK_CONTAINER(menu), sep);      
    }
    group = NULL;
    for( data = api->data; data; data=data->next) {
      if( data->type == DATA_XFLOW) {
	char *p = /* g_path_get_basename*/ (data->data.xflow.file->iuv->nom);
	GtkWidget *menusubitem = gtk_radio_menu_item_new_with_label( group, p);
	group =  gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menusubitem));
	gtk_widget_show( menusubitem);
	gtk_container_add( GTK_CONTAINER(menu), menusubitem);
	g_signal_connect ((gpointer) menusubitem, "activate",
			  G_CALLBACK (on_xflow_main_menu_activefield_activate),
			  api);
	gtk_widget_add_accelerator ( menusubitem, "activate", accel, 
				     gdk_keyval_from_name ("1") + iter ++,
				     (GdkModifierType) 0, GTK_ACCEL_VISIBLE);
	if( first) {
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menusubitem), TRUE);
	  first = 0;
	} else
	  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(menusubitem), FALSE);
      }	
    }
  }
  gtk_window_add_accel_group( GTK_WINDOW(window), accel);

  /* list of vector fields in the Vectors Notebook left pane */
  for( data=api->data; data; data=data->next) {
    if( data->type == DATA_XFLOW) {
      char *g = /*g_path_get_basename*/(data->data.xflow.file->iuv->nom);
      GtkWidget *but = gtk_check_button_new_with_label( g);
      gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(but), TRUE);

      GdkColor color;
      gdk_color_parse ( color_name(data->data.xflow.arrowcolor), &color);
      gtk_widget_modify_fg (GTK_WIDGET(but), GTK_STATE_NORMAL, &color);
      
      gtk_widget_show( but);
      gtk_container_add( GTK_CONTAINER(lookup_widget(api->mainwindow, 
						     "xflow_main_vectors_checks")),
			 but);
      g_signal_connect ((gpointer) but, "toggled",
			G_CALLBACK (on_xflow_main_vectors_checks_toggled), api);
      data->data.xflow.check = but;
    }
  }
  
  /* list of trajectories */
  if( with_trajs && api->active) {
    GtkWidget *treeview;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;
    
    /* model */
    store = gtk_list_store_new( 4,     /* four columns */
				G_TYPE_BOOLEAN,
				G_TYPE_BOOLEAN,
				G_TYPE_STRING,
				G_TYPE_STRING);
    
    treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store));
    

    /* 'Delete' button */
    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes( "Delete", renderer, "active", 0, NULL);
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (on_xflow_main_trajs_delete_toggled), api);    
    /* set this column to a fixed sizing (of 50 pixels) */
    //    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
    //				     GTK_TREE_VIEW_COLUMN_FIXED);
    //    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
    gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
    
    /* 'Hide' button */
    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes( "Hide", renderer, "active", 1, NULL);
    g_signal_connect( renderer, "toggled",
		      G_CALLBACK(on_xflow_main_trajs_hide_toggled), api);
    gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

    /* Coordinate initial point */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes( "(x0,y0,z0)", renderer, "text", 2, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
    
    /* Coordinate final point */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes( "(xn,yn,zn)", renderer, "text", 3, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
    
    
    gtk_widget_show( treeview);
    gtk_container_add( GTK_CONTAINER(lookup_widget(api->mainwindow, 
						   "xflow_main_vectors_paned_box")),
		       treeview);
    
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
    GtkWidget* widget = lookup_widget( api->mainwindow, "xflow_main"); //_vectors_draw");
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

/* Mise a jour du menu Vectors */
void xflow_api_update_menu( XFLOW_API *api) {
  if( api->active) {
    GtkWidget *widget;
    char lab[10]=" ";
    void on_xflow_main_menu_style_activate( GtkMenuItem *, gpointer);

    widget = lookup_widget( api->mainwindow, color_name( api->active->data.xflow.arrowcolor));
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE);

    widget = lookup_widget( api->mainwindow, size_name( api->active->data.xflow.arrowsize));
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE);
    
    *lab = '0' + api->active->data.xflow.arrowwidth;
    widget = lookup_widget( api->mainwindow, lab);
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE);

    strcpy( lab, "style ");
    lab[5] = '0' + api->active->data.xflow.arrowstyle;
    widget = lookup_widget( api->mainwindow, lab);
    on_xflow_main_menu_style_activate( GTK_MENU_ITEM(widget), api);
  }
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

void xflow_api_set_title( XFLOW_API *api) {
  GtkWidget *win = lookup_widget( api->mainwindow, "xflow_main");
  char title[256];
  if( api->active) {
    char *n = g_path_get_basename(api->active->data.xflow.file->iuv->nom);
    
    sprintf( title, "%s", n);
    if( api->background) {
      n = g_path_get_basename(api->background->data.image.file->nom);
      sprintf( title, "%s (%s)", title, n);
    } 
  } else if ( api->background) {
    char *n = g_path_get_basename(api->background->data.image.file->nom);
    sprintf( title, "%s", n);
  } else 
    strcpy( title, "no name");
  gtk_window_set_title( GTK_WINDOW(win), title);
}
