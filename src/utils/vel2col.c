/* 
 * Réécriture inrimage du color_flow.cpp de Middlebury
 * + modifs locales. 
 */


#include <inrimage/image.h>
#include <stdio.h>
#include <math.h>

typedef unsigned char uchar;
#include "colorcode.h"

// return whether flow vector is unknown

#define UNKNOWN_FLOW_THRESH 1e9

int unknown_flow(float u, float v) {
  return (fabs(u) >  UNKNOWN_FLOW_THRESH) 
    || (fabs(v) >  UNKNOWN_FLOW_THRESH)
    || isnan(u) || isnan(v);
}

int verbose = 0;

void vel2col( unsigned char *color, float *vel, int dimx, int dimy, float maxmotion) {
    int x, y;

    // determine motion range:
    float maxx = -999, maxy = -999;
    float minx =  999, miny =  999;
    float maxrad = -1;
    for (y = 0; y < dimy; y++) {
	for (x = 0; x < dimx; x++) {
	  float fx = *vel++;
	  float fy = *vel++;
	  
	  if (unknown_flow(fx, fy))
	    continue;
	  maxx = fmax(maxx, fx);
	  maxy = fmax(maxy, fy);
	  minx = fmin(minx, fx);
	  miny = fmin(miny, fy);
	  float rad = sqrt(fx * fx + fy * fy);
	  maxrad = fmax(maxrad, rad);
	}
    }

    if( verbose)
      fprintf( stderr, "max motion: %.4f  motion range: u = %.3f .. %.3f;  v = %.3f .. %.3f\n",
	       maxrad, minx, maxx, miny, maxy);

    
    if (maxmotion > 0) // i.e., specified on commandline
      maxrad = maxmotion;
    
    if (maxrad == 0) // if flow == 0 everywhere
      maxrad = 1;
    
    if (verbose)
      fprintf(stderr, "normalizing by %g\n", maxrad);

    vel -= dimx*dimy*2;

    for (y = 0; y < dimy; y++) {
      for (x = 0; x < dimx; x++) {
	float fx = *vel ++;
	float fy = *vel ++;
	unsigned char *pix = color ;
	if (unknown_flow(fx, fy)) {
	  pix[0] = pix[1] = pix[2] = 0;
	} else {
	  computeColor(fx/maxrad, fy/maxrad, pix);
	}
	color += 3;
      }
    }
}

int main(int argc, char **argv) {
  char nom[128];
  struct image *nf1, *nf2;
  Fort_int lfmt[9];
  float *vel, maxmotion = 0;
  unsigned char *color;

  inr_init( argc, argv, "1.02", "[velocityfile][colorimage] [-m maxmotion] -V", 
	    "Convert a 2D velocity file in a colored image using Middlebury algorithm.\n"
	    "Parameter -V: enable verbosity\n"
	    "Parameter -m: the algorithm normalizes (in norm) the vector field in order\n"
	    "the vector with the highest norm has the most saturated color, the lowest \n"
	    "is white. The vector field may be divided by maxmotion to change the \n"
	    "threshold of the maximal saturation.\n");

  igetopt1( "-m", "%f", &maxmotion);
  verbose = igetopt0("-V");
  infileopt( nom);

  nf1 = image_( nom, "e", "", lfmt);
  if (NDIMV != 2|| TYPE != REELLE || BSIZE != 4)
    imerror( 6, "Codage non accepté\n");

  vel = (float*) i_malloc(sizeof(float)*DIMX*NDIMY);
  color = (unsigned char*) i_malloc(3*NDIMX*NDIMY);

  outfileopt(nom);
  NDIMV = 3;
  DIMX = 3*NDIMX;
  TYPE = FIXE;
  BSIZE = 1;
  EXP = 0;
  nf2 = image_( nom, "c", "", lfmt);

  int iz = 1;
  while ( NDIMZ--) {
    c_lect( nf1, NDIMY, vel);
    // c_lectpl( nf1, 1, iz, 1, NDIMY, vel);
    vel2col( color, vel, NDIMX, NDIMY, maxmotion);
    c_ecr( nf2,  NDIMY, color);
    // c_ecrpl( nf2, 1, iz++, 1, NDIMY, color);
  }
  fermnf_(&nf1);
  fermnf_(&nf2);

  return 0;
}
