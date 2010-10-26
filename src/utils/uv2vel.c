/**
 * Programme de test de xflow
 * prend deux tampons u,v et transforme
 * dans un fichier XFLOW avec choix de la
 * fenetre et du pas de sous-échantillonage
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>

int main( int argc, char **argv) {
  char name[256];
  Fort_int lfmt[9];
  struct image *im_u, *im_v;
  float *pu, *pv;
  XFLOW *xflow;
  int dx=4, dy=4, ix=1, iy=1, iz=1;
  int j;

  inr_init( argc, argv, "", "", "");

  infileopt(name);
  im_u = image_( name, "e", " ", (void*)lfmt);
  infileopt(name);
  im_v = image_( name, "e", "a", (void*)lfmt);

  outfileopt(name);
  xflow = xflow_create( name, NDIMX, NDIMY, NDIMZ);
  
  pu = (float*)i_malloc(sizeof(float)*NDIMX);
  pv = (float*)i_malloc(sizeof(float)*NDIMX);

  
  for( j=0; j<DIMY; j++) {
    c_lecflt( im_u, 1, (char *)pu);
    c_lecflt( im_v, 1, (char *)pv);
    xflow_write( xflow, 1, pu, pv);
  }

  xflow_close( xflow);
  fermnf_( &im_u);
  fermnf_( &im_v);
  
  return 0;
}
