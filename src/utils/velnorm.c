#include <inrimage/image.h>
#include <math.h>

/* norme d'un champ de vecteur 
   image vel -> image scalaire u^2+v^2
*/

int main(int argc, char **argv) {
  char name[128];
  struct image *nf1, *nf2;
  Fort_int lfmt[9];
  float *buf1, *buf2;
  int x, y;

  inr_init( argc, argv, "", "", "");
  infileopt(name);
  nf1 = image_(name,"e","",lfmt);
  if( TYPE != REELLE || BSIZE != 4  || NDIMV != 2) 
    imerror( 6, "xflow file required\n");
  buf1 = (float*)i_malloc(sizeof(float)*DIMX);
  outfileopt(name);
  NDIMV = 1;
  DIMX=NDIMX;
  buf2 = (float*)i_malloc(sizeof(float)*DIMX);
  nf2 = image_(name, "c","", lfmt);
  for( y=0; y<DIMY; y++) {
    c_lect( nf1, 1, buf1);
    for( x=0;x<NDIMX; x++, buf1+=2, buf2++) 
      *buf2 = sqrt((double)*buf1**buf1 + *(buf1+1)**(buf1+1));
    buf2 -= DIMX;
    buf1 -= DIMX*2;
    c_ecr( nf2, 1, buf2);
  }
  fermnf_(&nf1);
  fermnf_(&nf2);
  return 0;
}
