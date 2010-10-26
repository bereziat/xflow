#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <math.h>
#include <string.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "data.h"
#include "utils.h"

extern XFLOW_API api;

// NOM A CHANGER -> dans utils.c

void alloc_for_resize ( XFLOW_API *api, int w, int h) 
{
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_drawing");
  XFLOW_DATA *pd;  

  api->wwin = w; api->hwin = h; 
  for( pd = api->data; pd; pd = pd -> next) {
    if( pd->type == DATA_IMAGE) {
      Fort_int lfmt[9];
      
      memcpy( lfmt, pd->data.image.file->lfmt, 9*sizeof(Fort_int));
      TYPE = FIXE; BSIZE = 1;

      if(debug) fprintf( stderr, "resize Image: (%d,%d)->(%d,%d)\n",
			 NDIMX, NDIMY, w, h);
            
      // REALLOCATION DU TAMPON
      DELETE( pd->data.image.buf);
      pd->data.image.buf = NEW(unsigned char, api->wwin*api->hwin*NDIMV);
      
      // ZOOM DEPUIS LE TAMPON DE LECTURE
      if( api->wwin == NDIMX && api->hwin == NDIMY)	  
	memcpy( pd->data.image.buf, pd->data.image.read, api->wwin*api->hwin*NDIMV);
      else 
	utils_zoom( pd->data.image.read, pd->data.image.buf, 
		    lfmt, api->wwin, api->hwin);
      
    } else if( pd->type == DATA_XFLOW) {
      int w, h, d;
      
      xflow_get_dims( pd->data.xflow.file, &w, &h, &d);
      
      if( pd->data.xflow.magbuf) {
	if(debug) fputs( "resize mag buffer", stderr);
	DELETE(pd->data.xflow.magbuf); 
	pd->data.xflow.magbuf = NEW(unsigned char, api->wwin*api->hwin);
	utils_mag( pd->data.xflow.buf, pd->data.xflow.magbuf, 
		   w, h, api->wwin, api->hwin, pd->data.xflow.normsup);
      }
      if( pd->data.xflow.divbuf) {
	if(debug) fputs( "resize div buffer", stderr);
	DELETE(pd->data.xflow.divbuf); 
	pd->data.xflow.divbuf = NEW(unsigned char, api->wwin*api->hwin);
	utils_div( pd->data.xflow.buf, pd->data.xflow.divbuf, w, h, api->wwin, api->hwin);
      }
      if( pd->data.xflow.rotbuf) {
	if(debug) fputs( "resize rot buffer", stderr);
	DELETE(pd->data.xflow.rotbuf); 
	pd->data.xflow.rotbuf = NEW(unsigned char, api->wwin*api->hwin);
	utils_rot( pd->data.xflow.buf, pd->data.xflow.rotbuf, w, h, api->wwin, api->hwin);
      }
    }
  }
}

