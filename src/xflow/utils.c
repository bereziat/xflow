/**
 * Fonctions diverses en vrac
 */
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inrimage/image.h>
#include <gtk/gtk.h>
#include "data.h"
#include "support.h"
#include "utils.h"
#include "interface.h"

// extern XFLOW_API api;

/* Gestion de la palette de couleurs */

static struct { unsigned char b,g,r,dum;} mag_pal[256];

void utils_pal_init( void) {
  int i;
  for( i=0; i<256; i++) {
    mag_pal[i].dum = 200;
    mag_pal[i].r = i;
    mag_pal[i].g = i; //256-i;
    mag_pal[i].b = i; //f%127;
  }  
}

void *utils_pal_get( void) {
  return mag_pal;
}

/* lit la palette de couleur d'une image et 
 * l'adapte au format GTK */

int utils_pal_from_image( struct image *nf, unsigned char pal[][4]) {
  Fort_int *lfmt = nf->lfmt;
  unsigned char r[256],g[256],b[256];
  int start, end;
    
  if( ird_ctrgb(nf, &start, &end, r, g, b, 256) > 0) {
    int i;
    for( i=0; i< 256; i++) {
      if( i>=start && i<=end) {
	pal[i][0] = b[i];
	pal[i][1] = g[i];
	pal[i][2] = r[i];	 
	pal[i][3] = 0;
      } else
	pal[i][0] = pal[i][1] = pal[i][2] = pal[i][3] = 0;
    }
    return 1; /* Il y a une palette */
  }    
  return 0; /* Pas de palette */
}


void utils_pal_stretch( float min, float max) {
  int g0 = -min/(max-min) * 255;
  int i;
  
  for(i=0;i<=g0;i++) {
    mag_pal[i].r = 255 * (float)( g0-i) / g0;
    mag_pal[i].g = mag_pal[i].b = 0;
  }
  
  for(i=g0;i<256;i++) {
    mag_pal[i].b = 255 * (float)( i-g0) / (256-g0);
    mag_pal[i].g = mag_pal[i].r = 0;
  }  
}

/* Dessin d'une flèche */

void utils_arrow( GdkDrawable *draw,
		 GdkGC *gc,
		  int xa, int ya, int xb, int yb, float size, int style ) {  
  double ab, x, y, xc, yc, alpha;

  gdk_draw_line( draw, gc, xa, ya, xb, yb);
  ab = sqrt( (xb-xa)*(xb-xa) + (yb-ya)*(yb-ya));

  xc = xb - (float)(xb-xa)/ab * size;
  yc = yb - (float)(yb-ya)/ab * size;

  if( abs(yb-yc) > 1) {
    alpha = (xb-xc)/(yb-yc);

    /* premier point */
    x = xc + size/2;
    y = yc - alpha*size/2;

    gdk_draw_line( draw, gc, xb, yb, x, y);
    
    /* second point */
    x = xc - size/2;
    y = yc + alpha*size/2;
    gdk_draw_line( draw, gc, xb, yb, x, y);

    if( style)
      gdk_draw_line( draw, gc,  xc + size/2, yc - alpha*size/2,
		     xc - size/2, yc + alpha*size/2);
    
  } else if( abs(xb-xc) > 1) {
    alpha = (yb-yc)/(xb-xc);

    /* premier point */
    y = yc + size/2;
    x = xc - alpha*size/2;
    gdk_draw_line( draw, gc, xb, yb, x, y);
    
    /* second point */
    y = yc - size/2;
    x = xc + alpha*size/2;
    gdk_draw_line( draw, gc, xb, yb, x, y);

    if( style)
      gdk_draw_line( draw, gc,  xc + alpha*size/2, yc - size/2,
		     xc - alpha*size/2, yc + size/2);
    
  }
}

/**
 * Zoom buffer 1 octet (ou 3octet)
 */

void utils_zoom( unsigned char *in, // image d'entrée  
		 unsigned char *out,     
		 Fort_int lfmt[],   // et son codage
		 int ow, int oh     // nouvelle taille
		 ) {  
  int i,j;
  float w_sc, h_sc;
  unsigned char *pin;
  float w_offset, h_offset;

#if 0
  ow *= NDIMV;
  w_sc = (float) DIMX / (float) ow;
  h_sc = (float) NDIMY / (float) oh;

  w_offset = h_offset = 0;
  pin = in;
  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {
      *out++ = *(pin + (int)floor(w_offset));
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pin = in + DIMX*((int)floor(h_offset));
  }
#else
  int v;

  w_sc = (float) NDIMX / ow;
  h_sc = (float) NDIMY / oh;
  w_offset = h_offset = 0;
  pin = in;
  for( j=0; j<oh; j++) {
    for( i=0; i<ow; i++) {
      for( v=0; v<NDIMV; v++) 
	*out ++ = *(pin + (int)(w_offset)*NDIMV+v);
      w_offset += w_sc;
    }
    w_offset = 0;
    h_offset += h_sc;
    pin = in + DIMX*((int)(h_offset));    
  }  
#endif
} 

