#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <math.h>
#include <string.h>

#include "data.h"
#include "utils.h"
#include "trajs.h"


void
on_xflow_main_destroy            (GtkObject       *object,
				  gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  xflow_api_delete( api);
  gtk_main_quit();
}

void
on_xflow_main_zscroll_value_changed    (GtkRange        *range,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  char notify[256];
  XFLOW_DATA *pd;

  data_read( api, (int)GtkAdj->value+1);
  xflow_api_update_widget( api);
  xflow_api_refresh_drawing_areas (api);
  
  sprintf ( notify, "Frame %d read. ", (int)GtkAdj->value+1);
  for( pd = api->data; pd; pd = pd->next)
    if( pd->type == DATA_XFLOW)
      sprintf( notify, "%s -n %f ", notify, pd->data.xflow.normsup);
  
  /* mettre a jour position trajectoire */
  trajs_update_list_store( api);
  utils_mesag( api, notify);
}

/* @FIXME: Changer le nom de cette fonction et la déplacer dans utils.c ou data.c */

void alloc_for_resize ( XFLOW_API *api, int w, int h) 
{
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
	if(debug) fputs( "resize mag buffer\n", stderr);
	DELETE(pd->data.xflow.magbuf); 
	pd->data.xflow.magbuf = NEW(unsigned char, api->wwin*api->hwin);
	utils_mag( pd->data.xflow.buf, pd->data.xflow.magbuf, w, h, api->wwin, api->hwin, 
		   &pd->data.xflow.magmin, &pd->data.xflow.magmax);
      }
      if( pd->data.xflow.divbuf) {
	if(debug) fputs( "resize div buffer\n", stderr);
	DELETE(pd->data.xflow.divbuf); 
	pd->data.xflow.divbuf = NEW(unsigned char, api->wwin*api->hwin);
	utils_div( pd->data.xflow.buf, pd->data.xflow.divbuf, w, h, api->wwin, api->hwin,
		   &pd->data.xflow.divmin, &pd->data.xflow.divmax);
      }
      if( pd->data.xflow.rotbuf) {
	if(debug) fputs( "resize rot buffer\n", stderr);
	DELETE(pd->data.xflow.rotbuf); 
	pd->data.xflow.rotbuf = NEW(unsigned char, api->wwin*api->hwin);
	utils_rot( pd->data.xflow.buf, pd->data.xflow.rotbuf, w, h, api->wwin, api->hwin,
		   &pd->data.xflow.rotmin, &pd->data.xflow.rotmax);
      }
      if( pd->data.xflow.hsvbuf) {
	if(debug) fputs( "resize hsv buffer\n", stderr);
	DELETE(pd->data.xflow.hsvbuf); 
	pd->data.xflow.hsvbuf = NEW(unsigned char, 3*api->wwin*api->hwin);
	utils_hsv( pd->data.xflow.buf, pd->data.xflow.hsvbuf, w, h, api->wwin, api->hwin,
		   pd->data.xflow.hsv_sat);
      }
    }
  }
}

/*
 * Callback de la barre d'outils (sliders, boutons, ...)
 */ 

void
on_xflow_main_sample_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  
  api->sample = (int)GtkAdj->value;
  xflow_api_refresh_drawing_areas (api);
}



void
on_xflow_main_scale_value_changed           (GtkRange        *range,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  
  api->scale = (float)GtkAdj->value;
  xflow_api_refresh_drawing_areas (api);
}


void
on_xflow_main_thresh_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);

  api->thresh = (float)GtkAdj->value;
  xflow_api_refresh_drawing_areas (api);
}

void
on_xflow_main_thresh_high_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);

  api->thresh_high = (float)GtkAdj->value;
  xflow_api_refresh_drawing_areas (api);
}

void
on_xflow_main_zoom_value_changed     (GtkRange        *range,
				      gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  GtkAdjustment* GtkAdj = gtk_range_get_adjustment( range);
  int nw = (float)api->wimg * GtkAdj->value;
  int nh = (float)api->himg * GtkAdj->value;

  // @TODO La requete devrait se faire faire l'onglet actif
  // @TODO Le changement d'onglet doit calculer la bonne taille
  if( GtkAdj->value >= 1) {
    gtk_widget_set_size_request ( lookup_widget( api->mainwindow, "xflow_main_vectors_draw"), nw, nh);
    gtk_widget_set_size_request ( lookup_widget( api->mainwindow, "xflow_main_mag_draw"), nw, nh);
    gtk_widget_set_size_request ( lookup_widget( api->mainwindow, "xflow_main_div_draw"), nw, nh);
    gtk_widget_set_size_request ( lookup_widget( api->mainwindow, "xflow_main_curl_draw"), nw, nh);
    gtk_widget_set_size_request ( lookup_widget( api->mainwindow, "xflow_main_hsv_draw"), nw, nh);
    alloc_for_resize( api, nw,nh);
    api->zoom = GtkAdj->value;
    xflow_api_refresh_drawing_areas (api);
  }
}