static void get_moy_vois( vel2d *uv, float *u, float *v, int dimx, int sample, int smooth) {
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

static void draw_vectors( GtkWidget *widget, XFLOW_DATA *pd) {
  int i,j;
  float w_sc, h_sc;
  int w_win = api.wwin;
  int h_win = api.hwin;
  int w_img, h_img, dum;
  int size;
  float scale;
  float tl, th;

  /* Couleur des vecteurs */
  color_set( &api, ( pd->data.xflow.arrowcolor == -1) ? 
	     api.arrowcolor : pd->data.xflow.arrowcolor);
  /* Taille */
  size =  (pd->data.xflow.arrowsize == -1) ? api.arrowsize : pd->data.xflow.arrowsize;

  /* Epaisseur */
  gdk_gc_set_line_attributes( api.gc, pd->data.xflow.arrowwidth, GDK_LINE_SOLID, GDK_CAP_NOT_LAST,  GDK_JOIN_MITER);
  
  xflow_get_dims( pd->data.xflow.file, &w_img, &h_img, &dum); 
	  
  w_sc = (float)w_win/(float)w_img;
  h_sc = (float)h_win/(float)h_img;

  tl = api . thresh * api . thresh;
  th = api . thresh_high * api . thresh_high;

  scale = (float) api . scale / (pd->data.xflow.norma?pd->data.xflow.normsup:1.);

  for( j = 0; j < h_img; j += api.sample)
    for( i = 0; i < w_img; i += api.sample) { 
      vel2d *uv = pd->data.xflow.buf + i + j * w_img;
      float u,v,n;

      get_moy_vois( uv, &u, &v, w_img, api.sample, pd->data.xflow.smooth);
      n = u*u + v*v;
      if( n > tl && n < th ) {
	utils_arrow(  widget->window, api . gc,			
		      (int)((i+0.5)*w_sc), 
		      (int)((j+0.5)*h_sc), 
		      (int)((i+0.5 + scale * u)*w_sc), 
		      (int)((j+0.5 + scale * v)*h_sc),
		      size + 3,
		      pd->data.xflow.arrowstyle);

      }      
    }      
  
  
}

gboolean
on_xflow_drawing_expose_event          (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  XFLOW_DATA *pd;
  int w_win = api.wwin; // widget->allocation.width;
  int h_win = api.hwin; // widget->allocation.height;
  int i,j;
  int w_img, h_img, dum;
  float w_sc, h_sc;
  Fort_int *lfmt;
  int background = 0;

  for( pd = api.data; pd; pd  = pd->next) {    
    switch( pd->type) {
    case DATA_IMAGE:
      /* Seule, la première image de fond est affichée */
      if( background) continue;
      background = 1;
      lfmt = pd->data.image.file->lfmt;
      switch( pd->data.image.display) {	
      case SCRN_RGB:
	gdk_draw_rgb_image( widget->window, 
			    widget->style->fg_gc[GTK_STATE_NORMAL],
			    0, 0, api.wwin, api.hwin,
			    GDK_RGB_DITHER_NORMAL, 
			    pd->data.image.buf,
			    3*api.wwin);
	break;
      case SCRN_INDEXED:  /* TODO : ajouter la palette */
	gdk_draw_indexed_image( widget->window, 
				widget->style->fg_gc[GTK_STATE_NORMAL],
				0, 0, api.wwin, api.hwin,
				GDK_RGB_DITHER_NORMAL, 
				pd->data.image.buf,
				api.wwin,
				pd->data.image.pal
				);
	break;
      case SCRN_GRAY:
      case SCRN_FLOAT:
	gdk_draw_gray_image( widget->window, 
			     widget->style->fg_gc[GTK_STATE_NORMAL],
			     0, 0, api.wwin, api.hwin,
			     GDK_RGB_DITHER_NORMAL, 
			     pd->data.image.buf,
			     api.wwin);
	break;
      }
      break;
    case DATA_XFLOW:
      if( pd->data.xflow.hide == 1) continue;
      draw_vectors( widget, pd);
      break;      
    }

  }
  return FALSE;
}

gboolean
on_xflow_mag_expose_event              (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  XFLOW_DATA *pd;

  for( pd = api.data; pd; pd  = pd->next) {    
    if( pd->type == DATA_XFLOW && pd->data.xflow.hide == 0 &&
	pd->data.xflow.magbuf
	) {
      gdk_draw_gray_image( widget->window, 
			      widget->style->fg_gc[GTK_STATE_NORMAL],
			      0, 0, api.wwin, api.hwin,
			      GDK_RGB_DITHER_NORMAL, 
			      pd->data.xflow.magbuf,
			      api.wwin);
			      // ,
			      //(GdkRgbCmap*) utils_pal_get() );
      draw_vectors( widget, pd);
      // break;
    }
   }
  return FALSE;
}

gboolean
on_xflow_div_expose_event              (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  XFLOW_DATA *pd;

  for( pd = api.data; pd; pd  = pd->next) {    
    if( pd->type == DATA_XFLOW && pd->data.xflow.hide == 0 &&
	pd->data.xflow.divbuf
	) {
      gdk_draw_indexed_image( widget->window, 
			      widget->style->fg_gc[GTK_STATE_NORMAL],
			      0, 0, api.wwin, api.hwin,
			      GDK_RGB_DITHER_NORMAL, 
			      pd->data.xflow.divbuf,
			      api.wwin,
			      (GdkRgbCmap*) utils_pal_get() );
      draw_vectors( widget, pd);
      // break;
    }
   }
  return FALSE;
}

gboolean
on_xflow_rot_expose_event              (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  XFLOW_DATA *pd;

  for( pd = api.data; pd; pd  = pd->next) {    
    if( pd->type == DATA_XFLOW && pd->data.xflow.hide == 0 &&
	pd->data.xflow.rotbuf ) {
      gdk_draw_indexed_image( widget->window, 
			      widget->style->fg_gc[GTK_STATE_NORMAL],
			      0, 0, api.wwin, api.hwin,
			      GDK_RGB_DITHER_NORMAL, 
			      pd->data.xflow.rotbuf,
			      api.wwin,
			      (GdkRgbCmap*) utils_pal_get() );
      draw_vectors( widget, pd);
      // break;
    }
   }
  return FALSE;
}


////////////////////// CALLBACKS NOTIFICATION & PRESS BUTTON /////////////////////////////


static
void xflow_field_notify( int ev_x, int ev_y, char notify[]) {
  int x, y;
  float u=0, v=0;
  XFLOW_DATA *pd;
  int pos = 0;

  *notify = '\0';

  for( pd = api.data; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {     
      int w_img, h_img, dum;
      size_t count;
      
      xflow_get_dims( pd->data.xflow.file, &w_img, &h_img, &dum);      
      x = (int)(ev_x * (float)w_img/(float)api.wwin);
      y = (int)(ev_y * (float)h_img/(float)api.hwin);

      if (pos == 0) {
	sprintf( notify, "-x %d -y %d ", x+1, y+1);
	pos = 1;
      }
      if( x<0 || x >= w_img || y<0 || y>=h_img) {
	sprintf( notify, "%s <out of range> ", notify);
      } else {
	count = x + y*w_img;
	u = pd->data.xflow.buf[count].u;
	v = pd->data.xflow.buf[count].v;      
	sprintf( notify, "%s -u %f -v %f ", notify, u, v);
      }
    }
  }
}


gboolean
on_xflow_drawing_motion_notify_event   (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
  char notify[256];
  
  xflow_field_notify( event->x, event->y, notify);
  utils_mesag( &api, notify);  
  return FALSE;
}

gboolean
on_xflow_drawing_button_press_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  char notify[256];

  if( event->button == 3) {
    xflow_field_notify( event->x, event->y, notify);
    fprintf( stdout, "%s\n", notify);
  }
  return FALSE;
}

