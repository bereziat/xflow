/**
 * visualisation d'un champ de vecteur par couleur:
 * magnétude -> Y  (luminance). Cela implique de 
 *              normaliser les vecteurs. On peut
 *              un parametre qui elimine n% les plus
 *              grands : PARAMETRE LOW/HIGH pour XFLOW !
 * orientation : U -> cos();

Jonathan : un carré :
Jaune ------ Vert
  |           |
  |           |
  |    Noir   |
  |           |
  |           |
Magenta ---- Bleu
 */


#include <inrimage/xflow.h>

int main( int argc, char **argv) {
  char nom[128];
  xflow *xf_in;
  vel2d *uv;

  struct image *nf_out;
  float *magcol;
  Fort_int lfmt[9];

  inr_init( argc, argv, "", "", "");
  
  infileopt( nom);
  xf_in = xflow_open( nom);

  outfileopt( nom);
  xflow_get_lfmt( xf_in, lfmt); 
  NDIMV = 3;
  DIMX = NDIMX * NDIMV;
  TYPE = FIXE;
  BSIZE = 1;
  EXP = 0;
  nf_out = image_( nom, "c", "", lfmt);

  uv = (float *) i_malloc( sizeof(v2d) * NDIMX * NDIMY);
  magcol = (float *) i_malloc( 3 * sizeof(float) * NDIMX * NDIMY);

  for( i=0; i<NDIMZ; i++) {
    xflow_read_f_v2d( xf_in, 1, uv);
    
    to_magcol( magcol, uv, NDIMX, NDIMY, 2);

    c_ecrflt( nf_out, NDIMY, magcol);
  }

  fermnf_( &nf_out);
  xflow_close( xf_in);
  
  return 0;
}

void to_magcol( float* magcol, float *uv, int dimx, int dimy, int strip) {
  size_t count;

  for( count = dimx*dimy; count ; count --) {
    magcol[3count] = vel2d[count].u * vel2d[count].u + vel2d[count].v * vel2d[count].v;  
  }

  /* seuillage & min/max */
  ltp =* dimx*dimy;
  htp =* dimx*dimy;
  min = max = *maguv;
  for( count = dimx*dimy; count ; count --) {
    if( 
  }

}
