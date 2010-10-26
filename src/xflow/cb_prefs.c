#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <math.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "data.h"
#include "utils.h"

extern XFLOW_API api;

/* Xflow buffer selected in Preference formular */
static XFLOW_DATA *cdata = NULL;


void 
on_prefs_menufile_item ( GtkMenuItem     *menuitem,
			 XFLOW_DATA      *pd)
{
  
    GtkWidget *widget;
    char text[256];
    Fort_int lfmt[9];
    int M,m,r;

    cdata = pd;

    /* Dimensions */
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_dims");
    xflow_get_lfmt( pd->data.xflow.file, lfmt);
    sprintf( text, "-x %d -y %d -z %d -v %d -r -o %d", 
	     NDIMX, NDIMY, NDIMZ, NDIMV, BSIZE);
    gtk_entry_set_text( GTK_ENTRY(widget), text);

    /* Image */
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_image");
    gtk_entry_set_text( GTK_ENTRY(widget), pd->data.xflow.file->i_name);

    /* Version */
    xflow_version( pd->data.xflow.file, &M, &m, &r);
    sprintf( text, "%d.%d.%d", M, m, r);
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_version");
    gtk_entry_set_text( GTK_ENTRY(widget), text);

    /* Hidden */
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_hidden");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), 
				  pd->data.xflow.hide?TRUE:FALSE);

    /* Norma */
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_norma");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), 
				  pd->data.xflow.norma?TRUE:FALSE);
    
    
    /* Arrow */
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_optionsizearrow"); 
    gtk_option_menu_set_history( GTK_OPTION_MENU(widget), pd->data.xflow.arrowsize+1);
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_optioncolorarrow"); 
    gtk_option_menu_set_history( GTK_OPTION_MENU(widget), pd->data.xflow.arrowcolor+1);
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_optionwidtharrow"); 
    gtk_option_menu_set_history( GTK_OPTION_MENU(widget), pd->data.xflow.arrowwidth-1);
    widget = lookup_widget( GTK_WIDGET(menuitem), "prefs_optionstylearrow"); 
    gtk_option_menu_set_history( GTK_OPTION_MENU(widget), pd->data.xflow.arrowstyle);
}

void 
on_prefs_menubg_item ( GtkMenuItem     *menuitem,
		       XFLOW_DATA      *pd)
{
  /* On place l'élément pd en première position car c'est
   * le premier élement image qui est affiché
   */
  XFLOW_DATA *prev;

  /* cas ou l'on sélectionne l'image active */
  if( pd == api.data) return;
  
  /* recherche du précédent de pd */
  for( prev=api.data; prev->next != pd; prev=prev->next);
  /* connection prev,next */
  prev->next = pd->next;
  /* placement de pd en première position */
  pd->next = api.data;
  api.data = pd;

  xflow_api_refresh_drawing_areas (&api);
}


void
on_clean_header_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  XFLOW_DATA *pd;

  for( pd = api.data; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {
      char buf[32];
      idelhline( pd->data.xflow.file->iuv, "XFLOW_PARAMS=", 1);
    }
  }
}

void
on_write_in_header_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  XFLOW_DATA *pd;

  for( pd = api.data; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {
      char buf[32];
      int a;
      idelhline( pd->data.xflow.file->iuv, "XFLOW_PARAMS=", 1);
      sprintf( buf, "%d %f %f %f", api.sample, api.scale,
	       api.thresh,api.thresh_high);
      a = iputhline( pd->data.xflow.file->iuv, "XFLOW_PARAMS=", buf);
      fprintf( stderr, "ecriture header : %d\n", a);
    }
  }
}



static int prefs = FALSE;

void
on_winprefs_destroy                    (GtkObject       *object,
                                        gpointer         user_data)
{
  prefs = FALSE;
}