static void get_moy_vois( vel2d *uv, float *u, float *v, int dimx, int sample_x, int sample_y) {
  int i,j;
  
  *u = *v = 0;
  for( j=0; j<sample_y; j++) {
    for( i=0; i<sample_x; i++) {
      *u += uv->u;
      *v += uv->v;
      uv ++;
    }
    uv += dimx-1;
  }
  *u /= sample_x*sample_y;
  *v /= sample_x*sample_y;

}

static void draw_vectors( XFLOW_API *api, GtkWidget *widget, XFLOW_DATA *pd) {
  int i,j;
  float w_sc, h_sc;
  int w_win = api->wwin;
  int h_win = api->hwin;
  int w_img, h_img, dum;
  int size;
  float scale;
  float tl, th;

  /* Couleur des vecteurs */
  color_set_by_id( api, pd->data.xflow.arrowcolor);

  /* Taille */
  size =  pd->data.xflow.arrowsize;

  /* Epaisseur */
  gdk_gc_set_line_attributes( api->gc, pd->data.xflow.arrowwidth, GDK_LINE_SOLID, GDK_CAP_NOT_LAST,  GDK_JOIN_MITER);
  
  xflow_get_dims( pd->data.xflow.file, &w_img, &h_img, &dum); 
	  
  w_sc = (float)w_win/(float)w_img;
  h_sc = (float)h_win/(float)h_img;

  tl = api -> thresh * api -> thresh;
  th = api -> thresh_high * api -> thresh_high;

  scale = (float) api -> scale / (pd->data.xflow.norma?pd->data.xflow.normsup:1.);

  for( j = 0; j < h_img; j += api->sample)
    for( i = 0; i < w_img; i += api->sample) { 
      vel2d *uv = pd->data.xflow.buf + i + j * w_img;
      float u,v,n;

      if( pd->data.xflow.smooth) {
	int sample_x = api->sample, sample_y = api->sample;
	/* on lit i .. i+sample-1 
	   0 1 2 3   (4 pixels), sample = 3
	   0 1 2 3 4 5
	   i=3 dimx=4
	   i+3=6   3-3

	*/
	if( i + api->sample >= w_img) sample_x = w_img - i + 1;
	if( j + api->sample >= h_img) sample_y = h_img - j + 1;
	get_moy_vois( uv, &u, &v, w_img, sample_x, sample_y);
      } else {
	u = uv->u;
	v = uv->v;
      }
      n = u*u + v*v;
      if( n > tl && n < th ) {
	utils_arrow(  widget->window, api -> gc,			
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
on_xflow_main_vectors_draw_expose_event        (GtkWidget       *widget,
						GdkEventExpose  *event,
						gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd;
  int w_win = api->wwin; // widget->allocation.width;
  int h_win = api->hwin; // widget->allocation.height;
  int i,j;
  int w_img, h_img, dum;
  float w_sc, h_sc;
  Fort_int *lfmt;

  pd = api->background;
  if( pd) {
    lfmt = pd->data.image.file->lfmt;
    switch( pd->data.image.display) {	
    case SCRN_RGB:
      gdk_draw_rgb_image( widget->window, 
			  widget->style->fg_gc[GTK_STATE_NORMAL],
			  0, 0, api->wwin, api->hwin,
			  GDK_RGB_DITHER_NORMAL, 
			  pd->data.image.buf,
			  3*api->wwin);
      break;
    case SCRN_INDEXED:  /* TODO : ajouter la palette */
      gdk_draw_indexed_image( widget->window, 
			      widget->style->fg_gc[GTK_STATE_NORMAL],
			      0, 0, api->wwin, api->hwin,
			      GDK_RGB_DITHER_NORMAL, 
			      pd->data.image.buf,
			      api->wwin,
			      pd->data.image.pal
			      );
      break;
    case SCRN_GRAY:
    case SCRN_FLOAT:
      gdk_draw_gray_image( widget->window, 
			   widget->style->fg_gc[GTK_STATE_NORMAL],
			   0, 0, api->wwin, api->hwin,
			   GDK_RGB_DITHER_NORMAL, 
			   pd->data.image.buf,
			   api->wwin);
      break;
    }
  }
  for( pd = api->data; pd; pd  = pd->next)
    if( pd->type == DATA_XFLOW && pd->data.xflow.hide == 0) {
      draw_vectors( api, widget, pd);
      trajs_draw( api, widget, pd);
    }

  return FALSE;
}

static
void vectors_notify (int evx, int evy, char notify[], int len, XFLOW_API *api) {
  XFLOW_DATA *pd = api->data;  
  int x, y;

  float u=0, v=0;

  /* Toutes les images ont la même taille en dimx et dimy (voir xflow_api_addimage) */
  x = (int)(evx * (float)api->wimg/(float)api->wwin);
  y = (int)(evy * (float)api->himg/(float)api->hwin);
  if( x<0 || x >= api->wimg || y<0 || y>api->himg)
    sprintf( notify,  "-x %d -y %d -z %d <out of range> ", x+1,y+1,api->zpos);
  else {
    sprintf( notify, "-x %d -y %d -z %d", x+1, y+1, api->zpos);
    for( ; pd; pd=pd->next) {    
      if( pd->type != DATA_XFLOW) continue;    
      
      size_t count = x + y*api->wimg;
      u = pd->data.xflow.buf[count].u;
      v = pd->data.xflow.buf[count].v;      
      sprintf( notify, "%s -u %f -v %f ", notify, u, v);
    }
  } 
}

gboolean
on_xflow_main_vectors_draw_motion_notify_event   (GtkWidget       *widget,
						  GdkEventMotion  *event,
						  gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  char notify[256];
  vectors_notify( event->x, event->y, notify, 256, api);
  utils_mesag( api, notify);  
  return FALSE;
}

extern int with_trajs;
extern float DT;
gboolean
on_xflow_main_vectors_draw_button_press_event    (GtkWidget       *widget,
						  GdkEventButton  *event,
						  gpointer         user_data)
{
  char notify[256];
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd;

  switch( event->button) {
  case 1:
    if( with_trajs) {
      int init = 1;
    for( pd = api->data; pd; pd=pd->next) 
      if( pd->type == DATA_XFLOW) {
	int x, y;
	
	x = (int)(event->x * (float)api->wimg/(float)api->wwin);
	y = (int)(event->y * (float)api->himg/(float)api->hwin);  
	
	if( x<0 || y<0 || x>=api->wimg || y>api->himg)  {
	  // printf( "* -ix %d -iy %d -iz %d: <out of range>\n", x+1, y+1, api->zpos);
	  break;
	} else {
	  int traj_id;
	  if(debug) printf( "DEBUG:traj: start with %d %d\n", x, y);
	  traj_id = trajs_add( pd, x, y, api->zpos, DT /* FIXME */, 3 /* FIXME */);
	  if(debug) printf( "DEBUG:traj: trajs_add returns %d\n", traj_id);
	  
	  if( traj_id >= 0) {
	    /* force xflow to read again the current frame */
	    api->zpos ++;
	    data_read( api, api->zpos-1);
	    
	    if( init) {
	      GtkWidget *widget  = lookup_widget( api->mainwindow, 
						  "xflow_main_vectors_paned_box");
	      GtkTreeModel *model;
	      char coords[40];
	      GtkTreeIter iter;
	      
	      model = gtk_tree_view_get_model ( GTK_TREE_VIEW(lastchild(widget)));
	      gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	      sprintf( coords, "(%d,%d,%d)", x+1, y+1, api->zpos);
	      gtk_list_store_set( GTK_LIST_STORE(model), &iter,
				  0, FALSE,
				  1, coords,
				  2, coords, -1);
	      
	      init = 0;
	      
	      widget = lookup_widget( api->mainwindow, 
				      "xflow_main_vectors_paned");
	      if( gtk_paned_get_position( GTK_PANED(widget)) == 0) 
		gtk_paned_set_position( GTK_PANED(widget), 230);
	    }
	  }

	  xflow_api_refresh_drawing_areas( api);
	}
      }
    }
    break;
  case 3:
    vectors_notify( event->x, event->y, notify, 256, api);
    printf( "%s\n", notify);
    break;
  }
  return FALSE;
}


void
on_xflow_main_trajs_delete_toggled( GtkCellRendererToggle *cell,
				    gchar *path_str,
				    gpointer userdata) {
  XFLOW_API *api = (XFLOW_API *)userdata;
  XFLOW_DATA *pd;
  int num_trajs = atoi( path_str);
 

  /* remove in the trajs lists */
  for( pd = api->data; pd; pd = pd->next) 
    if( pd->type == DATA_XFLOW) {
      GSList *l = pd->data.xflow.trajs;
      pd->data.xflow.trajs = g_slist_delete_link( l, g_slist_nth( l, num_trajs));
    }

  /* remove in the store */
  GtkTreePath *path = gtk_tree_path_new_from_string( path_str);
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model( 
       GTK_TREE_VIEW(lastchild(lookup_widget(api->mainwindow,
					     "xflow_main_vectors_paned_box"))));

  gtk_tree_model_get_iter ( model, &iter, path);
  gtk_list_store_remove( GTK_LIST_STORE(model), &iter);
  gtk_tree_path_free (path);
  xflow_api_refresh_drawing_areas( api);
}

/***********************************************************************/

gboolean
on_xflow_main_mag_draw_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd = api->active;
  if( pd && pd->data.xflow.magbuf) {
    gdk_draw_gray_image( widget->window, 
			 widget->style->fg_gc[GTK_STATE_NORMAL],
			 0, 0, api->wwin, api->hwin,
			 GDK_RGB_DITHER_NORMAL, 
			 pd->data.xflow.magbuf,
			 api->wwin);
    // FIXME: ajoute la palette
    //(GdkRgbCmap*) utils_data.deg128 );
    draw_vectors( api, widget, pd);
  }
  return FALSE;
}


gboolean
on_xflow_main_mag_legend_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd = api->active;

  if( pd) {
    char label[50];
    int rep;
    int offset = 5;
    for( rep=0; rep<30; rep++) {
      gdk_draw_gray_image( widget->window,
			   widget->style->fg_gc[GTK_STATE_NORMAL],
			   rep+offset, offset, 1, 128,
			   GDK_RGB_DITHER_NORMAL, 
			   utils_data.deg128,
			   1);
      // FIXME: ajoute la palette
      //(GdkRgbCmap*) utils_data.deg128 );
    }
    snprintf( label, 50, "%f", pd->data.xflow.magmin);
    gdk_draw_text( widget->window, utils_data.font,
		   widget->style->fg_gc[GTK_STATE_NORMAL],
		   rep+2*offset,offset+14, label, strlen(label));
    
    snprintf( label, 50, "%f", pd->data.xflow.magmax);
    gdk_draw_text( widget->window, utils_data.font,
		   widget->style->fg_gc[GTK_STATE_NORMAL],
		   rep+2*offset,offset+128, label, strlen(label));
  }
  return FALSE;
}


static void mag_notify(int ev_x, int ev_y, char notify[], int len, XFLOW_API *api) {
  XFLOW_DATA *pd = api->active;  
  int x, y;
  float u, v;

  /* Toutes les images ont la même taille en dimx et dimy (voir xflow_api_addimage) */
  x = (int)(ev_x * (float)api->wimg/(float)api->wwin);
  y = (int)(ev_y * (float)api->himg/(float)api->hwin);  
  if( x<0 || x >= api->wimg || y<0 || y>api->himg)
    sprintf( notify, "-x %d -y %d -z %d <out of range> ", x+1, y+1, api->zpos);
  else {
    sprintf( notify, "-x %d -y %d -z %d", x+1, y+1, api->zpos);
    size_t count = x + y*api->wimg;
    u = pd->data.xflow.buf[count].u;
    v = pd->data.xflow.buf[count].v;  
    sprintf( notify, "%s -mag %f ", notify, sqrt(u*u+v*v));
  } 
}

gboolean
on_xflow_main_mag_draw_motion_notify_event   (GtkWidget       *widget,
					      GdkEventMotion  *event,
					      gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  char notify[256];
  
  mag_notify( event->x, event->y, notify, 256, api);
  utils_mesag( api, notify);  
  return FALSE;
}

gboolean
on_xflow_main_mag_draw_button_press_event    (GtkWidget       *widget,
					      GdkEventButton  *event,
					      gpointer         user_data)
{
  char notify[256];

  if( event->button == 3) {
    mag_notify( event->x, event->y, notify, 256, (XFLOW_API *) user_data);
    fprintf( stdout, "%s\n", notify);
  }
  return FALSE;
}

/**********************************************************/

gboolean
on_xflow_main_div_draw_expose_event              (GtkWidget       *widget,
						  GdkEventExpose  *event,
						  gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd = api->active;

  if( pd && pd->data.xflow.divbuf) {
    gdk_draw_indexed_image( widget->window, 
			    widget->style->fg_gc[GTK_STATE_NORMAL],
			    0, 0, api->wwin, api->hwin,
			    GDK_RGB_DITHER_NORMAL, 
			    pd->data.xflow.divbuf,
			    api->wwin,
			    // FIXME (GdkRgbCmap*) 
			    utils_data.div_pal );
    draw_vectors( api, widget, pd);
  }
  return FALSE;
}

gboolean
on_xflow_main_div_legend_expose_event    (GtkWidget       *widget,
					  GdkEventExpose  *event,
					  gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd = api->active;
  
  if( pd) {
    char label[50];
    int rep;
    int offset = 5;
    for( rep=0; rep<30; rep++) {
      gdk_draw_indexed_image( widget->window,
			      widget->style->fg_gc[GTK_STATE_NORMAL],
			      rep+offset, offset, 1, 128,
			      GDK_RGB_DITHER_NORMAL, 
			      utils_data.deg128,
			      1,
			      utils_data.div_pal);
      // FIXME: ajoute la palette
      //(GdkRgbCmap*) utils_data.deg128 );
    }
    snprintf( label, 50, "%f", pd->data.xflow.divmin);
    gdk_draw_text( widget->window, utils_data.font,
		   widget->style->fg_gc[GTK_STATE_NORMAL],
		   rep+2*offset,offset+14, label, strlen(label));
    
    snprintf( label, 50, "%f", pd->data.xflow.divmax);
    gdk_draw_text( widget->window, utils_data.font,
		   widget->style->fg_gc[GTK_STATE_NORMAL],
		   rep+2*offset,offset+128, label, strlen(label));
    
  }
  return FALSE;
}

static void div_notify(int ev_x, int ev_y, char notify[], int len, XFLOW_API *api) {
  XFLOW_DATA *pd = api->active;  
  int x, y;

#define div(f,dimx) ((f+1)->u - (f-1)->u + (f+dimx)->v - (f-dimx)->v)/2

  /* Toutes les images ont la même taille en dimx et dimy (voir xflow_api_addimage) */
  x = (int)(ev_x * (float)api->wimg/(float)api->wwin);
  y = (int)(ev_y * (float)api->himg/(float)api->hwin);  
  if( x<0 || x >= api->wimg || y<0 || y>api->himg)
    sprintf( notify, "-x %d -y %d -z %d <out of range> ", x+1, y+1, api->zpos);
  else {
    size_t count = x + y*api->wimg;
    vel2d *f = pd->data.xflow.buf + count;

    sprintf( notify, "-x %d -y %d -z %d", x+1, y+1, api->zpos);
    if( x > 0 && y > 0 && x < api->wimg-1 && y <api->himg-1) 
      sprintf( notify, "%s -div %f ", notify, div(f, api->wimg));
    else
      sprintf( notify, "%s -div <out of range> ", notify);
  } 
}

gboolean
on_xflow_main_div_draw_motion_notify_event   (GtkWidget       *widget,
					      GdkEventMotion  *event,
					      gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  char notify[256];

  div_notify( event->x, event->y, notify, 256, api);
  utils_mesag( api, notify);   
  return FALSE;
}

gboolean
on_xflow_main_div_draw_button_press_event    (GtkWidget       *widget,
					      GdkEventButton  *event,
					      gpointer         user_data)
{
  char notify[256];

  if( event->button == 3) {
    div_notify( event->x, event->y, notify, 256, (XFLOW_API*)user_data);
    printf( "%s\n", notify);
  }
  return FALSE;
}


/********************************************************************/


gboolean
on_xflow_main_curl_draw_expose_event              (GtkWidget       *widget,
						   GdkEventExpose  *event,
						   gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd = api->active;

  if( pd && pd->data.xflow.rotbuf ) {
    gdk_draw_indexed_image( widget->window, 
			    widget->style->fg_gc[GTK_STATE_NORMAL],
			    0, 0, api->wwin, api->hwin,
			    GDK_RGB_DITHER_NORMAL, 
			    pd->data.xflow.rotbuf,
			    api->wwin,
			    // FIXME (GdkRgbCmap*) 
			    utils_data.rot_pal);
    draw_vectors( api, widget, pd);
   }
  return FALSE;
}


gboolean
on_xflow_main_curl_legend_expose_event    (GtkWidget       *widget,
					   GdkEventExpose  *event,
					   gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd;
  
  for( pd = api->data; pd; pd  = pd->next) {    
    if( pd->type == DATA_XFLOW && pd->data.xflow.hide == 0 &&
	pd->data.xflow.rotbuf
	) {
      char label[50];
      int rep;
      int offset = 5;
      for( rep=0; rep<30; rep++) {
	gdk_draw_indexed_image( widget->window,
				widget->style->fg_gc[GTK_STATE_NORMAL],
				rep+offset, offset, 1, 128,
				GDK_RGB_DITHER_NORMAL, 
				utils_data.deg128,
				1,
				utils_data.div_pal);
	// FIXME: ajoute la palette
	//(GdkRgbCmap*) utils_data.deg128 );
      }
      snprintf( label, 50, "%f", pd->data.xflow.rotmin);
      gdk_draw_text( widget->window, utils_data.font,
		     widget->style->fg_gc[GTK_STATE_NORMAL],
		     rep+2*offset,offset+14, label, strlen(label));

      snprintf( label, 50, "%f", pd->data.xflow.rotmax);
      gdk_draw_text( widget->window, utils_data.font,
		     widget->style->fg_gc[GTK_STATE_NORMAL],
		     rep+2*offset,offset+128, label, strlen(label));

	
      break;
    }
  }
  return FALSE;
}


static void rot_notify(int ev_x, int ev_y, char notify[], int len, XFLOW_API *api) {
  XFLOW_DATA *pd = api->active;  
  int x, y;

#define rot(f,dimx)  ((f+1)->v - (f-1)->v - (f+dimx)->u + (f-dimx)->u)/2

  /* Toutes les images ont la même taille en dimx et dimy (voir xflow_api_addimage) */
  x = (int)(ev_x * (float)api->wimg/(float)api->wwin);
  y = (int)(ev_y * (float)api->himg/(float)api->hwin);  
  if( x<0 || x >= api->wimg || y<0 || y>api->himg)
    sprintf( notify, "-x %d -y %d -z %d <out of range> ", x+1, y+1, api->zpos);
  else {
    size_t count = x + y*api->wimg;
    vel2d *f = pd->data.xflow.buf + count;

    sprintf( notify, "-x %d -y %d -z %d", x+1, y+1, api->zpos);
    if( x > 0 && y > 0 && x < api->wimg-1 && y <api->himg-1) 
      sprintf( notify, "%s -curl %f ", notify, rot(f, api->wimg));
    else
      sprintf( notify, "%s -curl <out of range> ", notify);
  } 
}

gboolean
on_xflow_main_curl_draw_motion_notify_event   (GtkWidget       *widget,
					      GdkEventMotion  *event,
					      gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  char notify[256] = "";

  rot_notify( event->x, event->y, notify, 256, api);
  utils_mesag( api, notify);   
  return FALSE;
}

gboolean
on_xflow_main_curl_draw_button_press_event    (GtkWidget       *widget,
					      GdkEventButton  *event,
					      gpointer         user_data)
{
  char notify[256];

  if( event->button == 3) {
    rot_notify( event->x, event->y, notify, 256, (XFLOW_API*)user_data);
    printf( "%s\n", notify);
  }
  return FALSE;
}


/*******************************************/

gboolean
on_xflow_main_hsv_draw_expose_event              (GtkWidget       *widget,
						  GdkEventExpose  *event,
						  gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*) user_data;
  XFLOW_DATA *pd = api->active;
  
  if( pd && pd->data.xflow.hsvbuf) {
    gdk_draw_rgb_image( widget->window, 
			widget->style->fg_gc[GTK_STATE_NORMAL],
			0, 0, api->wwin, api->hwin,
			GDK_RGB_DITHER_NORMAL, 
			pd->data.xflow.hsvbuf,
			3*api->wwin);			
    draw_vectors( api, widget, pd);
  }
  return FALSE;
}

void
on_xflow_main_hsv_saturation_value_changed   (GtkRange        *range,
					      gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  XFLOW_DATA *pd = api->active;
  int w,h,d;
  
  if( pd) {
    pd->data.xflow.hsv_sat = gtk_range_get_value( range);
    
    if( debug) fprintf( stderr, "**staturation:value-changed:%f\n", pd->data.xflow.hsv_sat);
    /* il faut recalculer la map */
    xflow_get_dims( pd->data.xflow.file, &w, &h, &d);  
    utils_hsv( pd->data.xflow.buf, pd->data.xflow.hsvbuf, w, h,
	       api->wwin, api->hwin, pd->data.xflow.hsv_sat);
    xflow_api_refresh_drawing_areas (api);
  }
}  


static void hsv_notify(int ev_x, int ev_y, char notify[], int len, XFLOW_API *api) {
  XFLOW_DATA *pd = api->active;  
  int x, y;
  float u, v;

  /* Toutes les images ont la même taille en dimx et dimy (voir xflow_api_addimage) */
  x = (int)(ev_x * (float)api->wimg/(float)api->wwin);
  y = (int)(ev_y * (float)api->himg/(float)api->hwin);  
  if( x<0 || x >= api->wimg || y<0 || y>api->himg)
    sprintf( notify, "-x %d -y %d -z %d <out of range> ", x+1, y+1, api->zpos);
  else {
    sprintf( notify, "-x %d -y %d -z %d", x+1, y+1, api->zpos);
    size_t count = x + y*api->wimg;
    u = pd->data.xflow.buf[count].u;
    v = pd->data.xflow.buf[count].v;  
    sprintf( notify, "%s -mag %f -orient %.1f", notify, sqrt(u*u+v*v), atan2(v,u)/M_PI*180);
  } 
}

gboolean
on_xflow_main_hsv_draw_motion_notify_event   (GtkWidget       *widget,
					      GdkEventMotion  *event,
					      gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  char notify[256];
  
  hsv_notify( event->x, event->y, notify, 256, api);
  utils_mesag( api, notify);  
  return FALSE;
}

gboolean
on_xflow_main_hsv_draw_button_press_event    (GtkWidget       *widget,
					      GdkEventButton  *event,
					      gpointer         user_data)
{
  char notify[256];
  
  if( event->button == 3) {
    hsv_notify( event->x, event->y, notify, 256, (XFLOW_API *) user_data);
    fprintf( stdout, "%s\n", notify);
  }
  return FALSE;
}

/************************ Buttons *************************/

void
on_xflow_main_normalize_toggled             (GtkToggleButton       *button,
					     gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  XFLOW_DATA *cdata;
  for ( cdata=api->data; cdata; cdata=cdata->next)
    if( cdata->type == DATA_XFLOW)
      cdata->data.xflow.norma = 1 - cdata->data.xflow.norma;
  xflow_api_refresh_drawing_areas (api);
}


void
on_xflow_main_smooth_toggled                (GtkToggleButton       *button,
					     gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  XFLOW_DATA *cdata;
  for ( cdata=api->data; cdata; cdata=cdata->next)
    if( cdata->type == DATA_XFLOW)
      cdata->data.xflow.smooth = 1 - cdata->data.xflow.smooth;
  xflow_api_refresh_drawing_areas (api);
}

void 
on_xflow_main_vectors_checks_toggled  (GtkToggleButton *button, gpointer user_data) {
  XFLOW_API *api = (XFLOW_API *) user_data;
  XFLOW_DATA *pd;

  for( pd = api->data; pd; pd=pd->next) 
    if( pd->type == DATA_XFLOW &&
	pd->data.xflow.check == GTK_WIDGET(button)
	) {
      pd->data.xflow.hide = 1- pd->data.xflow.hide;      
    }
  xflow_api_refresh_drawing_areas(api);
}


/************************ NoteBook ************************/

void
on_xflow_main_notebook_switch_page          (GtkNotebook     *notebook,
					     GtkNotebookPage *page,
					     guint            page_num,
					     gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  XFLOW_DATA *pd;
  int w,h,d;
  int nw, nh;
  GtkWidget *widget;
  char *p;

  switch( api->paned) {
  case 0:
    widget = lookup_widget( api->mainwindow, "xflow_main_vectors_paned");
    break;
  case 1:
    widget = lookup_widget( api->mainwindow, "xflow_main_mag_paned");
    break;
  case 2:
    widget = lookup_widget( api->mainwindow, "xflow_main_div_paned");
    break;
  case 3:
    widget = lookup_widget( api->mainwindow, "xflow_main_curl_paned");
    break;
  case 4:
    widget = lookup_widget( api->mainwindow, "xflow_main_hsv_paned");
    break;
  }
  d = gtk_paned_get_position( GTK_PANED(widget));
  api->paned = page_num;
  switch( api->paned) {
  case 0:
    widget = lookup_widget( api->mainwindow, "xflow_main_vectors_paned");
    p = "xflow_main_menu_view_vectors";
    break;
  case 1:
    widget = lookup_widget( api->mainwindow, "xflow_main_mag_paned");
    p = "xflow_main_menu_view_mag";
    break;
  case 2:
    widget = lookup_widget( api->mainwindow, "xflow_main_div_paned");
    p = "xflow_main_menu_view_div";
    break;
  case 3:
    widget = lookup_widget( api->mainwindow, "xflow_main_curl_paned");
    p = "xflow_main_menu_view_curl";
    break;
  case 4:
    widget = lookup_widget( api->mainwindow, "xflow_main_hsv_paned");
    p = "xflow_main_menu_view_hsv";
    break;
  }
  gtk_paned_set_position( GTK_PANED(widget), d);


  gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(lookup_widget(api->mainwindow,p)),
				  TRUE);

  /*  
  void get_size( GtkWidget *widget, char *name, int *w, int *h) {
    widget = lookup_widget( widget, name);
    *w = widget->allocation.width;
    *h = widget->allocation.height;
    }*/

  /* FIXME: pour le moment tout est alloué dans data_read()
     il n'y a rien a faire lorsqu'on change d'onglet
   */
  return ;

  nw = api->wwin; 
  nh = api->hwin;; 

  switch( page_num) {
  case 0:
    /* Sélection de l'onglet F */
    break;
  case 1:
    /* Sélection de l'onglet M */
    for( pd = api->data; pd; pd = pd -> next) {
      if( pd->type == DATA_XFLOW) {	
	xflow_get_dims( pd->data.xflow.file, &w, &h, &d);	
	//	get_size( GTK_WIDGET(notebook), "xflow_mag", &nw, &nh);
	if(debug) fprintf( stderr, "mag: (%d,%d)->(%d,%d)\n", w,h,nw,nh);
	pd->data.xflow.magbuf = NEW(unsigned char, nw*nh);
	utils_mag( pd->data.xflow.buf, pd->data.xflow.magbuf, 
		   w, h, nw, nh, &pd->data.xflow.magmin, &pd->data.xflow.magmax);
	break;
      }
    }
    break;
  case 2:
    /* Sélection de l'onglet D */
    for( pd = api->data; pd; pd = pd -> next) {
      if( pd->type == DATA_XFLOW) { 
	xflow_get_dims( pd->data.xflow.file, &w, &h, &d);
	//	get_size( GTK_WIDGET(notebook), "xflow_div", &nw, &nh);
	pd->data.xflow.divbuf = NEW(unsigned char, nw*nh);
	utils_div( pd->data.xflow.buf, pd->data.xflow.divbuf, 
		   w, h, nw, nh, &pd->data.xflow.divmin, &pd->data.xflow.divmax);
	break;
      }
    }
    break;
  case 3:
    /* Sélection de l'onglet R */
    for( pd = api->data; pd; pd = pd -> next) {
      if( pd->type == DATA_XFLOW) {      
	xflow_get_dims( pd->data.xflow.file, &w, &h, &d);
	//	get_size( GTK_WIDGET(notebook), "xflow_rot", &nw, &nh);
	pd->data.xflow.rotbuf = NEW(unsigned char, nw*nh);
	utils_rot( pd->data.xflow.buf, pd->data.xflow.rotbuf, 
		   w, h, nw, nh, &pd->data.xflow.rotmin, &pd->data.xflow.rotmax);
	break;
      }
    }
    break;
  }
}
/******/

