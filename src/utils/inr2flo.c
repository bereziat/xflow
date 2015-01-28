#include <inrimage/image.h>
#include <stdio.h>
#include <stdlib.h>

#define TAG_FLOAT 202021.25

int main( int argc, char **argv) {
  struct image *nf;
  FILE *fp;
  float *buf;
  int dimx, dimy;
  float tag;
  Fort_int lfmt[9];
  char name[128];


  inr_init(argc, argv, "", "", "");


  infileopt(name);
  nf = image_( name, "e", "", lfmt);
  if( NDIMV != 2 || TYPE != REELLE)
    imerror( 10, "Not an velocity file");
  buf = (float*)malloc( sizeof(float)*DIMX*NDIMY);
  c_lect( nf, NDIMY, buf);
  fermnf_(&nf);


  outfileopt(name);
  fp = *name == '>' ? stdout:fopen(name,"wb");
  tag = TAG_FLOAT;
  fwrite( &tag, sizeof(float), 1, fp);
  dimx = NDIMX; dimy = NDIMY;
  fwrite(&dimx,sizeof(int), 1, fp);
  fwrite(&dimy,sizeof(int), 1, fp);
  fwrite(buf,sizeof(float),DIMX*NDIMY, fp);
  fclose(fp);

  return 0;
}

