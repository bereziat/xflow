/*
 * The Middlebury color representation,
 * slightly modified by I.
 * @FIXME devrait être commun avec vel2col
 */
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "middlebury.h"

// relative lengths of color transitions:
// these are chosen based on perceptual similarity
// (e.g. one can distinguish more shades between red and yellow 
//  than between yellow and green)
#define MAXCOL 60
#define RY     15
#define YG      6
#define GC      4
#define CB     11
#define BM     13
#define MR      6

static t_wheel *colorwheel = NULL;

static 
void setcols( int r, int g, int b, int k) {
  colorwheel[k][0] = r;
  colorwheel[k][1] = g;
  colorwheel[k][2] = b;
}

void makecolorwheel( void) {  
  int i;
  int k = 0;

  assert( RY + YG + GC + CB + BM + MR < MAXCOL);
  if( colorwheel == NULL)  colorwheel = (t_wheel *) malloc (sizeof(t_wheel) * MAXCOL);
  
  for (i = 0; i < RY; i++) setcols(255,	         255*i/RY,	 0,	       k++);
  for (i = 0; i < YG; i++) setcols(255-255*i/YG, 255,		 0,	       k++);
  for (i = 0; i < GC; i++) setcols(0,		 255,		 255*i/GC,     k++);
  for (i = 0; i < CB; i++) setcols(0,		 255-255*i/CB,   255,	       k++);
  for (i = 0; i < BM; i++) setcols(255*i/BM,	 0,		 255,	       k++);
  for (i = 0; i < MR; i++) setcols(255,	         0,		 255-255*i/MR, k++);
}

void computeColor(float fx, float fy, unsigned char *pix)
{
  int ncols = RY + YG + GC + CB + BM + MR;
  
  float rad = sqrt(fx * fx + fy * fy);
  float a = atan2(-fy, -fx) / M_PI;
  float fk = (a + 1.0) / 2.0 * (ncols-1);
  int k0 = (int)fk;
  int k1 = (k0 + 1) % ncols;
  float f = fk - k0;
  int b;
  //f = 0; // uncomment to see original color wheel
  for (b = 0; b < 3; b++) {
    float col0 = colorwheel[k0][b] / 255.0;
    float col1 = colorwheel[k1][b] / 255.0;
    float col = (1 - f) * col0 + f * col1;
    if (rad <= 1)
      col = 1 - rad * (1 - col); // increase saturation with radius
    else
      col *= .75; // out of range
    pix[b] = (int)(255.0 * col);
  }
}


void freecolorwheel( void) {
  if( colorwheel) free( colorwheel);
}