void on_vbox4_size_request( ){
  puts("check-resize");
}

/************************* Menu callbacks ************************************/

void
on_xflow_main_menu_write_activate            (GtkMenuItem     *menuitem,
					      gpointer         user_data)
{
  XFLOW_API  *api = (XFLOW_API *) user_data;
  XFLOW_DATA *pd;

  for( pd = api->data; pd; pd=pd->next)
    data_set_settings( api, pd);
}

void
on_xflow_main_menu_clean_activate            (GtkMenuItem     *menuitem,
					      gpointer         user_data)
{
  XFLOW_API  *api = (XFLOW_API *) user_data;
  XFLOW_DATA *pd;

  for( pd = api->data; pd; pd=pd->next)
    data_del_settings( api, pd);
}


void
on_xflow_main_menu_activefield_activate            (GtkMenuItem     *menuitem,
						    gpointer         user_data)
{
  XFLOW_API  *api = (XFLOW_API *) user_data;
  XFLOW_DATA *pd = api->data;

  /* Cherche le bon champ */
  for( ; pd; pd=pd->next) {
    if( pd->type == DATA_XFLOW) {
      char *n = g_path_get_basename(pd->data.xflow.file->iuv->nom);
      if( strcmp( n, gtk_menu_item_get_label(menuitem)) == 0) {
	GtkWidget *win = lookup_widget( api->mainwindow, "xflow_main");
	// gtk_window_set_title( GTK_WINDOW(win), n);
	api->active = pd;
	xflow_api_set_title( api);
	xflow_api_refresh_drawing_areas( api);	
	xflow_api_update_menu( api);
	trajs_update_list_store( api);
	break;
      }
    }
  }  
}

