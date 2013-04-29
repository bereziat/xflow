#include <gtk/gtk.h>
#include <locale.h>
#include "data.h"
#include "utils.h"
#include "api.h"

int debug = 0;

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
  -sample %d: set value of sample parameter (1-30)\n\
  -scale %f:  set value of scale parameter\n\
  -tlow %f:   set value of low threshold parameter\n\
  -thigh %f:  set value of high threshold parameter\n\
  -zoom %f:   set zoom parameter (1-20)\n\
  -smooth:    smooth vector fields\n\
  -norma:     normalize vector fields\n\
";


int main( int argc, char **argv) {
  char name [256];
  XFLOW_API  *api;

  /* Init Inrimage */
  sprintf( name, "%d.%d.%d %s", XFLOW_API_MAJOR, XFLOW_API_MINOR, XFLOW_API_RELEASE,
	   XFLOW_API_BETA?"beta":"");
  inr_init( argc, argv, name, usage, detail); 
  if( igetopt0("--help")) {
    iusage_(usage,detail); 
    return 0;
  }
  if( igetopt0("--version")) {
    fprintf( stderr, "xflow version %s\n", name);
    return 0;
  }
  debug = igetopt0( "-d");

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


  /* Options ligne de commandes */
  igetopt1("-zoom", "%f", &api->zoom);
  igetopt1("-scale", "%f", &api->scale);
  igetopt1("-sample", "%d", &api->sample);
  igetopt1("-tlow", "%f", &api->thresh);
  igetopt1("-thigh", "%f", &api->thresh_high);

  /* Init Gtk */
  gtk_init (&argc, &argv);

  setlocale( LC_NUMERIC, "C");
  if(debug) printf("LC_NUMERIC set to '%s'\n", setlocale(LC_NUMERIC,NULL));
  
  xflow_api_show_window( api);

  /* Lire le premier plan des donn�es pour l'affichage */
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
  gtk_main ();

  return 0;
}