/*******/

static void xflow_mag_notify(int ev_x, int ev_y, char mes[]) {
  int x, y, pos=0;
  float m;
  XFLOW_DATA *pd;

  for( pd = api.data; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {
     
      int w_img, h_img, dum;
      size_t count;

      xflow_get_dims( pd->data.xflow.file, &w_img, &h_img, &dum);

      x = (int)(ev_x * (float)w_img/(float)api.wwin);
      y = (int)(ev_y * (float)h_img/(float)api.hwin);
      if (pos == 0) {
	sprintf( mes, "-x %d -y %d ", x+1, y+1);
	pos = 1;
      }
      
      if( x<0 || x >= w_img || y<0 || y>=h_img) {
	sprintf( mes, "%s <out of range> ", mes);
      } else {
	count = x + y*w_img;
	m = pd->data.xflow.buf[count].u * pd->data.xflow.buf[count].u 
	  + pd->data.xflow.buf[count].v * pd->data.xflow.buf[count].v ;     
	sprintf( mes, "%s -mag %f ", mes, sqrt(m));
      }
    }
  }
}

gboolean
on_xflow_mag_motion_notify_event   (GtkWidget       *widget,
				    GdkEventMotion  *event,
				    gpointer         user_data)
{
  char notify[256];
  
  xflow_mag_notify( event->x, event->y, notify);
  utils_mesag( &api, notify);  
  return FALSE;
}

gboolean
on_xflow_mag_button_press_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  char notify[256];

  if( event->button == 3) {
    xflow_mag_notify( event->x, event->y, notify);
    fprintf( stdout, "%s\n", notify);
  }
  return FALSE;
}

