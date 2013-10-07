/**
 * Fonctions diverses en vrac
 */
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "data.h"
#include "utils.h"
#include "middlebury.h"


/* Initialise les tableaux statiques utilitaires
   (table de couleurs, images, ...) */

UTILS_DATA utils_data = {NULL};

void utils_init(void) {
  int i;
  if( utils_data.mag_pal == NULL) {
    /* @FIXME: devrait etre une GdkRgbCmap */
    utils_data.mag_pal = NEW(t_col32,256);
    utils_data.div_pal = NEW(t_col32,256);
    utils_data.rot_pal = NEW(t_col32,256);

    utils_data.deg128 = NEW(unsigned char,128);
    for( i=0;i<128;utils_data.deg128[i++] = 2*i);

    makecolorwheel();

    utils_data.font = gdk_font_load("-*-courier-*-r-*-*-14-*-*-*-*-*-*-*");
  }
}

void utils_free() {
  DELETE(utils_data.mag_pal);
  DELETE(utils_data.div_pal);
  DELETE(utils_data.deg128);
  freecolorwheel();
}

/* Cherche un widget (comme le faisait Glade-2) */

GtkWidget *lookup_widget( GtkBuilder* builder, char *label) {  
  GObject * gobj = gtk_builder_get_object(builder,label);
  if( !gobj)  printf( "Warning: lookup_widget(%s) returns NULL value\n", label);
  return GTK_WIDGET(gobj);
}

/* Premier enfant d'un container */

GtkWidget *firstchild( GtkWidget *wid) {
  GList *l = gtk_container_get_children( GTK_CONTAINER(wid));
  return l->data;
}

/* Dernier enfant d'un container */

GtkWidget *lastchild( GtkWidget *wid) {
  GList *l = gtk_container_get_children( GTK_CONTAINER(wid));
  l = g_list_last(l);
  return l->data;
}

/* Gestion de la palette de couleurs */

