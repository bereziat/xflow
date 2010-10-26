/* 
 * Calcul du LIC (Line Integral Convolution)
 *
 * auteurs : N.David (stage ingénieur)/H.Yahia INRIA (1999) 
 * $Id: vel2lic.c,v 1.5 2007/09/24 11:43:36 bereziat Exp $
 */


/**
 * \page vel2lic Commande vel2lic
 *
 * Calcule le LIC (Line Integral Convolution) d'un champ de vecteurs.
 * Le LIC est une façon de visualiser un champ de vecteur. Le principe est
 * simple, pour tout points de l'image, on calcule la trajectoire du point
 * le long des lignes du champs de vecteur. Puis, en chaque point de l'image
 * LIC, on intégre les valeurs de l'image de long de la trajectoire (sur un
 * voisinnage en fait). Le LIC représente le champ
 * de force d'un champ de vecteur.
 *
 * Pour plus de détail, tapez :
 *
 * \code
 * % vel2lic -help
 * \endcode
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inrimage/xflow.h>

static char version[] = "1.1.0";
static char command[] = "[input] [output] [-l %d] [-bb]";
static char details[] = 
"Compute the Line Integral Convolution (LIC) from a vector flow.\n\
Commands are :\n\
\tinput  : vector field input data (XFLOW 2 format)\n\
\toutput : ouput image of LIC\n\
\t   -bb : use a blank noise image as input instead of the\n\
\t         image background associated to input\n\
\t-l %d : trajectory lenght\
";

static float calcul_ds ( float Px, float Py, float *VectX, float *VectY, int dimX, int verb, int sens );
static float calcul_h  (float ds, float Px, float Py, float *In, int dimX);
static void  Next_P    ( float *pPx, float *pPy, float *VectX, float *VectY, float ds, int dimX);
static float F         ( float *In, float Px, float Py, int dimX);

#define MIN(X,Y) ((X<Y)?X:Y)

int FlowLICB_ComputeLICMethod( float *In, vel2d *uv, float *Out, 
			       int dimX, int dimY, int L, 
			       float FixedNormalize, int verb) {

  
  int x,y,index;
  float s,SFh,Sh,ds,h;
  float Px,Py;
  int null_vector;
  float norm;

  float *VectX, *VectY, *pu, *pv;
  size_t count = dimX * dimY;


  pu = VectX = (float*)malloc(sizeof(float)*count);
  pv = VectY = (float*)malloc(sizeof(float)*count);

  while( count --) {
    norm = sqrt(uv->u*uv->u+uv->v*uv->v);
    // norm = 1;
    if( norm) {
      *pu = uv->u/norm;
      *pv = uv->v/norm;
    }
    pu ++; pv ++;
    uv ++;
  }

  for (y=0; y<dimY; y++) {
    if (!(y % 10)) fprintf( stderr, "\r%d%%", (int)((float)y*100./(float)dimY)); 
    for (x=0; x<dimX; x++) {
      index = y*dimX + x;
      Px = (float) x + 0.5;
      Py = (float) y + 0.5;
      s = 0; SFh = 0; Sh = 0;


      null_vector = 0;
      while ( !null_vector && s <= L ) {
 	if( (ds = calcul_ds( Px, Py, VectX, VectY, dimX, verb, 1))==0 ) { 
	  null_vector = 1; 
	  continue; 
	}
	h  = calcul_h(ds,Px,Py,In,dimX);	
	SFh += h*F( In, Px, Py, dimX);
	Sh += h;
	
	Next_P( &Px, &Py, VectX, VectY, ds, dimX);
	s += ds;

	/* ce n'est pas un vecteur nul mais c'est pour sortir car on a touche un bord de l'image */
      	if ( Px<0 || Py<0 || Px>dimX-1 || Py>dimY-1 ) null_vector=1;
      }
      
      Px = (float) x + 0.5;
      Py = (float) y + 0.5;
      s = 0;
      
      null_vector = 0;
      while ( !null_vector && s <= L ) {
	if ((ds = calcul_ds(Px,Py,VectX,VectY,dimX,verb,-1))==0) { 
	  null_vector = 1; 
	  continue; 
	}
	h  = calcul_h(ds,Px,Py,In,dimX);
	SFh += h*F(In,Px,Py,dimX);
	Sh += h;
	
	Next_P(&Px,&Py,VectX,VectY,-ds,dimX);
	s += ds;


	/* ce n'est pas un vecteur nul mais c'est pour sortir car on a touche un bord de l'image */
      	if ( Px<0 || Py<0 || Px>dimX-1 || Py>dimY-1 ) null_vector=1;

      
      if (Sh)
	if (FixedNormalize)
	  Out[index] = SFh/FixedNormalize;
	else
	  Out[index] = SFh/Sh;
      else
	Out[index] = 0;

      }
    }
  }
  fprintf( stderr, "\r100%%");
  free( VectX);
  free( VectY);
  return 0;
}