/*************************/
static void xflow_div_notify( int ev_x, int ev_y, char mes[]) {
  int x, y;
  int pos = 0;
  float d=0;
  XFLOW_DATA *pd;
  vel2d *f;
  
  inline float div( vel2d *f, int dimx) {
    return ((f+1)->u - (f-1)->u + (f+dimx)->v - (f-dimx)->v)/2;
  }

  for( pd = api.data; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {
     
      int w_img, h_img, dum;
      size_t count;

      xflow_get_dims( pd->data.xflow.file, &w_img, &h_img, &dum);

      x = (int)(ev_x * (float)w_img/(float)api.wwin);
      y = (int)(ev_y * (float)h_img/(float)api.hwin);
      
      if (pos == 0) {
	sprintf( mes, "-x %d -y %d ", x+1, y+1);
	pos = 1;
      }

      if( x<0 || x >= w_img || y<0 || y>=h_img) {
	sprintf( mes, "%s <out of range> ", mes);
      } else {
	count = x + y*w_img;
	f = pd->data.xflow.buf + count;
	if( x > 0 && y > 0 && x < w_img-1 && y <h_img-1) 
	  sprintf( mes, "%s -div %f ", mes, div(f, w_img));
	else
	  sprintf( mes, "%s -div <out of range> ", mes);
      }
    }
  }
}

gboolean
on_xflow_div_motion_notify_event   (GtkWidget       *widget,
				    GdkEventMotion  *event,
				    gpointer         user_data)
{
  char notify[256];

  xflow_div_notify( event->x, event->y, notify);
  utils_mesag( &api, notify);   
  return FALSE;
}

gboolean
on_xflow_div_button_press_event    (GtkWidget       *widget,
				    GdkEventButton  *event,
				    gpointer         user_data)
{
  char notify[256];

  if( event->button == 3) {
    xflow_div_notify( event->x, event->y, notify);
    fprintf( stdout, "%s\n", notify);
  }
  return FALSE;
}

/************/

static void xflow_rot_notify( int ev_x, int ev_y, char mes[])
{
  int x, y;
  int pos = 0;
  float d=0;
  XFLOW_DATA *pd;
  vel2d *f;
  
  inline float rot( vel2d *f, int dimx) {
    return ((f+1)->v - (f-1)->v - (f+dimx)->u + (f-dimx)->u)/2;
  }

  for( pd = api.data; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {
     
      int w_img, h_img, dum;
      size_t count;

      xflow_get_dims( pd->data.xflow.file, &w_img, &h_img, &dum);

      x = (int)(ev_x * (float)w_img/(float)api.wwin);
      y = (int)(ev_y * (float)h_img/(float)api.hwin);
      
      if (pos == 0) {
	sprintf( mes, "-x %d -y %d ", x+1, y+1);
	pos = 1;
      }
    
      if( x<0 || x >= w_img || y<0 || y>=h_img) {
	sprintf( mes, "%s <out of range> ", mes);
      } else {
	count = x + y*w_img;
	f = pd->data.xflow.buf + count;
	if( x > 0 && y > 0 && x < w_img-1 && y <h_img-1) 
	  sprintf( mes, "%s -rot %f ", mes, rot(f, w_img));
	else
	  sprintf( mes, "%s -rot <out of range> ", mes);
      }
    }
  }
}

gboolean
on_xflow_rot_motion_notify_event   (GtkWidget       *widget,
				    GdkEventMotion  *event,
				    gpointer         user_data)
{
  char notify[256];
  xflow_rot_notify( event->x, event->y, notify);
  utils_mesag( &api, notify);   
  return FALSE;
}

gboolean
on_xflow_rot_button_press_event    (GtkWidget       *widget,
				    GdkEventButton  *event,
				    gpointer         user_data)
{
  char notify[256];

  if( event->button == 3) {
    xflow_rot_notify( event->x, event->y, notify);
    fprintf( stdout, "%s\n", notify);
  }
  return FALSE;
}

// @TODO : CHAMPS MULITPLE NON GERES

