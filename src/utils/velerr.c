#include <inrimage/image.h>
#include <math.h>

/* conforme à l'état de l'art */
void angular_error( float *out, float *l1, float *l2, int size, double d2, double unit) {
  while( size--) {
    *out++ = unit*acos((l1[0] * l2[0] + l1[1]*l2[1] + d2 ) /
	    sqrt( (l1[0]*l1[0] + l1[1]*l1[1] + d2)*(l2[0]*l2[0] + l2[1]*l2[1] + d2)));
    l1+=2; l2+=2;
  }
}

/* Ce que j'ai mis dans ICCV:
 *   |lr-le| / max( |lr|, |le|)
 */
void norm_error_dom( float *out, float *lr, float *le, int size) {
  double alpha=1e-10; /* avoid division by zero */
  while(size--) {
    double m1 = lr[0]*lr[0]+lr[1]*lr[1]+alpha;
    double m2 = le[0]*le[0]+le[1]*le[1]+alpha;

    if (m1<m2) m1=m2;
    *out++ = sqrt((lr[0]-le[0])*(lr[0]-le[0]) + (lr[1]-le[1])*(lr[1]-le[1]))/(2*sqrt(m1));

    lr+=2;le+=2;    
  }
}

/* Calcul de norm_error_percent de stflow (c'est relatif) 
 * formule: max( abs(|lr|^2 - |le|^2) / |lr|^2 , 0) 
 */
void norm_error_percent( float *out, float *lr, float *le, int size) {
  while(size--) {
    double m1=hypot(lr[0],lr[1]);
    double m2=hypot(le[0],le[1]);
    
    *out ++ = (m1>0)?fabs(m1-m2)/m1:0;
    lr+=2;le+=2;    
  }
}

/* Calcul de norm_error de stflow (c'est pas du tout relatif) 
 * formulel |lr-le|^2
 */
void norm_error( float *out, float *lr, float *le, int size) {
  while(size--) {
    *out ++ = hypot(lr[0]-le[0],lr[1]-le[1]);
    lr+=2;le+=2;
  }
}

#define ANGULAR_ERROR 0
#define NORM_ERROR 1
#define NORM_ERROR_PERCENT 2
#define NORM_ERROR_DOM 3

int main(int argc, char **argv) {
  struct image *nf1, *nf2, *nf3;
  Fort_int lfmt[9];
  float *line1, *line2, *line3;
  char name[128];
  int y,iz1=1,iz2=1,z;
  double delta=1, unit=180./M_PI;
  int algo=ANGULAR_ERROR;
  
  inr_init( argc, argv, "1.0", "ref est out [-d delta][-r|-n][-iz iz1 iz2][-z nbz]", 
	    "compute angular or norm error between two 2D vector fields. Defaut\n"
	    "is to compute angular error in degree unit.\n"
	    "-rad: computes angular error in radian unit.\n"
	    "  -n1: computes |ref-est|^2 \n"
	    "  -n2: computes abs(|ref|^2-|est|^2)/|ref|^2 \n"
	    "  -n3: computes |ref-est|/max(|ref|,|est|) \n"
	    " -iz: index of first frame in ref and est to process\n"
	    "  -z: number of frames to process\n"
	    );


  if( igetopt0("-rad")) unit=1; /* radian rather than degree */
  if( igetopt0("-n1")) algo=NORM_ERROR; /* norm !!*/
  if( igetopt0("-n2")) algo=NORM_ERROR_PERCENT; /* norm !!*/
  if( igetopt0("-n3")) algo=NORM_ERROR_DOM; /* norm !!*/

  if( igetopt1("-d","%lf",&delta)) delta*=delta;
  
  infileopt( name);
  nf1 = image_(name, "e", "", lfmt);
  if( NDIMV != 2)
    imerror( 7, "image -v 2 required\n");
  if( TYPE != REELLE)
    imerror( 4, "image -r required\n");

  z = NDIMZ;
  igetopt1( "-z", "%d", &z);
  
  infileopt( name);
  nf2 = image_(name, "e", "", lfmt);
  if( NDIMV != 2)
    imerror( 7, "image -v 2 required\n");
  if( TYPE != REELLE)
    imerror( 4, "image -r required\n");

  line1 = (float*)i_malloc(sizeof(float)*DIMX);
  line2 = (float*)i_malloc(sizeof(float)*DIMX);

  if( igetopt2( "-iz", "%d", &iz1,"%d", &iz2)) {
    c_lptset( nf1, 1+(iz1-1)*NDIMY);
    c_lptset( nf2, 1+(iz2-1)*NDIMY);
  }

  outfileopt( name);
  NDIMV = 1;  DIMX = NDIMX;
  NDIMZ = z;  DIMY = NDIMY*z;
  nf3 = image_(name, "c", "", lfmt);
  line3 = (float*)i_malloc(sizeof(float)*DIMX);

  switch ( algo) {
  case ANGULAR_ERROR:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      angular_error( line3, line1, line2, NDIMX, delta, unit);
      c_ecr( nf3, 1, line3);
    }
    break;
  case NORM_ERROR:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      norm_error( line3, line1, line2, NDIMX);
      c_ecr( nf3, 1, line3);
    }
    break;
  case NORM_ERROR_PERCENT:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      norm_error_percent( line3, line1, line2, NDIMX);
      c_ecr( nf3, 1, line3);
    }
    break;
  case NORM_ERROR_DOM:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      norm_error_dom( line3, line1, line2, NDIMX);
      c_ecr( nf3, 1, line3);
    }
    break;

  }
    
  fermnf_(&nf1);
  fermnf_(&nf2);
  fermnf_(&nf3);
}
