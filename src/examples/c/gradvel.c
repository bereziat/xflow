/**
 * \page gradvel
 * Exemple d'utilisation de la librairie libxflow.
 * On calcule le gradient d'une image (avec la librairie Edges) et 
 * on crée un fichier XFLOW en résultat.
 *
 * Voir aussi le fichier gradvel.sh qui est le strict équivalent en
 * script bash.
 */

#include <inrimage/image.h>
#include <xflow.h>
#include <edges.h>

int main( int argc, char **argv) {
  char name[256];
  Fort_int lfmt[9];
  struct image *img;
  XFLOW *xflow;
  float *gx, *gy;
  void *buf;
  int z;

  /* Parametres pour edges */
  derivativeOrder dx[3] = {DERIVATIVE_1,DERIVATIVE_0,DERIVATIVE_0};
  derivativeOrder dy[3] = {DERIVATIVE_0,DERIVATIVE_1,DERIVATIVE_0};
  int   deriv_border[3] = {10,10,10};
  float deriv_coefs[3] = {1.,1.,1.};
  recursiveFilterType deriv_type = ALPHA_DERICHE;
  int dims[3];
  int type;

  inr_init( argc, argv, "", "", "");

  /* Options de dérivations */
  if( igetopt1( "-a", "%f", deriv_coefs)) {
    deriv_type = ALPHA_DERICHE;
    deriv_coefs[2] = deriv_coefs[1] = deriv_coefs[0];
  }
  if( igetopt1( "-s", "%f", deriv_coefs)){
    deriv_type = GAUSSIAN_DERICHE;
    deriv_coefs[2] = deriv_coefs[1] = deriv_coefs[0];
  }
  if( igetopt1( "-cont", "%d", deriv_border)) {
    deriv_border[2] = deriv_border[1] = deriv_border[0];
  }  

  /* Image d'entrée */
  infileopt(name);
  img = image_( name, "e", " ", (void*)lfmt);

  //  buf = (void *)i_malloc( BSIZE*NDIMX*DIMY);
  buf = (void *)i_malloc( sizeof(float)*NDIMX*DIMY);
  gx = (float *)i_malloc( sizeof(float)*NDIMX*DIMY);
  gy = (float *)i_malloc( sizeof(float)*NDIMX*DIMY);

  /* Champ de vecteurs en sortie */
  outfileopt(name);
  xflow = xflow_create( name, NDIMX, NDIMY, NDIMZ);
  xflow_set_image( xflow, img->nom);
  dims[0] = NDIMX;
  dims[1] = NDIMY;  
  dims[2] = NDIMZ;

  if( TYPE == FIXE && BSIZE == 1) type = UCHAR;
  if( TYPE == FIXE && BSIZE == 2) type = USHORT;
  if( TYPE == FIXE && BSIZE == 4) type = INT;
  if( TYPE == REELLE && BSIZE == 4) type = FLOAT;
  if( TYPE == REELLE && BSIZE == 8) type = DOUBLE;
  type = FLOAT;
  c_lecflt( img, DIMY, (char *)buf);
  RecursiveFilterOnBuffer( buf, type, gx, FLOAT, 
			   dims, deriv_border, dx, 
			   deriv_coefs, deriv_type);
  RecursiveFilterOnBuffer( buf, type, gy, FLOAT, 
			   dims, deriv_border, dy, 
			   deriv_coefs, deriv_type);

  xflow_write_f( xflow, NDIMZ, gx, gy);
  

  xflow_close( xflow);
  fermnf_( &img);

  return 0;
}