/*****************************************************************************/
static
float calcul_ds( float Px, float Py, float *VectX, float *VectY, int dimX, int verb, int sens) {
/*****************************************************************************/
  float stop, sbottom, sleft, sright;
  int index;
  float ds;
  
  /* NB : ds is between 0 et 1 */
  /* sens is -1 or +1 : ti is to determinate if we calcul the ds to the edge on the vector sens or the opposite */


  index = (int) ( floorf(Py) * dimX +  floorf(Px) );
  
  /* null vector */
 if (!VectX[index] && !VectY[index]) return 0;
  

  /* calcul s(e,c)...if vector // to an edge : value =  infiny */
 
  if (!VectX[index]) { 
    sleft = 9999999999; sright = 9999999999; }
  else { 
    sleft  = - ( Px - floorf(Px) ) / VectX[index]*sens;
    sright =   ( ceilf(Px) - Px  ) / VectX[index]*sens;}
  if (!VectY[index]) { 
    stop = 9999999999; sbottom = 9999999999; }
  else { 
    stop   = - ( Py - floorf(Py) ) / VectY[index]*sens;
    sbottom=   ( ceilf(Py) - Py  ) / VectY[index]*sens;
  }
  
  /* keep only the positive value...the negative corresponding to the false sens of the vector */
  if (stop<=0.00005)    stop   =9999999999;
  if (sbottom<=0.00005) sbottom=9999999999;
  if (sleft<=0.00005)   sleft  =9999999999;
  if (sright<=0.00005)  sright =9999999999;

  ds = MIN(MIN(stop,sbottom),MIN(sleft,sright));
  ds += 0.005;

  if (ds>2) ds = 0;

  return ds;
}


/*****************************************************************************/
static
float calcul_h(float ds, float Px, float Py, float *In, int dimX) {
/*****************************************************************************/
  int index;

  index = ((int) floorf(Py)) * dimX + ((int) floorf(Px));
  return ds*In[index];

}

/*****************************************************************************/
static
void Next_P( float *pPx, float *pPy, float *VectX, float *VectY, float ds, int dimX) {
/*****************************************************************************/
  int index;

  index = ((int) floorf(*pPy)) * dimX + ((int) floorf(*pPx));

  /* nb :  the norm is not null because it has been verified before inside the while loop */
  *pPx += ds*VectX[index];
  *pPy += ds*VectY[index];
  
}


/* OK */
static
float F( float *In, float Px, float Py, int dimX) {
  int index;
  
  index = ((int) floorf(Py)) * dimX + ((int) floorf(Px));
  return In[index];
}

int main( int argc, char **argv) {
  XFLOW *xfl;
  char name[256];
  vel2d *uv;
  Fort_int lfmt[9], gfmt[9];
  float *image, *out, *pimg;
  size_t count ;
  struct image *im, *im_in;
  int l = 10;
  int z;

  inr_init( argc, argv, version, command, details);
  infileopt( name);
  xfl = xflow_open( name);
  xflow_get_lfmt( xfl, lfmt);
  if( !igetopt0( "-bb"))
    im_in = xflow_get_image( xfl, gfmt);
  else
    im_in = NULL;
  igetopt1( "-l", "%d", &l);

  uv    = (vel2d *)malloc( sizeof(vel2d)*NDIMX*NDIMY);

  image = (float *)malloc( sizeof(float)*NDIMX*NDIMY);
  out   = (float *)malloc( sizeof(float)*NDIMX*NDIMY);

  /* bb sur image */
  pimg = image;
  
  for( count = NDIMX*NDIMY; count>0; count --)
    *pimg++ = (float) random() / (float) RAND_MAX ;
  
    /*  } else {
	int i,j;
	for ( i = 0; i< NDIMY; i++)
	for ( j = 0; j< NDIMX; j++) {
	if( (i % 10)==0 && (j % 10)==0)
	*pimg++ = 1
	else
	*pimg++ = 0;
	}
	} */

  outfileopt( name);
  NDIMV = 1;
  DIMX = NDIMX;
  im = image_( name, "c", "", lfmt);

  z = NDIMZ;
  while( z--) {
    fprintf( stderr, "Process frame %d...\n", NDIMZ-z);    
    if( im_in) c_lecflt( im_in, NDIMY, image);
    
    xflow_read_f_v2d( xfl, 1, uv);
    FlowLICB_ComputeLICMethod( image, 
			       uv, 
			       out,
			       NDIMX, NDIMY,
			       l, 0, 0);
    c_ecr( im, NDIMY, out);
    fprintf( stderr, "\n");
  }
  xflow_close (xfl);  
  fermnf_( &im);
  if( im_in) fermnf_(&im_in);
  return 0;
}
