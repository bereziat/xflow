/*
 * genere un code Xfig pour une figure contenant :
 *  - l'image
 *  - le champ de vecteurs (sous echantillonn�)
 * (c) 2003/2013 D.B�r�ziat LIP6/UPMC
 * genere aussi le eps et le tex a inclure dans LaTeX
 *
 * Version 1.4.2: modif dans le calcul des tailles de fleches pour convenir � la fois aux grandes et petites images.
 * Version 1.4.1: vel2fig g�re correctement les fichiers qui ne sont pas dans le r�pertoire courant
 * Version 1.4.0: ajout option -roi='ix,iy,x,y,w,col' mettre 0 a w pour extraire le ROI
 * Version 1.3.3: ajout seuil relatif (postfixe %) + Fix temporaire image couleurs & inr2gif + fix bug vitesse a composante nulle
 * Version 1.3.2: ajout -asize huge et -ahead <width> <height>
 * Version 1.3.1 : controle absence d'argument, ajout option -nf (noframe)
 * Version 1.3.0 : ajout option -norma
 * Version 1.2.5 : fixe bogue: image background couleur -v 3 correctement affichee
 * Version 1.2.4 : ajout option -sarrow
 * Version 1.2.3 : ajout option -smooth
 * Version 1.2 : on ajoute une boite blanche autour de l'image
 *               option -bb pour �liminer les vecteurs qui sortent de l'image
 *
 * $Id: vel2fig.c,v 1.14 2010/09/28 20:55:40 bereziat Exp $
 */


/**
 * \page vel2fig Commande vel2fig
 *
 * Programme pour g�n�rer des figures postscript (ou autre) � partir
 * de fichiers xflow. Attention, ce programme n�cessite que xfig soit
 * install� (en particulier, il utilise le programe fig2dev). Il est
 * utilis� par xflow pour exporter des figures.
 *
 * Pour plus de d�tail, tapez :
 *
 * \code
 * % vel2fig -help
 * \endcode
 */


#include <inrimage/image.h>
#include <inrimage/xflow.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <libgen.h>

#define MAX(a,b)  ((a)>(b)?(a):(b))
extern int debug_;

/* enl�ve l'extension d'un nom de fichier, 
 * l'extension .gz est pris en compte */

char *rmext( char *name) {
  char *p;
  basename( name);
  p = strrchr( name, '.');
  if( p) *p = '\0';
  if( !strcmp( p+1, "gz")) {
    p = strrchr( name, '.');
    if( p) *p = '\0';
  }
  return name;
}

