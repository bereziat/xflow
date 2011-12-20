#include <inrimage/image.h>
#include <math.h>

void inv_NS( float *uv, long s) {
  while( s--) {
    uv[1] *= -1;
    uv += 2;
  }
} 


/* orientation of a vector field */

double alpha = 1e-10;

int main(int argc, char **argv) {
  char name[128];
  struct image *nf1, *nf2;
  Fort_int lfmt[9];
  float *buf1, *buf2;
  int x, y;
  double unit = 180./M_PI;
  int cart;

  inr_init( argc, argv, "1.0", "vel [-rad][-cart]", 
	    "compute orientation of a vector field.\n"
	    "\t-rad: compute in radian unit\n"
	    "\t-cart: compute in cartesian plan rather image plan\n"
	    );

  /* Lecture image + test format */
  infileopt(name);
  nf1 = image_(name,"e","",lfmt);
  if( TYPE != REELLE || BSIZE != 4  || NDIMV != 2) 
    imerror( 6, "xflow file required\n");
  buf1 = (float*)i_malloc(sizeof(float)*DIMX);

  /* sortie */
  outfileopt(name);
  NDIMV = 1;
  DIMX=NDIMX;
  buf2 = (float*)i_malloc(sizeof(float)*DIMX);
  nf2 = image_(name, "c","", lfmt);

  /* paramètres */
  if(igetopt0("-rad")) unit=1; /* unité en radian ?*/
  cart=igetopt0("-cart"); /* si -cart, utiliser le
			     repère cartésien plutôt qu'image */

  /* traitement */
  for( y=0; y<DIMY; y++) {
    c_lect( nf1, 1, buf1);
    if( cart) inv_NS(buf1,DIMX);
    for( x=0;x<NDIMX; x++, buf1+=2, buf2++) 
      *buf2 = unit * atan ( *(buf1+1) / (*buf1+alpha));
    buf2 -= DIMX;
    buf1 -= DIMX*2;
    c_ecr( nf2, 1, buf2);
  }
  fermnf_(&nf1);
  fermnf_(&nf2);
  return 0;
}