/* Même chose pour les images de magnétude/divergence et rotationnel */

void utils_mag( vel2d *in,         // image d'entrée  
		unsigned char *out,      
		int iw, int ih,
		int ow, int oh,     // nouvelle taille
		float normsup
		) {  
  int i,j;
  float w_sc, h_sc;
  vel2d *pin;
  float w_offset, h_offset;
  size_t pos;

  /*  
  inline float norm( vel2d *f) {
    return sqrtf(f->u*f->u + f->v*f->v);
    } */
# define norm(f) sqrtf((f)->u*(f)->u + (f)->v*(f)->v)
  
  w_sc = (float) iw / (float) ow;
  h_sc = (float) ih / (float) oh;
  
  w_offset = h_offset = 0;
  pin = in;
  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {
      *out++ = (unsigned char)(255*norm(pin + (int)floor(w_offset))/normsup);
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pin = in + iw*((int)floor(h_offset));
  }
} 

void utils_div( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh) {  
  int i,j;
  float w_sc, h_sc;
  vel2d *pin;
  float w_offset, h_offset;
  size_t pos;

  float min, max;
  float *bufdiv, *pbufdiv;

  /*
  inline float div( vel2d *f, int dimx) {
    return ((f+1)->u - (f-1)->u + (f+dimx)->v - (f-dimx)->v)/2;
    }*/
#define div(f,dimx) ((f+1)->u - (f-1)->u + (f+dimx)->v - (f-dimx)->v)/2

  w_sc = (float) iw / (float) ow;
  h_sc = (float) ih / (float) oh;

  pbufdiv = bufdiv = (float*)malloc(sizeof(float)*iw*ih);
  pin = in;
  min = max = div(in+1+iw, iw);
  for(j=0; j<ih; j++) {
    for(i=0; i<iw; i++) {
      if( i>0 && i<iw-1 && j>0 && j<ih-1)
	*pbufdiv = div(pin, iw);
      else
	*pbufdiv = 0;
      if( *pbufdiv < min) min = *pbufdiv;
      if( *pbufdiv > max) max = *pbufdiv;
      pbufdiv ++;
      pin ++;
    }
  }

  if(debug) fprintf( stderr, "DIV:%f %f\n", min, max);
  
  /* Calcul table des couleurs */
  utils_pal_stretch( min, max);
 
  /*
  if( fabsf(min) < fabsf(max) ) 
    min = -fabsf(max);
  else
    max = -fabsf(min);
  */
  
  max -= min;

  w_offset = h_offset = 0;
  pbufdiv = bufdiv;

  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {
      *out++ = 255*(*(pbufdiv + (int)floor(w_offset)) - min / max);
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pbufdiv = bufdiv + iw*((int)floor(h_offset));
  }
} 

void utils_rot( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh) {  
  int i,j;
  float w_sc, h_sc;
  vel2d *pin;
  float w_offset, h_offset;
  size_t pos;
  float min, max;
  float *bufrot, *pbufrot;

  /*
  inline float rot( vel2d *f, int dimx) {
    return ((f+1)->v - (f-1)->v - (f+dimx)->u + (f-dimx)->u)/2;
    }*/
#define rot(f,dimx) ((f+1)->v - (f-1)->v - (f+dimx)->u + (f-dimx)->u)/2

  w_sc = (float) iw / (float) ow;
  h_sc = (float) ih / (float) oh;
  
  pbufrot = bufrot = (float*)malloc(sizeof(float)*iw*ih);
  pin = in;
  min = max = rot(in+1+iw, iw);
  for(j=0; j<ih; j++) {
    for(i=0; i<iw; i++) {
      if( i>0 && i<iw-1 && j>0 && j<ih-1)
		*pbufrot = rot(pin, iw);
      else
	*pbufrot = 0;
      if( *pbufrot < min) min = *pbufrot;
      if( *pbufrot > max) max = *pbufrot;
      pbufrot ++;
      pin ++;
    }
  }

  if(debug) fprintf( stderr, "ROT:%f %f\n", min, max);
  
  /* Calcul table des couleurs */
  utils_pal_stretch( min, max);

  /*
  if( fabsf(min) < fabsf(max) ) 
    min = -fabsf(max);
  else
    max = -fabsf(min);
  */
  
  max -= min;

  w_offset = h_offset = 0;
  pbufrot = bufrot;

  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {
      *out++ = 255*(*(pbufrot + (int)floor(w_offset)) - min / max);
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pbufrot = bufrot + iw*((int)floor(h_offset));
  }
} 

void utils_normsup( vel2d *buf, int count, float *max) {
  float a;
  
  *max *= *max;
  while( count --) {
    a = buf->u * buf->u + buf->v * buf->v ;
    if( a > *max) *max = a;
    buf ++;
  }
  *max = sqrtf(*max);
}

