/** 
 * \page velpar Documentation de l'utilitaire velpar
 * 
 * La commande velpar donne des informations sur un fichier XFLOW
 * à la manière de la commande par d'INRIMAGE.
 *
 * $Id: velpar.c,v 1.5 2007/09/24 11:43:37 bereziat Exp $
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>

void minmaxmoy( XFLOW *xflc) {
  vel2d *uv;
 
}

int main(int argc, char **argv) {
  char name[256];

  inr_init( argc, argv, "1.0.0", "[-x|-y|-z|-img|-version] [xflow file [...]]", 
	    "Display information about xflow files specified in command line. "
	    "Options are :\n"
	    "  -x|-y|-z : display image dimensions\n"
	    "  -img : display associated background file\n"
	    "  -version : display libxflow version that build the file\n");
  
  infileopt( name);
  
  do {    
    int x, y, z;
    XFLOW *xflow;
    struct image *im;
    Fort_int lfmt[9];
    
    xflow = xflow_open( name);
    if( xflow == NULL) {
      fprintf( stderr, "%s: fichier inexistant ou mauvais format\n", name);
      continue;
    }
    xflow_get_dims( xflow, &x, &y, &z);
    im = xflow_get_image( xflow, lfmt);

    fprintf( stdout, "%s\t", name);

    if( tstopts()) {
      if( igetopt0( "-x"))
	fprintf( stdout, "-x %d ", x);
      if( igetopt0( "-y"))
	fprintf( stdout, "-y %d ", y);
      if( igetopt0( "-z"))
	fprintf( stdout, "-z %d ", z);
      if( igetopt0( "-version"))
	fprintf( stdout, "-version %d.%d.%d ",  
		 xflow->version.major,
		 xflow->version.minor,
		 xflow->version.release);
      if( igetopt0( "-img"))
	fprintf( stdout, "-img %s ", im?im->nom:"(aucune)");
    } else
      fprintf( stdout, "-x %d -y %d -z %d -img %s -version %d.%d.%d\n", 
	       x, y, z, im?im->nom:"(aucune)",
	       xflow->version.major,
	       xflow->version.minor,
	       xflow->version.release);
    
    
    xflow_close(xflow);
    if( im) fermnf_( &im);
  } while (infileopt(name));
}