void
on_prefs_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *pref_menufile, *item, *pref_menubg, *prefs_root;
  XFLOW_DATA *pd;
  static int first_xflow = 0;
  static int first_data = 0;
  
  if( prefs == TRUE) return;
  prefs = TRUE;
  
  prefs_root = create_winprefs();
  pref_menufile = lookup_widget( prefs_root, "prefs_menufile");
  pref_menubg = lookup_widget( prefs_root, "prefs_menubackground");
   
  for( pd = api.data; pd; pd=pd->next) {
    /* Ajout des entrées xflow au menu pref_menufile */
    if( pd->type == DATA_XFLOW) {
      item = gtk_menu_item_new_with_mnemonic (_(pd->data.xflow.file->iuv->nom));
      gtk_widget_show (item);
      gtk_container_add (GTK_CONTAINER (pref_menufile), item);
      g_signal_connect ((gpointer) item, "activate",
			G_CALLBACK (on_prefs_menufile_item),
			pd);
      
      /* Sélectionner le premier élément du menu */
      if( !first_xflow) {
	gtk_menu_item_activate( GTK_MENU_ITEM(item));
	gtk_option_menu_set_history( GTK_OPTION_MENU(lookup_widget( prefs_root, "prefs_optionfile")), 1);
	cdata = pd;
	first_xflow = 1;
	/* Configuration des menus aspect champ */
	gtk_option_menu_set_history( GTK_OPTION_MENU(lookup_widget( prefs_root, "prefs_optioncolorarrow")), 
				     pd->data.xflow.arrowcolor + 1);
	gtk_option_menu_set_history( GTK_OPTION_MENU(lookup_widget( prefs_root, "prefs_optionsizearrow")), 
				     pd->data.xflow.arrowsize + 1);
	
      }

    } else 
      /* Ajout des entrées image au menu pref_menubg */
      if( pd->type == DATA_IMAGE) {
	item = gtk_menu_item_new_with_mnemonic (_(pd->data.image.file->nom));
	gtk_widget_show (item);
	gtk_container_add (GTK_CONTAINER (pref_menubg), item);
	g_signal_connect ((gpointer) item, "activate",
			  G_CALLBACK (on_prefs_menubg_item),
			  pd);
	
	/* Sélectionner le premier élément du menu */
	if( !first_data) {
	  gtk_menu_item_activate( GTK_MENU_ITEM(item));
	  gtk_option_menu_set_history( GTK_OPTION_MENU(lookup_widget( prefs_root, "prefs_optionbackground")), 1);
	  first_data = 1;
	}	  
      }    
  } /* for */

  gtk_widget_show( prefs_root);
}



void
on_prefs_black_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = BLACK;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_blue_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = BLUE;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_green_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = GREEN;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_cyan_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = CYAN;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_red_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = RED;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_magenta_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = MAGENTA;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_yellow_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = YELLOW;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_white_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowcolor = WHITE;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_default1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
 if( cdata) {
   cdata->data.xflow.arrowsize = -1;
   xflow_api_refresh_drawing_areas (&api);
 }
}


void
on_prefs_small_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
 if( cdata) {
   cdata->data.xflow.arrowsize = 0;
   xflow_api_refresh_drawing_areas (&api);
 }
}


void
on_prefs_normal_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowsize = 1;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_large_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowsize = 2;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_big_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if( cdata) {
    cdata->data.xflow.arrowsize = 3;
    xflow_api_refresh_drawing_areas (&api);
  }
}


void
on_prefs_close_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  cdata = NULL;
  gtk_widget_destroy( gtk_widget_get_toplevel(GTK_WIDGET(button)));
}


void
on_prefs_default_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  
}


void
on_prefs_hidden_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  if( cdata) {
    int active = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(button));
    if( active != cdata->data.xflow.hide ) {
      xflow_api_refresh_drawing_areas (&api);
      cdata->data.xflow.hide ^=1;
    }
  }
}