void
on_xflow_main_menu_activebg_activate            (GtkMenuItem     *menuitem,
						 gpointer         user_data)
{
  XFLOW_API  *api = (XFLOW_API *) user_data;
  XFLOW_DATA *pd = api->data;

  /* Cherche la bonne image */
  for( ; pd; pd=pd->next) {
    if( pd->type == DATA_IMAGE) {
      char *n = g_path_get_basename(pd->data.image.file->nom);
      if( strcmp( n, gtk_menu_item_get_label(menuitem)) == 0) {
	GtkWidget *win = lookup_widget( api->mainwindow, "xflow_main");
	api->background = pd;
	xflow_api_set_title( api);
	xflow_api_refresh_drawing_areas( api);	
	break;
      }
    }
  }  
}

void
on_xflow_main_menu_color_activate                  (GtkMenuItem     *menuitem,
						    gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  const char *color = gtk_menu_item_get_label( menuitem);
  int i = color_set_by_name( api, (char*)color);

  if( api->active->data.xflow.arrowcolor != i ) {
    api->active->data.xflow.arrowcolor = i;
    xflow_api_refresh_drawing_areas (api);
  }
}


void
on_xflow_main_menu_size_activate                    (GtkMenuItem     *menuitem,
						     gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  int i = get_size_by_name( (char*)gtk_menu_item_get_label( menuitem));

  if( api->active->data.xflow.arrowsize != i) {
    api->active->data.xflow.arrowsize = i;
    xflow_api_refresh_drawing_areas (api);
  }
}

