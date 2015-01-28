/**
 * Convert flo (velocity file) to inrimage
 * 2013 D.Béréziat
 */
#include <inrimage/image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG_FLOAT 202021.25

FILE *flo_open( char *name, int *dimx, int *dimy) {
  float tag; 
  FILE *fp = *name == '<' ? stdin : fopen( name, "rb");
  
  /* FIXME: int pas toujours 32 bits, peu courant */
  if ((int)fread(&tag,  sizeof(float), 1, fp) != 1 ||
      (int)fread(dimx, sizeof(int),   1, fp) != 1 ||
      (int)fread(dimy, sizeof(int),   1, fp) != 1)
    imerror( 10, "bad file format\n");
  if( tag != TAG_FLOAT)
    imerror( 10, "bad file header\n");
  return fp;
}

void flo_read( FILE *fp, float *buf, int size) {
  if( fread( buf, sizeof(float), size, fp) != size)
    imerror( 10, "bad file header size\n");
}

void set_fmt( Fort_int *lfmt, int dimx, int dimy, int dimz) {
  NDIMX = dimx; NDIMY = dimy;
  NDIMV = 2; NDIMZ = dimz;
  DIMX = NDIMX*NDIMV;
  DIMY = NDIMY*NDIMZ;
  EXP = 0; BSIZE=4; TYPE = REELLE;
}

int main( int argc, char **argv) {
  FILE *fp;
  char nom[128], sortie[128];
  int dimx, dimy;
  float *buf;
  Fort_int lfmt[9];
  int y;
  struct image *nf;

  inr_init( argc, argv, "1.0", "[input][output] OR -o output file1 file2 ...", "Convert FLO velocity images to Inrimage");

  if( igetopt1("-o","%s",sortie)) {
    char **liste_entree = NULL;
    int n = 0, i;

    while( infileopt(nom)) {
      liste_entree = (char**)realloc(liste_entree, sizeof(char*)*(n+1));
      liste_entree[n++] = strdup(nom);      
    } 
    
    fp = flo_open( liste_entree[0], &dimx, &dimy);

    set_fmt( lfmt, dimx, dimy, n);
    nf = image_(sortie, "c", "", lfmt);
    buf=(float*)i_malloc(sizeof(float)*DIMX*NDIMY);
    
    flo_read( fp, buf, DIMX*NDIMY);
    c_ecr( nf, NDIMY, buf);
    fclose(fp);
    
    for( i=1; i<n; i++) {
      fp = flo_open( liste_entree[i], &dimx, &dimy);
      if( dimx != NDIMX || dimy != NDIMY)
	imerror( 10, "bad dimension for image %s\n", liste_entree[i]);
      flo_read( fp, buf, DIMX*NDIMY);
      c_ecr( nf, NDIMY, buf);
      fclose(fp);
    }
  } else {
    infileopt(nom);
    fp = flo_open( nom, &dimx, &dimy);

    set_fmt( lfmt, dimx, dimy, 1);
    
    buf=(float*)i_malloc(sizeof(float)*DIMX*NDIMY);
    flo_read( fp, buf, DIMX*NDIMY);
    fclose(fp);
    
    outfileopt(nom);
    nf = image_(nom, "c", "", lfmt);
    c_ecr( nf, NDIMY, buf);
    fermnf_(&nf);

  }
  return 0;
}