void
on_prefs_hidden_others_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  if( cdata) {
    XFLOW_DATA *pd;
    int active = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(button));

    for( pd = api.data; pd; pd=pd->next) {
      if( pd == cdata) continue;
      if( pd->type == DATA_XFLOW)
	pd->data.xflow.hide = active;
    }
    xflow_api_refresh_drawing_areas (&api);
  }
}



void
on_prefs_norma_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  int active = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(button));
  if( cdata) {
    cdata->data.xflow.norma = active>0;
    xflow_api_refresh_drawing_areas (&api);
  }
}

void
on_prefs_smooth_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  int active = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(button));
  if( cdata) {
    cdata->data.xflow.smooth = active>0;
    xflow_api_refresh_drawing_areas (&api);
  }
}


/*
 * V2 des preferences 
 */

enum {
  COLUMN_FILENAME,
  COLUMN_COLOR,
  COLUMN_SIZE,
  COLUMN_DISPLAY,
  COLUMN_SOLO,
  COLUMN_NORMALIZE,
  COLUMN_SMOOTHING,
  NUM_COLUMNS
};

char *combo_colors[] = { "default", "black", "blue", "green", "cyan", "red", 
			 "magenta", "yellow", "white", NULL  };
char *combo_sizes[] = { "default", "small", "normal", "large", "big", NULL };

static void
on_combo_colors_edited ( GtkCellRendererText *renderer,
			 const gchar *path_str,
			 const gchar *text,
			 gpointer user_data)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
  GtkTreeModel *model;
  GtkTreeIter iter;
  int row, color;
  XFLOW_DATA *pd;

  /* get selected value and modify the list_store */
  model = gtk_tree_view_get_model( tree_view);
  gtk_tree_model_get_iter_from_string( model, &iter, path_str);
  gtk_list_store_set( GTK_LIST_STORE(model), &iter, COLUMN_COLOR, text, -1);

  /* get index in the color table */
  for( color=0; combo_colors[color] != NULL; color++) 
    if( !strcmp( combo_colors[color], text))
      break;

  /* modify global data */
  for( pd = api.data, row=atoi(path_str); pd; pd=pd->next, row--)
    if( pd->type == DATA_XFLOW && row == 0) {
      pd->data.xflow.arrowcolor = color;
      xflow_api_refresh_drawing_areas (&api);
      break;
    }

  /* numero et valeur */
  printf ( "TRACE: %s %s\n",path_str,text);  
}

