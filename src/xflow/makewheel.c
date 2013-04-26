/**
 *  Génère un champ de vitesse pour la mire Middelbury
 */
#include <inrimage/image.h>
#include <math.h>
#define min(a,b) ((a)<(b)?(a):(b))

int main( int argc, char **argv) {
  char name[256];  
  Fort_int lfmt[9];
  int ix, iy;
  int x0,y0;
  struct image *nf;
  float *buf;

  inr_init( argc, argv, "", "", "");
  outfileopt(name);
  if(igetopt1("-x","%d",&ix))
    NDIMX = ix;
  if(igetopt1("-y","%d",&ix))
    NDIMY = ix;
  NDIMZ=1; NDIMV=2;
  TYPE=REELLE; BSIZE=4;
  DIMX=NDIMV*NDIMX;
  DIMY=NDIMY;

  buf = (float*)i_malloc(sizeof(float)*DIMX*DIMY);
  x0 = NDIMX/2;
  y0 = NDIMY/2;
  for( iy=0; iy < NDIMY; iy++)
    for( ix=0; ix < NDIMX; ix++) {
      float norm = sqrt ((ix-x0) * (ix-x0) + (iy-y0)*(iy-y0));
      if(  ix == x0 && iy == y0) {
	buf[ix*2+iy*DIMX] = buf[(ix-1)*2+iy*DIMX];
	buf[1+ix*2+iy*DIMX] = buf[1+(ix-1)*2+iy*DIMX];
      } else if( norm > min(NDIMX/2,NDIMY/2))
	buf[ix*2+iy*DIMX] = buf[1+ix*2+iy*DIMX] = 0;
      else {
	float coef = norm/min(NDIMX/2,NDIMY/2);
	buf[ix*2+iy*DIMX] = (ix-x0) / norm * coef;
	buf[1+ix*2+iy*DIMX] = (iy-y0) / norm * coef;
      }
    }
  nf = image_(name,"c","", lfmt);
  c_ecr(nf,DIMY,buf);
  fermnf_(&nf);
  return 0;
}