void
on_xflow_notebook_switch_page          (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data)
{
  XFLOW_DATA *pd;
  int w,h,d;
  int nw, nh;
  GtkWidget *widget;
  
  void get_size( GtkWidget *widget, char *name, int *w, int *h) {
    widget = lookup_widget( widget, name);
    *w = widget->allocation.width;
    *h = widget->allocation.height;
  }

  if(debug) fprintf( stderr, "Selection onglet %d\n", page_num);
  
  /* Nettoyage des tampons */
  for( pd = api.data; pd; pd = pd -> next) {
    if( pd->type == DATA_XFLOW) {
      if( pd->data.xflow.magbuf) {
	DELETE(pd->data.xflow.magbuf);
	pd->data.xflow.magbuf = NULL;
      }
      if( pd->data.xflow.divbuf) {
	  DELETE(pd->data.xflow.divbuf);
	  pd->data.xflow.divbuf = NULL;
      }
      if( pd->data.xflow.rotbuf) {
	DELETE(pd->data.xflow.rotbuf);
	pd->data.xflow.rotbuf = NULL;
      }
    }
  }  
  nw = api.wwin; 
  nh = api.hwin;; 
  switch( page_num) {
  case 0:
    /* Sélection de l'onglet F */
    break;
  case 1:
    /* Sélection de l'onglet M */
    for( pd = api.data; pd; pd = pd -> next) {
      if( pd->type == DATA_XFLOW) {	
	xflow_get_dims( pd->data.xflow.file, &w, &h, &d);	
	//	get_size( GTK_WIDGET(notebook), "xflow_mag", &nw, &nh);
	if(debug) fprintf( stderr, "mag: (%d,%d)->(%d,%d)\n", w,h,nw,nh);
	pd->data.xflow.magbuf = NEW(unsigned char, nw*nh);
	utils_mag( pd->data.xflow.buf, pd->data.xflow.magbuf, 
		   w, h, nw, nh, pd->data.xflow.normsup);
	break;
      }
    }
    break;
  case 2:
    /* Sélection de l'onglet D */
    for( pd = api.data; pd; pd = pd -> next) {
      if( pd->type == DATA_XFLOW) { 
	xflow_get_dims( pd->data.xflow.file, &w, &h, &d);
	//	get_size( GTK_WIDGET(notebook), "xflow_div", &nw, &nh);
	pd->data.xflow.divbuf = NEW(unsigned char, nw*nh);
	utils_div( pd->data.xflow.buf, pd->data.xflow.divbuf, 
		   w, h, nw, nh);
	break;
      }
    }
    break;
  case 3:
    /* Sélection de l'onglet R */
    for( pd = api.data; pd; pd = pd -> next) {
      if( pd->type == DATA_XFLOW) {      
	xflow_get_dims( pd->data.xflow.file, &w, &h, &d);
	//	get_size( GTK_WIDGET(notebook), "xflow_rot", &nw, &nh);
	pd->data.xflow.rotbuf = NEW(unsigned char, nw*nh);
	utils_rot( pd->data.xflow.buf, pd->data.xflow.rotbuf, 
		   w, h, nw, nh);
	break;
      }
    }
    break;
  }
}


void
on_menu_field_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(menuitem), "xflow_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 0);
}


void
on_menu_mag_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(menuitem), "xflow_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 1);
}


void
on_menu_div_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(menuitem), "xflow_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 2);
}


void
on_menu_rot_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(menuitem), "xflow_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 3);
}


///////////////// CALLBACK DES SLIDERS  ////////////////////

void
on_xflow_sample_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  
  api.sample = (int)GtkAdj->value;
  xflow_api_refresh_drawing_areas (&api);
}



void
on_xflow_scale_value_changed           (GtkRange        *range,
                                        gpointer         user_data)
{
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  
  api.scale = (float)GtkAdj->value;
  xflow_api_refresh_drawing_areas (&api);
}


void
on_xflow_thresh_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);

  api.thresh = (float)GtkAdj->value;
  xflow_api_refresh_drawing_areas (&api);
}

void
on_xflow_thresh_high_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);

  api.thresh_high = (float)GtkAdj->value;
  xflow_api_refresh_drawing_areas (&api);
}