void
on_xflow_main_menu_style_activate                    (GtkMenuItem     *menuitem,
						     gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget;
  char label[] = "stylex";
  int i;
  
  for( i=0; i<6; i++) {
    label[5] = '0'+i;
    widget = lookup_widget( api->mainwindow, label);
    gtk_menu_item_set_label( GTK_MENU_ITEM(widget), "");
    if( widget == GTK_WIDGET(menuitem))
	api->active->data.xflow.arrowstyle = i;
  }
  gtk_menu_item_set_label( menuitem, "*");
  xflow_api_refresh_drawing_areas (api);
}

void
on_xflow_main_menu_next_activate            (GtkMenuItem     *menuitem,
					     gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_main_zscroll");
  GtkAdjustment *adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  int value = (int) adj->value;

  value = (value+1) % (int)adj->upper;

  gtk_adjustment_set_value( adj, value);
  on_xflow_main_zscroll_value_changed ( GTK_RANGE(widget), user_data);  
}


void
on_xflow_main_menu_previous_activate                  (GtkMenuItem     *menuitem,
						       gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_main_zscroll");
  GtkAdjustment *adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  int value = (int) adj->value;
  
  value = (value > 0)?(value-1):adj->upper-1;  
  gtk_adjustment_set_value( adj, value);
  on_xflow_main_zscroll_value_changed ( GTK_RANGE(widget), user_data);  
}