char version[]="1.4.1";
char cmd[] = "[global-options] file1 [file1-options] file2 [file2-options] ...";
char help[]=
"Create a Xfig output from an INRIMAGE sequence and one or several XFLOW data.\n\
This program requires the fig2dev command (provided by xfig or transgif)\n\
Global options are:\n\
\t-o %s    : generic output filename. Default is 'xflow-export'.\n\
\t-iz %d   : frame index in the sequence to process.\n\
\t-frame   : alias of -iz.\n\
\t-size %s : size of output figure. Format is %fin for inche unit.\n\
\t           or %fcm for centimeter unit, %f standing for a float number.\n\
\t-type %s : output format. Usual formats are: eps,ps,jpeg,... see\n\
\t           fig2dev -L for an exhaustive list.\n\
\t-q %d    : jpeg quality (0--100) if -type jpeg has been specified\n\
\t-fig     : only generate an xfig output file. In this case, the fig2dev\n\
\t           command is not invoked.\n\
\t-nvo     : remove vectors outside of the image background. This allows\n\
\t           to generate figures with the same size.\n\
\t-bbo %s  : offset between the bounding box and the figure. An unit\n\
\t           should be specified as for -size.\n\
\t-nf      : no frame, do not insert a frame around the figure.\n\
\t-roi=ix,iy,x,y,width,col: print a region of interest\n\
Local options (they must be given after the targeted XFLOW filename) are:\n\
\t-scale %f    : scale factor for vector norm.\n\
\t-sample %d   : under-sampling step on vector field.\n\
\t-threshold %f: threshold low on vector norm.\n\
\t               a percentage value (eg. 10%) may be given \n\
\t-smooth      : smooth the vector field (if sample>1).\n\
\t-norma       : normalize the vector field.\n\
\t-tl %f       : alias for -threshold.\n\
\t-th %f       : threshold high for vector norm (defaut is 1O^5).\n\
\t               a percentage value (eg. 90%) may be given \n\
\t-acolor <col>: arrow color (black,blue,green,cyan,red,magneta,\n\
\t               yellow,white).\n\
\t-awidth %d   : arrow width.\n\
\t-asize <size>: predefined arrow head size, possible are: micro, tiny, small,\n\
\t               normal, large, big  and huge .\n\
\t-ahead <width> <height>: arrow head size, dimensions must be explicitly given, \n\
\t                this overrides -ahead switch.\n\
\t-astyle <style> : arrow style (0-6).\n\
\n\
The vel2fig command produces two files 'generic.fig' and 'generic.gif' (image\n\
background). The fig2dev command is then called (unless -fig is specified) and\n\
produces the final output in the specified format (-type).\n\
If -fig option is given, generic.fig can be edited using xfig.\n\
SEE ALSO: xflow\n";


char *tcolor[] = {"black", "blue", "green", "cyan", "red", "magenta",
		  "yellow", "white", NULL};

char *tsize[] = {"micro", "tiny", "small", "normal", "large", "big", "huge", NULL};

float ttsize[][2] = {
  /*
    40,90,  
    60,120,
    80,150,
    90,180,
    100,210, 
  */
  0.5,0.75, /* micro */
  1,1.5, /* tiny */
  2,3, /* small */
  3,4, /* normal */
  4,5, /* large */
  5,6, /* big */
  6,7, /* huge */
#define MANUAL    5   
  0,0
};

int getind( char **table, char *key) {
  int i;
  for( i=0; table[i]; i++)
	if( !strcasecmp(table[i], key))
	  return i;
  return 0;
}

void getnum( char *name, float *val, float *unit) {
  *val = atof(name);
  if( strstr(name, "cm") > name)
    *unit = 3./8.;
  else if( strstr(name,"in") > name )
    *unit = 1;
}

void get_moy_vois( vel2d *uv, float *u, float *v, int dimx, int sample, int smooth) {
  int i,j;
  
  if( smooth) {
    *u = *v = 0;
    for( j=0; j<sample; j++)
      for( i=0; i<sample; i++) {
	*u += uv->u;
	*v += uv->v;
	uv ++;
      }
    *u /= sample*sample;
    *v /= sample*sample;
  } else {
    *u = uv->u;
    *v = uv->v;
  }
}

float utils_normsup( vel2d *buf, int count) {
  double a, max;
  max = 0;
  while( count --) {
    a = buf->u * buf->u + buf->v * buf->v ;
    if( a > max) max = a;
    buf ++;
  }
  return sqrtf(max);
}

void getopt_roi( int roi[6]) {
  char str[128];

  if( igetopt0x( "-roi=","%s", str)) {
    char col[10];
    sscanf( str, "%d,%d,%d,%d,%d,%s", roi, roi+1, roi+2, roi+3,
	    roi+4, col);
    roi[5] = getind( tcolor, col);
  }
}