void utils_mesag( XFLOW_API *api, char *fmt, ...) {
  va_list va;
  char string[256];

  va_start( va, fmt);
  vsprintf( string, fmt, va);
  va_end( va);
  
  gtk_statusbar_push( GTK_STATUSBAR(lookup_widget(api->mainwindow, 
						  "xflow_message")), 1, string);
}

/* en cours */
struct xflow_colors list_colors [] = {
  "default", {0,0,0,0},
  "black",   {0,0,0,0},
  "green",   { 0, 0, 0, 0},
  "blue",    { 0, 0, 0, 65535},
  "green",   { 0, 0, 65535, 0},
  "cyan",    { 0, 0, 65535, 65535},
  "red",     { 0, 65535, 0, 0},
  "magenta", { 0, 65535, 0, 65535},
  "yellow",  { 0, 65535, 65535, 0},
  "white",   { 0, 65535, 65535, 65535},
  NULL
};

static GdkColor black    = { 0, 0, 0, 0};
static GdkColor blue     = { 0, 0, 0, 65535};
static GdkColor green    = { 0, 0, 65535, 0};
static GdkColor cyan     = { 0, 0, 65535, 65535};
static GdkColor red      = { 0, 65535, 0, 0};
static GdkColor magenta  = { 0, 65535, 0, 65535};
static GdkColor yellow   = { 0, 65535, 65535, 0};
static GdkColor white    = { 0, 65535, 65535, 65535};
 
static GdkColor *datacolor[] = { &black, &blue, &green, &cyan, &red, &magenta,
			  &yellow, &white, NULL };

void color_init( XFLOW_API *api) {
  int i;
  GdkColormap *cmap;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_drawing");

  cmap = gdk_window_get_colormap( widget->window);
  for( i=0; datacolor[i]; gdk_color_alloc (cmap, datacolor[i++]));
  // for( i=1; list_color[i].name; gdk_color_alloc (cmap, &list_color[i++].gdkcolor));
}

void color_set( XFLOW_API *api, int index) {
  gdk_gc_set_foreground( api->gc, datacolor[index]);
}

/* en cours */

void color_set_by_id( XFLOW_API *api, enum color id) {
  if( id > DEFAULT)
    gdk_gc_set_foreground( api->gc, &list_colors[id].gdkcolor);
  else
    gdk_gc_set_foreground( api->gc, &list_colors[api->arrowcolor+1].gdkcolor);
}

void color_set_by_name( XFLOW_API *api, char *name) {
  int i;
  for( i=0; list_colors[i].name != NULL; i++)
    if( !strcmp( name, list_colors[i].name))
      break;
  color_set_by_id( api, i);
}

void size_set_by_name();
void size_set_by_id();

static 
char *tcolor[] = {"black", "blue", "green", "cyan", "red", 
		  "magenta", "yellow", "white", NULL};
char *color_name( XFLOW_API *api, int color) {
  if( color == -1) color = api->arrowcolor;
  return tcolor[color];
}

static char *tsize[] = {"small", "normal", "large", "big", NULL};
char *size_name( XFLOW_API *api, int size) {
  if( size == -1) size = api->arrowsize;
  return tsize[size];
}


/* comme basename mais enleve les extensions */ 

char* utils_basename( char *p) {
  char *s;
  basename( p);
  
  if( (s=strstr( p, ".gz")) &&
      s[strlen(".gz")] == '\0') 
    *s = '\0';
  s = strrchr( p, '.');
  if( s) *s = '\0';
  return p;
}


void utils_controls_snap( XFLOW_API *api, XFLOW_DATA *pd) {
  
}

void utils_controls_read() {}

void check_version( void) {
  char *p, name[256];
  FILE *fp;
  int oldversion = 0;

  sprintf( name, "%s/.xflowrc", getenv("HOME"));
  fp = fopen( name, "r");
  if( fp) {
    int maj, min, rel;

    fscanf( fp, "%d.%d.%d", &maj, &min, &rel);
    fclose( fp);
    if( maj < XFLOW_API_MAJOR ||
	min < XFLOW_API_MINOR ||
	rel < XFLOW_API_RELEASE )
      oldversion = 1;    
  } 

  if( !fp || oldversion) {
    oldversion = 1;
    GtkWidget *wid = create_alert_newversion ();
    gtk_widget_show( wid);
    
    /* update xflowrc file */
      
    fp = fopen(  name, "w");
    fprintf( fp, "%d.%d.%d\n", XFLOW_API_MAJOR, 
	     XFLOW_API_MINOR, XFLOW_API_RELEASE);
    fclose( fp);
    
  }
}

/* Callback function to close alert formular */
void
on_alert_close                         (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy( lookup_widget( GTK_WIDGET(button), "alert_newversion"));      
}