void
on_xflow_main_menu_width_activate                    (GtkMenuItem     *menuitem,
						     gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  int i = get_width_by_name( gtk_menu_item_get_label( menuitem));

  if( api->active->data.xflow.arrowwidth != i) {
    api->active->data.xflow.arrowwidth = i;
    xflow_api_refresh_drawing_areas (api);
  }
}

void
on_xflow_main_menu_view_vectors_activate                 (GtkMenuItem     *menuitem,
							    gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget = lookup_widget(  api->mainwindow, "xflow_main_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 0);
}


void
on_xflow_main_menu_view_mag_activate                   (GtkMenuItem     *menuitem,
							  gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_main_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 1);
}


void
on_xflow_main_menu_view_div_activate                   (GtkMenuItem     *menuitem,
							  gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_main_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 2);
}


void
on_xflow_main_menu_view_curl_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_main_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 3);
}


void
on_xflow_main_menu_view_hsv_activate                   (GtkMenuItem     *menuitem,
							gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *)user_data;
  GtkWidget *widget = lookup_widget( api->mainwindow, "xflow_main_notebook");
  gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), 4);
}


void on_xflow_main_menu_trajs_edit_activate( GtkMenuItem *menuitem,
					     gpointer userdata) {
  XFLOW_API *api = (XFLOW_API*) userdata;


}


void on_xflow_main_menu_trajs_dump_activate( GtkMenuItem *menuitem,
					     gpointer userdata) {
  XFLOW_API *api = (XFLOW_API*) userdata;
  XFLOW_DATA *pd;  
  
  for( pd=api->data; pd; pd=pd->next)
    trajs_print( pd);
}
