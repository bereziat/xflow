#include <config.h>
#include <gtk/gtk.h>
#include <locale.h>
#include "data.h"
#include "utils.h"
#include "api.h"

int debug = 0;
int with_trajs = 0;
float DT = 1.0;

extern int point_factor, line_width; /* provisoire, dans trajs.c */


static char usage[]  = "[options|--help|--version] images";
static char detail[] = "images can be a list of any INRIMAGE file or XFLOW2 image.\n\
  - an XFLOW2 image will be displayed as a vector field. \n\
    If several XFLOW images are specified, vector fields \n\
    are surperposed.\n\
  - an INRIMAGE image will be displayed as a background image.\n\
    If several INRIMAGE images are specified, the first one is\n\
    displayed (other image can be selected inside xflow)\n\
  \n\
Options are:\n\
  -sample %d: set value of sample parameter [1-30]\n\
  -scale %f:  set value of scale parameter >0\n\
  -tlow %f:   set value of low threshold parameter\n\
  -thigh %f:  set value of high threshold parameter\n\
  -zoom %f:   set zoom parameter ]0-20]\n\
  -smooth:    smooth vector fields\n\
  -norma:     normalize vector fields\n\
  -mag|-div|-curl|-hsv: display respectively magnetude, divergence,\n\
                        rotational or HSV instead of vectors\n\
  -wt: enable the trajectory manager, experimental!\n\
       -dt %f: set the time step\n\
       -pf %d: set the current position size\n\
       -lw %d: set the trajectory line width\n\
";


int main( int argc, char **argv) {
  char name [256];
  XFLOW_API  *api;
  int have_config;
  
  /* Init Inrimage */

  inr_init( argc, argv, PACKAGE_VERSION, usage, detail); 
  if( igetopt0("--help")) {
    iusage_(usage,detail); 
    return 0;
  }
  if( igetopt0("--version")) {
    fprintf( stderr, "xflow version %s\n", PACKAGE_VERSION);
    return 0;
  }
  debug = igetopt0( "-d");

  with_trajs = igetopt0("-wt"); /* with trajectories */
  if( with_trajs) {
    igetopt1( "-dt", "%f", &DT);
    igetopt1( "-pf", "%d", &point_factor);
    igetopt1( "-lw", "%d", &line_width);    
  }

  /* Init api & chargement des images */
  api = xflow_api_new();

  infileopt( name);
  do {
    int ret = data_addimage( api, name);
    switch( ret) {
    case -2:
      printf("\033[31m%s\033[39m %s is not a valid image\n", "Warning:", name);
      break;
    case -3:
      printf("\033[31m%s\033[39m %s has incompatible dimensions\n", "Warning:", name);
      break;
    }
  } while( infileopt(name));    


  /* config ? */

  have_config = read_config( api);

  /* Options ligne de commandes */
  
  igetopt1("-zoom", "%f", &api->zoom);
  if( api->zoom <= 0) imerror( 1, "-zoom should be strictly postive");
  igetopt1("-scale", "%f", &api->scale);
  if( api->scale <= 0) imerror( 1, "-scale should be strictly postive");
  igetopt1("-sample", "%d", &api->sample);
  igetopt1("-tlow", "%f", &api->thresh);
  igetopt1("-thigh", "%f", &api->thresh_high);


  /* Init Gtk */
  gtk_init (&argc, &argv);

  setlocale( LC_NUMERIC, "C");
  if(debug) printf("LC_NUMERIC set to '%s'\n", setlocale(LC_NUMERIC,NULL));


  xflow_api_show_window( api);
  xflow_api_set_title( api);

  /*
   * paramètres qui nécessitent que la fenêtre soit construite
   */

  if( api->w > 0)
    gtk_window_resize ( GTK_WINDOW( gtk_builder_get_object (api->mainwindow, "xflow_main")), api->w, api->h);
  
  if( api->zoom < 1) {
    GtkWidget * widget = lookup_widget( api->mainwindow, "xflow_main_unzoom");
    api->zoom = 1 / api->zoom;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), 1);
  }
  if( api->scale <1 ) {
    GtkWidget * widget = lookup_widget( api->mainwindow, "xflow_main_unscale");
    api->scale = 1 / api->scale;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), 1);
  }

  if( have_config) {
    GtkWidget * widget = lookup_widget( api->mainwindow, "xflow_main_menu_remember_config");
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(widget), 1);
  }
  
  /* Lire le premier plan des données pour l'affichage */
  data_read( api, 1);
  xflow_api_update_widget( api);

  if( igetopt0("-norma")) {
    GtkWidget * widget = lookup_widget( api->mainwindow, "xflow_main_normalize");
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), 1);
  }
  if( igetopt0("-smooth")) {
    GtkWidget * widget = lookup_widget( api->mainwindow, "xflow_main_smooth");
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), 1);
  }

  
  if( igetopt0("-mag")) {
    GtkWidget *wid = lookup_widget( api->mainwindow, 
				   "xflow_main_notebook");
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(wid), 1);
  } else if( igetopt0("-div")) {
    GtkWidget *wid = lookup_widget( api->mainwindow, 
				   "xflow_main_notebook");
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(wid), 2);
  } else if( igetopt0("-curl")) {
    GtkWidget *wid = lookup_widget( api->mainwindow, 
				   "xflow_main_notebook");
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(wid), 3);
  } else if( igetopt0("-hsv")) {
    GtkWidget *wid = lookup_widget( api->mainwindow, 
				   "xflow_main_notebook");
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(wid), 4);
  }


  gtk_main ();

  return 0;
}

