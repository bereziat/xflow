/**
 * \page velset Documentation de l'utilitaire velset.
 *
 * La commande velset modifie ou créée un fichier XFLOW. Elle
 * sert dans les scripts en conjonction d'autres commandes 
 * Inrimage.
 *
 * \section velset_usage Usage
 * 
 * \section velset_exemple Exemple
 * Créer une image XFLOW (vide) en utilisant le format de l'image taxi.inr.gz :
 * \verbatim % velset -blank  taxi.inr.gz > taxi.uv \endverbatim
 *  
 *
 * $Id: velset.c,v 1.4 2007/09/24 11:43:37 bereziat Exp $
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>

char vers[] = "1.0.0";
char cmd[] = "[options] file";
char detail[] = "\
Modify a XFLOW file. Options are :\n\
\t-blank background image : create a blank file\n\
\t-set u v [-ix ix][-iy iy][-iz iz] : set pixel ix iy iz to value (u,v)\n\
\t-img infile: define the background image name\
";

int main( int argc, char **argv) {
  char par1[256];
  char par2[256];
  float u, v;
  XFLOW *xfl;
  Fort_int lfmt[9];

  inr_init( argc, argv, vers, cmd, detail);
  
  /** Creer un fichier XFLOW vide **/
  if( igetopt1( "-blank", "%s", par1)) {
    struct image *img;
    vel2d *uv;
    int z;

    img = image_( par1, "e", "", (void*)lfmt);
    fermnf_(&img);

    outfileopt(par2);
    xfl = xflow_create( par2, NDIMX, NDIMY, NDIMZ);
    xflow_set_image( xfl, par1);

    uv = (vel2d *)i_calloc( NDIMX*DIMY,sizeof(vel2d));
    z = NDIMZ;
    while( z--) xflow_write_v2d( xfl, NDIMY, uv);
    xflow_close( xfl);
    
  } else     
    /** Ajouter un vecteur **/    
    if( igetopt2( "-set", "%f", &u, "%f", &v)) {
      int x=1,y=1,z=1;
      vel2d *uv;
      int dimx, dimy, dimz;

      igetopt1( "-ix", "%d", &x);
      igetopt1( "-iy", "%d", &y);
      igetopt1( "-iz", "%d", &z);


      infileopt( par2);
      xfl = xflow_open( par2);
      xflow_get_dims( xfl, &dimx, &dimy, &dimz);
      /* admissibilité des paramètres */	
      if( x>0 && x <= dimx &&
	  y>0 && y <= dimy &&
	  z>0 && z <= dimz) {
	x --; y --;
	xflow_seek_f( xfl, z);
	uv = (vel2d *)i_malloc( dimx * dimy * sizeof(vel2d));
	xflow_read_f_v2d( xfl, 1, uv);
	uv[x+y*dimx].u = u;
	uv[x+y*dimx].v = v;
	xflow_seek_f( xfl, z);
	xflow_write_f_v2d( xfl, 1, uv);
	xflow_close( xfl);
      } else
	  fprintf( stderr, "%s -set: paramètre hors domaine.\n", argv[0]);
    } else
      if( igetopt1( "-img", "%s", par1) ||
	  igetopt1( "-name", "%s", par1) /* ancient option name */ ) {
	infileopt( par2);
	xfl = xflow_open( par2);
	xflow_set_image( xfl, par1);
	xflow_close( xfl);
      } else
	iusage_(cmd,detail);
  return 0;
}