void
on_legendes_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{  
  XFLOW_DATA *pd;
  int i,n;

  GtkWidget *prefs_root = create_winprefs_V2();

  /* Créer la Tree View (pas géré dans Glade) */  
  GtkListStore *store;
  GtkTreeIter iter;
  GtkWidget *treeview;
  
  /* create our tree model */
  store = gtk_list_store_new ( NUM_COLUMNS, 
			       G_TYPE_STRING,
			       G_TYPE_STRING,
			       G_TYPE_STRING,
			       G_TYPE_BOOLEAN,
			       G_TYPE_BOOLEAN,
			       G_TYPE_BOOLEAN,
			       G_TYPE_BOOLEAN);
  
  /* adding xflow data to the list store */
  for( pd = api.data; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
			  COLUMN_FILENAME,  pd->data.xflow.file->iuv->nom,
			  COLUMN_COLOR,     combo_colors[pd->data.xflow.arrowcolor + 1],
			  COLUMN_SIZE,      combo_sizes[pd->data.xflow.arrowsize + 1],
			  COLUMN_DISPLAY,  !pd->data.xflow.hide,
			  COLUMN_SOLO,      FALSE,
			  COLUMN_NORMALIZE, pd->data.xflow.norma,
			  COLUMN_SMOOTHING, pd->data.xflow.smooth,
			  -1 );      
    }
  }

  /* create tree view */
  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  //  gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), COLUMN_DESCRIPTION);
  g_object_unref (GTK_TREE_MODEL(store));
  gtk_widget_show( treeview);

  /* ajout du tree view dans notre scrolled window */
  gtk_container_add (GTK_CONTAINER (lookup_widget( prefs_root, "sw_list_vf")), 
		     treeview);
  //  add_columns (GTK_TREE_VIEW (treeview));

  /* Finaly, adding columns */
  {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkListStore *combo_list;

    /* first column : a label indicating the vector field */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes( "Filename",
						       renderer,
						       "text",
						       COLUMN_FILENAME,
						       NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_FILENAME);
    gtk_tree_view_append_column (treeview, column);

    /* second column : choice of color using a combobox */
    renderer = gtk_cell_renderer_combo_new();
    combo_list = gtk_list_store_new (1, G_TYPE_STRING);
    for( i=0; combo_colors[i] != NULL; i++) {
      gtk_list_store_append ( combo_list, &iter);
      gtk_list_store_set ( combo_list, &iter, 0, combo_colors[i], -1);
    }
    g_object_set( renderer,  "model", combo_list, "text-column", 0,
		  "editable", TRUE, "has-entry", FALSE, 
		  "foreground", "red", // TODO
		  NULL);
    g_signal_connect (G_OBJECT (renderer), "edited",
		      G_CALLBACK (on_combo_colors_edited), treeview);
    g_object_unref (G_OBJECT (combo_list));

    column = gtk_tree_view_column_new_with_attributes( "Color", renderer, 
						       "text", COLUMN_COLOR, NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_COLOR);
    gtk_tree_view_append_column (treeview, column);


    /* third column : choice of size using a combobox */
    renderer = gtk_cell_renderer_combo_new();
    combo_list = gtk_list_store_new (1, G_TYPE_STRING);
    for( i=0; combo_sizes[i] != NULL; i++) {
      gtk_list_store_append ( combo_list, &iter);
      gtk_list_store_set ( combo_list, &iter, 0, combo_sizes[i], -1);
    }
    g_object_set( renderer,  "model", combo_list, "text-column", 0,
		  "editable", TRUE, "has-entry", FALSE, 
		  NULL);
    g_object_unref (G_OBJECT (combo_list));

    column = gtk_tree_view_column_new_with_attributes( "Size", renderer, 
						       "text", COLUMN_SIZE, NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_SIZE);
    gtk_tree_view_append_column (treeview, column);


    /* last columns */
    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes( "Show", renderer,
						       "active", COLUMN_DISPLAY, NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_DISPLAY);
    gtk_tree_view_append_column (treeview, column);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes( "Solo", renderer,
						       "active", COLUMN_SOLO, NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_SOLO);
    gtk_tree_view_append_column (treeview, column);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes( "Norma", renderer,
						       "active", COLUMN_NORMALIZE, NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_NORMALIZE);
    gtk_tree_view_append_column (treeview, column);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes( "Smooth", renderer,
						       "active", COLUMN_SMOOTHING, NULL);
    gtk_tree_view_column_set_sort_column_id (column, COLUMN_SMOOTHING);
    gtk_tree_view_append_column (treeview, column);
    
  }
  


  gtk_widget_show( prefs_root);
}

static void set_width( int val) {
  if( cdata) {
    cdata->data.xflow.arrowwidth = val;
    xflow_api_refresh_drawing_areas (&api);
  }
}

void
on_w1_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_width( 1);
}


void
on_prefs_w2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_width( 2);
}


void
on_prefs_w3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_width( 3);
}


void
on_prefs_w4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_width( 4);
}


void
on_prefs_w5_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_width( 5);
}



static void set_style( int val) {
  if( cdata) {
    cdata->data.xflow.arrowstyle = val;
    xflow_api_refresh_drawing_areas (&api);
  }
}

void
on_a0_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_style(0);
}


void
on_prefs_a1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_style(1);
}


void
on_prefs_a2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{  
  set_style(2);
}


void
on_prefs_a3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_style(3);
}


void
on_prefs_a4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_style(4);
}


void
on_a5_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_style(5);
}


void
on_a6_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  set_style(6);
}