void
on_xflow_zoom_value_changed          (GtkRange        *range,
				      gpointer         user_data)
{
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  int nw = (float)api.wimg * GtkAdj->value;
  int nh = (float)api.himg * GtkAdj->value;

  // AFAIRE La requete doit se faire faire l'onglet actif
  // AFAIRE Le changement d'onglet doit calculer la bonne taille

  gtk_widget_set_size_request ( lookup_widget( GTK_WIDGET(range), "xflow_drawing"), nw, nh);
  gtk_widget_set_size_request ( lookup_widget( GTK_WIDGET(range), "xflow_mag"), nw, nh);
  gtk_widget_set_size_request ( lookup_widget( GTK_WIDGET(range), "xflow_div"), nw, nh);
  gtk_widget_set_size_request ( lookup_widget( GTK_WIDGET(range), "xflow_rot"), nw, nh);
  alloc_for_resize( &api, nw,nh);
  api.zoom = GtkAdj->value;
  xflow_api_refresh_drawing_areas (&api);
 }


void
on_zscroll_value_changed               (GtkRange        *range,
                                        gpointer         user_data)
{
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  char notify[256];
  XFLOW_DATA *pd;

  data_read( &api, (int)GtkAdj->value);
  xflow_api_update_widget( &api);
  xflow_api_refresh_drawing_areas (&api);
  
  sprintf ( notify, "Frame %d read. ", (int)GtkAdj->value);
  for( pd = api.data; pd; pd = pd->next)
    if( pd->type == DATA_XFLOW)
      sprintf( notify, "%s -n %f ", notify, pd->data.xflow.normsup);
  
  utils_mesag( &api, notify);
}

void
on_menu_next_frame_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(menuitem), "xflow_zscroll");
  GtkAdjustment *adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  int value = (int) adj->value;

  value = (value+1) % (int)adj->upper;

  gtk_adjustment_set_value( adj, value);
  on_zscroll_value_changed ( GTK_RANGE(widget), user_data);  
}


void
on_menu_prev_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(menuitem), "xflow_zscroll");
  GtkAdjustment *adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  int value = (int) adj->value;
  
  value = (value > 1)?(value-1):1;  
  gtk_adjustment_set_value( adj, value);
  on_zscroll_value_changed ( GTK_RANGE(widget), user_data);  
}



///////////////// CALLBACKS DES MENUS /////////////////////


void
on_color_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_black_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = BLACK);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_blue_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = BLUE);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_green_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = GREEN);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_cyan_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = CYAN);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_red_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = RED);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_magenta_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = MAGENTA);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_yellow_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = YELLOW);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_white_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  color_set( &api, api.arrowcolor = WHITE);
  xflow_api_refresh_drawing_areas (&api);
}


void
on_size1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_small_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  api.arrowsize = 0;
  xflow_api_refresh_drawing_areas (&api);
}


void
on_normal_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  api.arrowsize = 1;
  xflow_api_refresh_drawing_areas (&api);
}


void
on_large_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  api.arrowsize = 2;
  xflow_api_refresh_drawing_areas (&api);
}


void
on_big_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  api.arrowsize = 3;
  xflow_api_refresh_drawing_areas (&api);
}



///////////////// CALLBACK MENU RESTANT ////////////////////


void
on_ouvrir1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_quitter1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_destroy( api.mainwindow);
}


void
on_display1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_xflow_mainwindow_destroy            (GtkObject       *object,
                                        gpointer         user_data)
{
  data_free( &api);
  gtk_main_quit();
}



void
on_xflow_normalize_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  XFLOW_DATA *cdata;
  int active = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(button));
  for ( cdata=api.data; cdata; cdata=cdata->next)
    if( cdata->type == DATA_XFLOW)
      cdata->data.xflow.norma = active>0;
  xflow_api_refresh_drawing_areas (&api);
}


void
on_xflow_smooth_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  XFLOW_DATA *cdata;
  int active = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(button));
  for ( cdata=api.data; cdata; cdata=cdata->next)
    if( cdata->type == DATA_XFLOW)
      cdata->data.xflow.smooth = active>0;
  xflow_api_refresh_drawing_areas (&api);
}