void utils_pal_init( void) {
  int i;
  for( i=0; i<256; i++) {
    utils_data.mag_pal[i].dum = 200;
    utils_data.mag_pal[i].r = i;
    utils_data.mag_pal[i].g = i; //256-i;
    utils_data.mag_pal[i].b = i; //f%127;
  }  
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


void utils_pal_stretch( t_col32 *pal, float vmin, float vmax) {
   int i;

   if( vmin == vmax) {
     if( vmin > 0)
       for(i=0; i<256;i++) {     
	 pal[i].b = 255;
	 pal[i].r = pal[i].g = 0;
	 pal[i].dum = 0;
       } 
     else 
       for(i=0;i<256;i++) {
	 pal[i].r = 255;
	 pal[i].b = pal[i].g = 0;
	 pal[i].dum = 0;
       }
   } else if( vmin > 0 ) {  /* Palette de bleu */
     for(i=0; i<256; i++) {
       pal[i].b = i;
       pal[i].r = pal[i].g = 0;
       pal[i].dum = 0;
     }
   } else if ( vmax < 0) { /* Palette de rouge */
     for(i=0; i<256; i++) {
       pal[i].r = 255-i;
       pal[i].b = pal[i].g = 0;
       pal[i].dum = 0;
     }
   } else { /* Palette bleu/rouge */
     int i1 = 255 * vmax / (vmax-vmin);
     for( i=0; i<i1; i++) {
       pal[i].r = 255-i * 255./i1;
       pal[i].g = pal[i].b = 0;
       pal[i].dum = 0;
     }
     for( ;i<256; i++) {
       pal[i].b = (i-i1) * 255./(255-i1);
       pal[i].r = pal[i].g = 0;
       pal[i].dum = 0;
     }
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
		float *vmin, float *vmax
		) {  
  int i,j;
  float w_sc, h_sc;
  float w_offset, h_offset;
  size_t pos;
  float *pnbuf, *nbuf = NEW(float,iw*ih);
  
# define norm2(f) ((f)->u*(f)->u + (f)->v*(f)->v)

  pos = iw*ih;
  *vmin = *vmax = norm2( in);
  while( pos --) {
    *nbuf = norm2( in);
    if( *nbuf < *vmin) *vmin = *nbuf;
    if( *nbuf > *vmax) *vmax = *nbuf;
    nbuf++; in++;
  }
  nbuf -= iw*ih;
  
  w_sc = (float) iw / (float) ow;
  h_sc = (float) ih / (float) oh;
  
  w_offset = h_offset = 0;
  pnbuf = nbuf;
  
  *vmax -= *vmin;
  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {      
      *out++ = (unsigned char)
	255*sqrt( (*(pnbuf + (int)floor(w_offset)) - *vmin)/ *vmax);
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pnbuf = nbuf + iw*((int)floor(h_offset));
  }
  *vmax += *vmin;
  *vmin = sqrt(*vmin);
  *vmax = sqrt(*vmax);
  DELETE(nbuf);
} 

void utils_div( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh, 
		float *vmin, float *vmax) {  
  int i,j;
  float w_sc, h_sc;
  vel2d *pin;
  float w_offset, h_offset;
  size_t pos;

  float *bufdiv, *pbufdiv;

#define div(f,dimx) ((f+1)->u - (f-1)->u + (f+dimx)->v - (f-dimx)->v)/2

  w_sc = (float) iw / (float) ow;
  h_sc = (float) ih / (float) oh;

  pbufdiv = bufdiv = NEW( float, iw*ih);

  /* calcul div & min/max */
  pin = in;
  *vmin = *vmax = div(in+1+iw, iw);
  for(j=0; j<ih; j++) {
    for(i=0; i<iw; i++) {
      if( i>0 && i<iw-1 && j>0 && j<ih-1) {
	*pbufdiv = div(pin, iw);
	if( *pbufdiv < *vmin) *vmin = *pbufdiv;
	if( *pbufdiv > *vmax) *vmax = *pbufdiv;
      } 
      pbufdiv ++;
      pin ++;
    }
  }

  /* dupliquer les bords pour ne pas perturber 
     la normalisation */
  for(i=0; i<iw; i++) {
    bufdiv[i] = bufdiv[i+iw];
    bufdiv[i+iw*(ih-1)] = bufdiv[i+iw*(ih-2)];
  }
  for(j=0;j<ih; j++) {
    bufdiv[iw*j] = bufdiv[1+iw*j];
    bufdiv[iw-1+iw*j] = bufdiv[iw-2+iw*j];
  }

  /* FIXME */

  if(debug) fprintf( stderr, "DIV:%f %f\n", *vmin, *vmax);
  
  /* Calcul table des couleurs */
  utils_pal_stretch( utils_data.div_pal, *vmin, *vmax);  
  
  /* Normalisation 
   * FIXME si vmax == vmin, pas de normalisation  */
  *vmax -= *vmin;

  w_offset = h_offset = 0;
  pbufdiv = bufdiv;

  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {
      *out++ = (*vmax>0)?255*((*(pbufdiv + (int)floor(w_offset)) - *vmin) / *vmax):0;
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pbufdiv = bufdiv + iw*((int)floor(h_offset));
  }

  DELETE( bufdiv);
  *vmax += *vmin;
}

void utils_rot( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh,
		float *vmin, float *vmax) {  
  int i,j;
  float w_sc, h_sc;
  vel2d *pin;
  float w_offset, h_offset;
  size_t pos;
  float *bufrot, *pbufrot;

#define rot(f,dimx) ((f+1)->v - (f-1)->v - (f+dimx)->u + (f-dimx)->u)/2

  w_sc = (float) iw / (float) ow;
  h_sc = (float) ih / (float) oh;
  
  /* calcul min/max */
  pbufrot = bufrot = NEW( float, iw*ih);
  pin = in;
  *vmin = *vmax = rot(in+1+iw, iw);
  for(j=0; j<ih; j++) {
    for(i=0; i<iw; i++) {
      if( i>0 && i<iw-1 && j>0 && j<ih-1)
		*pbufrot = rot(pin, iw);
      else
	*pbufrot = 0;
      if( *pbufrot < *vmin) *vmin = *pbufrot;
      if( *pbufrot > *vmax) *vmax = *pbufrot;
      pbufrot ++;
      pin ++;
    }
  }

  if(debug) fprintf( stderr, "ROT:%f %f\n", *vmin, *vmax);
  
  /* Calcul table des couleurs */
  utils_pal_stretch( utils_data.rot_pal, *vmin, *vmax);

  *vmax -= *vmin;

  w_offset = h_offset = 0;
  pbufrot = bufrot;

  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {
      *out++ = *vmax>0?255*((*(pbufrot + (int)floor(w_offset)) - *vmin) / *vmax):0;
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pbufrot = bufrot + iw*((int)floor(h_offset));
  }
  DELETE(bufrot);
  *vmax += *vmin;
} 


/* Même chose pour la middlebury */

void utils_hsv( vel2d *in,         // image d'entrée  
		unsigned char *out,      
		int iw, int ih,
		int ow, int oh,     // nouvelle taille
		float maxmotion
		) {  
  int i,j;
  float w_sc, h_sc;
  float w_offset, h_offset;
  size_t pos;
  unsigned char *col, *pcol;
 
  if( maxmotion == 0) maxmotion=1;
  pcol = col = NEW(unsigned char, 3*ih*iw);
  /* calcul couleur */
  for(j=0; j<ih; j++)
    for(i=0; i<iw; i++) {
      computeColor( in->u/maxmotion, in->v/maxmotion, pcol);
      pcol += 3; in ++;
    }

  /* redim */
  w_sc = (float) iw / (float) ow;
  h_sc = (float) ih / (float) oh;
  w_offset = h_offset = 0;
  pcol = col;
  for(j=0; j<oh; j++) {
    for(i=0; i<ow; i++) {
      unsigned char *ppcol = pcol + 3*(int)floor(w_offset);
      *out++ = *ppcol;
      *out++ = *(ppcol+1);
      *out++ = *(ppcol+2);
      w_offset += w_sc;
    } 
    w_offset = 0;
    h_offset += h_sc;
    pcol = col + 3*iw*((int)floor(h_offset));
  }
  DELETE(col);
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

/* Ce qui suit devrait allez dans l'API.c */

void utils_mesag( XFLOW_API *api, char *fmt, ...) {
  va_list va;
  char string[256];

  va_start( va, fmt);
  vsprintf( string, fmt, va);
  va_end( va);
  
  gtk_statusbar_push( GTK_STATUSBAR(lookup_widget(api->mainwindow, 
						  "xflow_main_mesag")), 1, string);
}




/* en cours */

struct xflow_colors {
  char *name;
  GdkColor gdkcolor;
} list_colors [] = {
  "red",     { 0, 65535, 0, 0},
  "blue",    { 0, 0, 0, 65535},
  "green",   { 0, 0, 65535, 0},
  "cyan",    { 0, 0, 65535, 65535},
  "black",   { 0, 0, 0, 0},
  "magenta", { 0, 65535, 0, 65535},
  "yellow",  { 0, 65535, 65535, 0},
  "white",   { 0, 65535, 65535, 65535},
  NULL
};

void color_init( XFLOW_API *api) {
  int i;
  GdkColormap *cmap;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_main"); //_vectors_draw");

  cmap = gdk_window_get_colormap( widget->window);
  for( i=0; list_colors[i].name; gdk_color_alloc (cmap, &list_colors[i++].gdkcolor));
}

char* color_name( int id) {
  if( id>=0 && id<9) 
    return list_colors[id].name;
  return "";
}

char* color_set_by_id( XFLOW_API *api, enum color id) {
  if(id>=0 && id<9) {
    gdk_gc_set_foreground( api->gc, &list_colors[id].gdkcolor);
    return list_colors[id].name;
  }
  return NULL;
}

int color_set_by_name( XFLOW_API *api, char *name) {
  int i;
  for( i=0; list_colors[i].name != NULL; i++)
    if( !strcasecmp( name, list_colors[i].name))
      break;
  color_set_by_id( api, i);
  return i;
}

static char *tsize[] = {"small", "normal", "large", "big", NULL};

char *size_name( int id) {
  if( id>=0 && id <4)
    return tsize[id];
  return "";
}

int get_size_by_name( char *name) {
  int i;
  for( i=0; tsize[i] != NULL; i++)
    if( !strcasecmp( name, tsize[i]))
      break;
  return i;
}

static char *twidth[] = {"thin", "normal", "thick", NULL};

int get_width_by_name( char *name) {
  int i;
  for( i=0; twidth[i] != NULL; i++)
    if( !strcasecmp( name, twidth[i]))
      break;
  return i+1;
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

/* Callback function to close alert formular */
void
on_alert_close                         (GtkButton       *button,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  gtk_widget_destroy( lookup_widget( api->mainwindow, "alert_newversion"));      
}

#if 0
void check_version( XFLOW_API *api) {
  char *p, name[256];
  FILE *fp;
  int oldversion = 0;

  strcpy(name, getenv("HOME"));
  strcat(name, "/.xflowrc");
  //  sprintf( name, "%s/.xflowrc", getenv("HOME"));
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
    GtkWidget *wid = lookup_widget( api->mainwindow, "alert_newversion");
    gtk_widget_show( wid);
    
    /* update xflowrc file */
      
    fp = fopen(  name, "w");
    fprintf( fp, "%d.%d.%d\n", XFLOW_API_MAJOR, 
	     XFLOW_API_MINOR, XFLOW_API_RELEASE);
    fclose( fp);
    
  }
}

#endif