int main( int argc, char **argv) {
  char name[256],image[256]="", output[256];
  struct image *impt;
  Fort_int lfmt[9];

  float size   = 10; /* version 1 : inches */
  int   sample = 10; /* sous-echantillonage du champ de vecteur % a l'image */
  int   smooth, norma;
  float scale  = 10; /* coef multiplication amplitude du champ de vecteur */ 
  int astyle = 0;    /* style des fleches 0=creux,1=triangle,2=intermedaire */
  float lscale;
  int   frame = 1;
  float unit  = 3. / 8.; /* unit� cm */
  char  type[10] = "eps";

  int image_bg_read = 0;

  int acolor = 4;
  int awidth = 1;
  int asize = 1; /* index in arrow size array */
  int ascale;    /* scale parameter for arrow size */

  float threshold = 0, TH=1e5;
  float normsup = 1.;

  FILE *fp;
  XFLOW *xflow;
  vel2d *uv;
  int i,j;
  int w, h;
  float norm,x,y;
  int x_off, y_off;
  int x2, y2;
  int nvo , nf;
  char seuil[40];

  char jpeg_quality = 0;
  int roi[6] = {0};
  inr_init( argc, argv, version, cmd, help);


  /* Lecture des options globales */
  /* largeur et unit� (cm/in) de la figure dans le document LaTeX */
  if( igetopt1( "-size", "%s", name))
    getnum( name, &size, &unit);
  igetopt1( "-type", "%s", type);
  if( !strcmp( type, "jpeg")) 
    igetopt1( "-q", "%d", &jpeg_quality);      
  if( !igetopt1( "-iz", "%d", &frame))
    igetopt1( "-frame", "%d", &frame);  
  if( !igetopt1( "-o", "%s", output)) strcpy( output, "xflow-export");
  sprintf( output, "%s-%d", output, frame);
 
  // NoVectorOut
  nvo = igetopt0( "-nvo");
  // BoundingBoxOffset
  if( igetopt1( "-bbo", "%s", name)) {
    float s, u;
    getnum( name, &s, &u);
    x_off = y_off = s*u*1200;
  } else
    x_off = y_off = 150;
  // NoFrame
  nf = igetopt0( "-nf");

  // Roi to draw
  getopt_roi( roi);
  /* NOUVELLE INTERFACE :
     xflow2fig  [options globales]  champ_1 [options_champ_1]  champ_2 [options_champ_2] ...
  */


  if( !infileopt( name) )
    iusage_( cmd, "Error: a filename is required. Type 'vel2fig -help' for details.\n");
    
  /* Creation du fichier fig */
  {
    char name[256];
    sprintf( name, "%s.fig", output);
    fp = fopen( name, "w");
  }
  fprintf( fp, "#FIG 3.2\nLandscape\nCenter\n%s\nLetter\n",
	   (unit> .5)?"Inches":"Metric");
  fprintf( fp, "100.00\n");/* magnification (peut servir pour la taille) */
  fprintf( fp, "Single\n-2\n1200 2\n"); /* ici la resolution en ppi (1200) */

  //  while( infileopt(name)) {
  do {
    static int bg_frame = 0;
    xflow = xflow_open( name);

    if( xflow == NULL && image_bg_read ) continue;
    if( xflow == NULL) {
      image_bg_read = 1;
      impt = image_( name, "e", " ", (void*)lfmt);      
      strcpy( image, name);
    } else {
      if( image_bg_read == 0) {
	image_bg_read = 1;
	impt = xflow_get_image( xflow, lfmt);
	if( impt) strcpy( image, impt->nom);
      }
      xflow_get_lfmt( xflow, lfmt);
    }

    /* Un cadre � fond blanc plus grand pour contenir les fl�ches 
     * et avoir une taille fixe d'image de sortie */
    if( !bg_frame) {
      bg_frame = 1;
    
      fprintf( fp, "# White box as background\n");
      w = size * 1200 * unit ;
      h = w * NDIMY / NDIMX;
      fprintf( fp, "2 2 0 %d 0 7 60 -1 20 0.000 0 0 -1 0 0 5\n", 1-nf);
      fprintf( fp, "      %d %d %d %d %d %d %d %d %d %d\n",	     
	       0, 0, 
	       w + 2*x_off, 0,
	       w + 2*x_off, h + 2*y_off,
	       0 , h + 2*y_off,
	       0, 0);
    }

    /* Image : en gif pour gain de place */
    /* le 59 : la profondeur */
    if( impt) {
      fprintf( fp, "# Image %s\n", output);
      fprintf( fp, "2 5 0 2 0 -1 59 0 -1 0.000 0 0 -1 0 0 5 \n");
      fprintf( fp, "  0 %s.gif\n", output);
      w = size * 1200 * unit ;
      h = w * NDIMY / NDIMX;
      // x  y  w  y  w  h  x  h  x  y
      fprintf( fp, "  %d %d %d %d %d %d %d %d %d %d\n", 
	       x_off,    y_off,
	       x_off+w,  y_off,
	       x_off+w,  y_off+h,
	       x_off,    y_off+h,
	       x_off,    y_off);
      fermnf_( &impt);
    }
 

    /* Cadre Region of Interest */
    if( *roi) {
      fprintf( fp, "# Region of interest\n");
      int x1, y1, x2, y2;
      fprintf( fp, "2 1 0 %d %d 7 58 -1 -1 0.000 0 0 -1 0 0 5\n", roi[4], roi[5]);
      x1 = x_off + roi[0] * w / NDIMX;
      y1 = y_off + roi[1] * h / NDIMY;
      x2 = x1 + roi[2] * w / NDIMX;
      y2 = y1 + roi[3] * h / NDIMY;
      fprintf( fp, "  %d %d %d %d %d %d %d %d %d %d\n", x1, y1, x2, y1, x2, y2, x1, y2, x1, y1);
      *roi = 0;
    }

    /* Champ de vecteur */
    if( xflow) {
      /* Options locales */
      if( igetopt1( "-acolor", "%s", name))
	acolor = getind( tcolor, name);
      if( igetopt1( "-asize", "%s", name))
	asize = getind( tsize, name);  
      if( igetopt2( "-ahead", "%d", &awidth, "%d", &astyle)) {
	asize = MANUAL;
	ttsize[MANUAL][0] = awidth ;
	ttsize[MANUAL][1] = astyle;
	ascale = 1; 
      } else
	ascale = size*1200*unit / MAX(MAX(100,NDIMX),NDIMY);
      igetopt1( "-awidth", "%d", &awidth);
      igetopt1( "-astyle", "%d", &astyle);
      
      igetopt1( "-sample", "%d", &sample);  
      igetopt1( "-scale", "%f", &scale);
      smooth = igetopt0( "-smooth");      
      norma = igetopt0( "-norma");
      
      /* lecture du bon plan */
      uv = (vel2d *)malloc( NDIMX*NDIMY*sizeof(vel2d));
      xflow_seek_f    ( xflow, frame);
      xflow_read_f_v2d( xflow, 1, uv);

      normsup = norma?utils_normsup(uv,NDIMX*NDIMY):1.;
      lscale = scale * w/(NDIMX*normsup);

      /* seuil, et calcul des seuils relatifs */
      normsup = utils_normsup(uv,NDIMX*NDIMY);

      if( igetopt1( "-threshold", "%s", seuil) || 
	  igetopt1( "-tl", "%s", seuil)) {
	sscanf( seuil, "%f", &threshold);
	if( seuil[strlen(seuil)-1] == '%')
	  threshold *= (normsup/100);
      }
      if( igetopt1( "-th", "%s", seuil)) {
	sscanf( seuil, "%f", &TH);
	if( seuil[strlen(seuil)-1] == '%')
	  TH *= (normsup/100);
      }

      if( debug_) fprintf(stdout, "*** DEBUG: normsup=%f tl=%f th=%f\n",
	normsup, threshold, TH); 


      threshold*=threshold;
      TH*=TH;

      // scale *= size;
      // scale /= 1200;

      fprintf(fp,"# List of vectors\n");

      /* Ecriture des vecteurs */      
      for(j=0; j < NDIMY; j += sample)
	for(i=0; i < NDIMX; i += sample) {
	  float u,v;

	  get_moy_vois( uv + i + j*NDIMX, &u ,&v, NDIMX, sample, smooth);
	    
	  x = i*w/NDIMX;
	  y = j*h/NDIMY;
	  x2 = (int)(x + lscale*u);
	  y2 = (int)(y + lscale*v);
	  
	  norm = u*u + v*v;
	  
	  if( norm > threshold  && norm < TH &&
	      (!nvo || (x2 >= 0 && y2 >=0 && x2 < w && y2 < h))
	      ) {
	    /* ligne d'attributs */
	    fprintf( fp, "2 1 0 %d %d 7 50 0 -1 0.000 0 0 -1 1 0 2\n", awidth, acolor);
	    /* attribut fleche: 0 0 epaisseur angle longueur */
	    fprintf( fp, "  %d %d %d %d %d\n", astyle, astyle>0, awidth, 
		     (int)(ttsize[asize][0]*ascale), (int)(ttsize[asize][1]*ascale));
	    /* coordonn�es du vecteur */
	    fprintf( fp, "  %d %d %d %d\n", (int)x+x_off, (int)y+y_off, x2+x_off, y2+y_off);
	  }
	} 
      free( uv);
      xflow_close( xflow);
      xflow = NULL;
    }

    /* Analyse du prochain fichier ... */
  } while( infileopt(name));

  fclose( fp);


  if( *image) {
    /* verification si l'image du plan est bien la 
     * cr�ation si besoin */
    char tmpname[] = "/tmp/vel2figXXXXXX";

    mkstemp( tmpname);
    sprintf( name, "%s%d.gif", output, frame);
    fp = fopen( name, "r");
    if( fp == NULL || igetopt0("-doeps")) {
      /*
	fprintf( stderr, 
	"L'image gif du plan n'existe pas!\n"
	"je la cr��e � partir de la s�quence d'image ...\n");
      */

      sprintf( name, "extg %s %s -iz %d -z 1 ", image, tmpname, frame);
      system( name);

      /* Normalisez l'image de fond si besoin */
      {
	Fort_int lfmt[9];
	struct image *img = image_( tmpname, "e", "", lfmt);
	fermnf_( &img);
	if( !(TYPE==FIXE && BSIZE==1)) {
	  sprintf( name, "norma %s %s2", tmpname, tmpname);
	  system( name);
	  sprintf( name, "mv %s2 %s", tmpname, tmpname);
	  system( name);
	}
     
	/* inr2gif plante en couleur ... */
	if( NDIMV == 3) 
	  sprintf( name, "inr2png %s %s.png; convert %s.png %s.gif; rm %s.png",
		   tmpname, tmpname, tmpname, output, tmpname);
	else
	  sprintf( name, "inr2gif %s %s %s.gif", 
		   /* @FIXME : cas des couleurs -z 3 et � palette */
		   NDIMV==3?"-C":"",
		   tmpname,
		   output);
      }
      
      if(debug_)fprintf( stderr, "system: %s\n", name);
      system( name);
      sprintf( name, "rm -f %s", tmpname);
      system( name);

    }
    if(fp)  fclose( fp);
  }
  
  if( !igetopt0( "-fig")) {
    if( jpeg_quality)
      sprintf( name, "fig2dev -L %s -q %d < %s.fig > %s.%s", 
	       type, jpeg_quality, output, output, type);
    else
      sprintf( name, "fig2dev -L %s < %s.fig > %s.%s", type, output, output, type);
    if(debug_)fprintf( stderr, "system: %s\n", name);
    system( name);
    /* La sortie LaTeX ne me convinc pas */
    // sprintf( name, "fig2ps2tex %s%d.eps > %s%d.tex", generic, frame, generic, frame);
    // system( name);
  }

  return 0;
}


