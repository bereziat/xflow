/*
 * Encore en chantier... lui préférer velstats.c
 */

#include <inrimage/image.h>
#include <math.h>

double alpha=1e-10; /* avoid division by zero */

void inv_NS( float *uv, long s) {
  while( s--) {
    uv[1] *= -1;
    uv += 2;
  }
} 

/* Conforme à l'état de l'art  (Barron et al)
 *  calcule l'angle entre deux vecteurs déplacements lr et le dans l'espace 
 *  spatio-temporel cad:
 *    acos [ (lr,d).(le,d) / |(lr,d)||(le,d)| ]
 * Si on considère qu'on calcule des déplacements entre plan,
 * mettre d à 1 pour obtenir des erreurs directions des vecteurs
 * vitesse d'unité pixels/plan.
 * On peut aussi mettre d à très petit pour comparer les orientations
 * des vecteurs déplacements.
 */

void angular_error( float *out, float *lr, float *le, int size, 
		    double d2, double unit) {
  while( size--) {
    double ur = lr[0], vr = lr[2];
    double ue = le[0], ve = le[2];
    *out++ = unit*acos((ur*ue + vr*ve + d2 ) /
		       sqrt( (ur*ur + vr*vr + d2)*
			     (ue*ue + ve*ve + d2)));
    lr+=2; le+=2;
  }
}

/*
 * Norm of absolute error: |lr-le|
 */

void absolute_error_norm( float *out, float *lr, float *le, int size) {
  double alpha=1e-10; /* avoid division by zero */
  while(size--) {
    *out++ = hypot(lr[0]-le[0],lr[1]-le[1]);
    lr+=2;le+=2;    
  }  
}

/*
 * Norm of relative error: |lr-le|/|lr|
 */

void relative_error_norm( float *out, float *lr, float *le, int size) {
  while(size--) {
    *out++ = sqrt(((lr[0]-le[0])*(lr[0]-le[0]) 
		   + (lr[1]-le[1])*(lr[1]-le[1]))/
		  (lr[0]*lr[0]+lr[1]*lr[1]+alpha));
    lr+=2;le+=2;
  }  
}

/*
 * Variante de norm_relative_error().
 * a été utilisé dans RSE.
 *  |lr-le| / 2*max( |lr|, |le|)
 */

void error_dom( float *out, float *lr, float *le, int size) {
  while(size--) {
    double m1 = lr[0]*lr[0]+lr[1]*lr[1]+alpha;
    double m2 = le[0]*le[0]+le[1]*le[1]+alpha;

    if (m1<m2) m1=m2;
    *out++ = sqrt((lr[0]-le[0])*(lr[0]-le[0]) + 
		  (lr[1]-le[1])*(lr[1]-le[1]))/(2*sqrt(m1));    
    lr+=2;le+=2;    
  }
}

/*
 * Relative norm error : abs(|lr| - |le|) / |lr|
 */

void relative_norm_error( float *out, float *lr, float *le, int size) {
  while(size--) {
    double m1=hypot(lr[0],lr[1]);
    double m2=hypot(le[0],le[1]);
    
    *out ++ =  fabs(m1-m2)/(m1+alpha);
    lr+=2;le+=2;    
  }
}

#define ANGULAR_ERROR 0
#define ABSOLUTE_ERROR_NORM 1
#define RELATIVE_ERROR_NORM 2
#define ERROR_DOM 3
#define RELATIVE_NORM_ERROR 4

int main(int argc, char **argv) {
  struct image *nf1, *nf2, *nf3;
  Fort_int lfmt[9];
  float *line1, *line2, *line3;
  char name[128];
  int y,iz1=1,iz2=1,z;
  double delta=1;
  double unit=180./M_PI;
  int algo = ANGULAR_ERROR;
  int cart;

  inr_init( argc, argv, "1.1", "ref est out [options]\n",
	    "compute angular or norm error between two 2D vector fields. Default\n"
	    "is to compute angular error in degree unit. Options are:\n"
	    "  -d: third dimension coordinate (for angular error), default is 1\n"
	    "  -v: assume vectors are velocities, shortcut for -d 1e-10\n"
	    "  -rad: computes angular error in radian unit.\n"
	    "  -cart: computes angular error in cartesian plan (instead of image plan)\n"
	    "  -aen: computes norm of absolute error (|ref-est|) \n"
	    "  -ren: computes norm of relative error (|ref-est|)/|ref| \n"
	    //	    "  -ed: computes error dom |ref-est|/2*max(|ref|,|est|) \n"
	    "  -rne: computes relative norm error (|ref|-|est|)/|ref|) \n"
	    " -iz: index of first frame in ref and est to process\n"
	    "  -z: number of frames to process\n"
	    );


  if( igetopt0("-aen")) algo=ABSOLUTE_ERROR_NORM;
  if( igetopt0("-ren")) algo=RELATIVE_ERROR_NORM;
  if( igetopt0("-dom")) algo=ERROR_DOM;
  if( igetopt0("-rne")) algo=RELATIVE_NORM_ERROR;

  if( igetopt0("-rad")) unit=1; /* radian rather than degree */
  if( igetopt1("-d","%lf",&delta)) delta*=delta;
  cart=igetopt0("-cart"); /* si -cart, utiliser le
			     repère cartésien plutôt qu'image */

  if( igetopt0( "-v"))
    delta = alpha*alpha;

  infileopt( name);
  nf1 = image_(name, "e", "", lfmt);
  if( NDIMV != 2)
    imerror( 7, "%s must have two components\n", name);
  if( TYPE != REELLE)
    imerror( 4, "%s must be float\n", name);

  z = NDIMZ;
  igetopt1( "-z", "%d", &z);
  
  infileopt( name);
  nf2 = image_(name, "e", "", lfmt);
  if( NDIMV != 2)
    imerror( 7, "%s must have two components\n", name);
  if( TYPE != REELLE)
    imerror( 4, "%s must be float\n", name);

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
      if( cart) {
	inv_NS( line1, DIMX);
	inv_NS( line2, DIMX);
      }
      angular_error( line3, line1, line2, NDIMX, delta, unit);
      c_ecr( nf3, 1, line3);
    }
    break;
  case ABSOLUTE_ERROR_NORM:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      absolute_error_norm( line3, line1, line2, NDIMX);
      c_ecr( nf3, 1, line3);
    }
    break;
  case RELATIVE_ERROR_NORM:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      relative_error_norm( line3, line1, line2, NDIMX);
      c_ecr( nf3, 1, line3);
    }
    break;
  case ERROR_DOM:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      error_dom( line3, line1, line2, NDIMX);
      c_ecr( nf3, 1, line3);
    }
    break;
  case RELATIVE_NORM_ERROR:
    for( y=0; y<DIMY; y++) {
      c_lect( nf1, 1, line1);
      c_lect( nf2, 1, line2);
      relative_norm_error( line3, line1, line2, NDIMX);
      c_ecr( nf3, 1, line3);
    }
    break;

  }
    
  fermnf_(&nf1);
  fermnf_(&nf2);
  fermnf_(&nf3);
}
