#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
#include "data.h"
#include "trajs.h"
#include "utils.h"

typedef float (*pfun1D) (float,float);
typedef float (*pfun2D) (float,float,float);

/*
 * Integrate the 1-D ODE $dy/dt = F(y)$ using a 4-order Runge Kutta scheme.
 *
 * t0,y0: initial point y(t0) = y0
 * dt: time step
 * F: funtion F
 * num_iter: number of steps.
 * P : output, the trajectory
 */

static void  RK4_1D( float t0, float y0, float dt, pfun1D F,
	      float num_iter, POINT2D* P ){
  int n;
  float t=t0;
  int current_t=0;
  P[current_t].x=t0;
  P[current_t].y=y0;
  for(n=0;n<num_iter;++n) {
    float K1,K2,K3,K4;
    K1=F(t,P[current_t].y);
    K2=F(t+dt/2.0,P[current_t].y+K1*dt/2.0);
    K3=F(t+dt/2.0,P[current_t].y+K2*dt/2.0);
    K4=F(t+dt,P[current_t].y+dt*K3);
    P[1+current_t].y=P[current_t].y+(1/6.0)*dt*(K1+2*K2+2*K3+K4);
    P[current_t].x=t;
    ++current_t;
    t+=dt;
  }
}

/*
 * Integrate a 2-D ODE using a 4-order Runge-Kutta scheme.
 * FIXME le 1D n'est pas utilisé, le virer et transférer la doc ici
 */

void  RK4_2D( float t0, float i0, float j0, float dt,
	      pfun2D F_U, pfun2D F_V,float num_iter, POINT3D*  P ) {

  int n;
  float t=t0;
  int current_t=0;
  P[current_t].t=t0;
  P[current_t].i=i0;
  P[current_t].j=j0;
  for(n=0;n<num_iter;++n) {
    float K1_U,K2_U,K3_U,K4_U;
    float K1_V,K2_V,K3_V,K4_V;
    
    K1_U=F_U(t,P[current_t].i,P[current_t].j);
    K1_V=F_V(t,P[current_t].i,P[current_t].j);
    
    K2_U=F_U(t+dt/2.0,P[current_t].i+K1_U*dt/2.0,P[current_t].j+K1_V*dt/2.0);
    K2_V=F_V(t+dt/2.0,P[current_t].i+K1_U*dt/2.0,P[current_t].j+K1_V*dt/2.0);
    
    K3_U=F_U(t+dt/2.0,P[current_t].i+K2_U*dt/2.0,P[current_t].j+K2_V*dt/2.0);
    K3_V=F_V(t+dt/2.0,P[current_t].i+K2_U*dt/2.0,P[current_t].j+K2_V*dt/2.0);
    
    K4_U=F_U(t+dt,P[current_t].i+dt*K3_U,P[current_t].j+dt*K3_V);
    K4_V=F_V(t+dt,P[current_t].i+dt*K3_U,P[current_t].j+dt*K3_V);
    

    P[1+current_t].i=P[current_t].i+(1/6.0)*dt*(K1_U+2*K2_U+2*K3_U+K4_U);
    P[1+current_t].j=P[current_t].j+(1/6.0)*dt*(K1_V+2*K2_V+2*K3_V+K4_V);

    ++current_t;
    t+=dt;
    P[current_t].t=t;
  }
}


/*
 * Read a velocity from a data
 */


static 
void read_velocity( XFLOW_DATA *pd, int t, int pixel_i, int pixel_j, vel2d *vel) {
  Fort_int *lfmt=pd->data.xflow.file->iuv->lfmt;
  vel2d *line = NEW(vel2d, DIMX);


  if( t > NDIMZ) {
    fprintf( stderr, "\033[31m%s\033[39m time %d if out of range in image %s\n", 
	     "Warning:",t,pd->data.xflow.file->iuv->nom);
    t = NDIMZ;
  }
  xflow_seek( pd->data.xflow.file, (t-1)*NDIMY + pixel_j + 1);
  xflow_read_v2d( pd->data.xflow.file, 1, line);
  
  vel->u = line[pixel_i].u;
  vel->v = line[pixel_i].v;
  
  DELETE(line);
}

static
void read_velocity_interp( XFLOW_DATA *pd, float t, float pixel_i, float pixel_j,
			   vel2d *vel) {

  vel2d veli[6];
  int i=(int)pixel_i;
  int j=(int)pixel_j;
  float dist_i = pixel_i-i;
  float dist_j = pixel_j-j;
  float dist_t = t - (int) t;
  
  /* à t */

  read_velocity( pd, (int) t, i, j, veli);
  read_velocity( pd, (int) t, i+1, j, veli+1);
  veli[2].u = veli[0].u * (1-dist_i) + veli[1].u * dist_i;
  veli[2].v = veli[0].v * (1-dist_i) + veli[1].v * dist_i;

  read_velocity( pd, (int) t, i, j+1, veli);
  read_velocity( pd, (int) t, i+1, j+1, veli+1);
  veli[3].u = veli[0].u * (1-dist_i) + veli[1].u * dist_i;
  veli[3].v = veli[0].v * (1-dist_i) + veli[1].v * dist_i;

  veli[4].u = veli[2].u * (1-dist_j) + veli[3].u * dist_j;
  veli[4].v = veli[2].v * (1-dist_j) + veli[3].v * dist_j;


  /* à t + 1 */

  read_velocity( pd, 1 + (int) t, i, j, veli);
  read_velocity( pd, 1 + (int) t, i+1, j, veli+1);
  veli[2].u = veli[0].u * (1-dist_i) + veli[1].u * dist_i;
  veli[2].v = veli[0].v * (1-dist_i) + veli[1].v * dist_i;

  read_velocity( pd, 1 + (int) t, i, j+1, veli);
  read_velocity( pd, 1 + (int) t, i+1, j+1, veli+1);
  veli[3].u = veli[0].u * (1-dist_i) + veli[1].u * dist_i;
  veli[3].v = veli[0].v * (1-dist_i) + veli[1].v * dist_i;


  veli[5].u = veli[2].u * (1-dist_j) + veli[3].u * dist_j;
  veli[5].v = veli[2].v * (1-dist_j) + veli[3].v * dist_j;

  /* Final */

  vel->u = veli[4].u * (1-dist_t) + veli[5].u * dist_t;
  vel->v = veli[4].v * (1-dist_t) + veli[5].v * dist_t;

}

static float norm( float x1, float x2) {
  return sqrt( x1*x1 + x2*x2);
}

/* ''driver'' for RK-O4 */

static XFLOW_DATA *current;
static vel2d uv;

float Velocity_u( float t, float x, float y) {
  read_velocity_interp( current, t, x, y, &uv);
  //  printf( "## %f %f %f -> %f ", t, x, y, uv.u);
  return uv.u;
}

float Velocity_v( float t, float x, float y) {
  //  printf( "%f\n", uv.v);
  return uv.v;
}

/* Api */

/**
 * Compute a new trajectory.
 * @param pd : a vectory field data,
 * @param x0, y0, z0 : initial point,
 * @param tol : the trajectory is computed if not too close to the others ones.
 * @return position of the trajectory in the list, otherwise an error code
 *    - -1 : trajectory already defined
 *    - -2 : cannot read velocities from stdin
 *    - -3 : not a XFLOW image
 */
int trajs_add( XFLOW_DATA *pd, int x0, int y0, int z0, float dt, float tol) {
  if( pd->type == DATA_XFLOW) {
    GSList *l;
    TRAJECTORY *traj;
    int count = 0;

    if( pd->data.xflow.file->iuv->f_type & FL_PIPE) 
      return -2;
   
    for( l= pd->data.xflow.trajs; l; l=l->next) {
      traj = l->data;
      count ++;
      if( traj->starting_zpos == z0 &&
	  norm( x0 - traj->precise_coords[0].i, y0 - traj->precise_coords[0].j) < tol) {
	if( debug) printf("DEBUG:traj: required: %d %d, found: %f %f\n",
			  x0, y0, traj->precise_coords[0].i, traj->precise_coords[0].j);
     	return -1;
      }
    }
    
    traj = NEW( TRAJECTORY, 1);
    pd->data.xflow.trajs = g_slist_append( pd->data.xflow.trajs, traj);
    
    traj -> num_points =  pd->data.xflow.file->iuv->NDIMZ - z0 + 1;
    traj -> starting_zpos = z0;
    traj -> precise_coords = NEW(POINT3D,traj->num_points);
    traj -> hidden = 0;

    current = pd;
    
    RK4_2D( z0, x0, y0, dt, Velocity_u, Velocity_v, 
	    traj->num_points - 1 , traj->precise_coords);
    
    return count;
  }
  return -3;
}

void trajs_print( XFLOW_DATA *pd) {
  if( pd->type == DATA_XFLOW && pd->data.xflow.trajs) {
    GSList *l;
    int i=1;
    
    printf( "## Trajectories for %s\n", pd->data.xflow.file->iuv->nom);
    for( l = pd -> data.xflow.trajs; l; l = l->next) {
      int j;
      TRAJECTORY *t = l->data;
      printf("## trajectory #%d\n", i++);
      for( j=0; j<t->num_points; j++)
	printf( "%f %f %f\n", 
		t->precise_coords[j].t, t->precise_coords[j].i, t->precise_coords[j].j);
    }
  }
}

int point_factor = 3;
int line_width = 4;

void trajs_draw( XFLOW_API *api, GtkWidget *widget, XFLOW_DATA *pd) {
  if ( pd->type == DATA_XFLOW && pd->data.xflow.trajs) {
    GSList *l;
    int j = 0;

    /* FIXME : si on a plusieurs champs de vitesse, on a une couleur
     * par champs. Si on un seul champ de vitesse, on fait varier les
     * couleurs des trajectoires. */
    int fixcolor = 0;
    XFLOW_DATA *p;
    for( p=api->data; p; p=p->next) {
      if( p->type == DATA_XFLOW) fixcolor++;
      if( fixcolor > 1) break;
    }

    for( l = pd->data.xflow.trajs; l; l=l->next) {
      TRAJECTORY *traj = l -> data;
      if( traj -> hidden == 0 ) {
	GdkPoint *points = NEW( GdkPoint, traj->num_points);
	int i;
	for( i=0; i<traj->num_points; i++) {
	  points[i].x = (int)((traj->precise_coords[i].i)*((float)api->wwin/(float)api->wimg)) ;
	  points[i].y = (int)((traj->precise_coords[i].j)*((float)api->hwin/(float)api->himg)) ;
	}
	
	if( fixcolor == 1) 
	  color_set_by_id( api, pd->data.xflow.arrowcolor+j++);
	else
	  color_set_by_id( api, pd->data.xflow.arrowcolor);
	
	gdk_gc_set_line_attributes( api->gc, line_width, 
				    GDK_LINE_DOUBLE_DASH, GDK_CAP_NOT_LAST,  GDK_JOIN_MITER);
	
	gdk_draw_lines( widget->window, api->gc, points, i);
	
	float scale = api->wwin/ api->wimg;
	// FIXME
#define POINTSIZE point_factor * scale
	if( traj -> starting_zpos <= api->zpos) {
	  gdk_draw_arc (widget->window,
			api->gc,
			FALSE,
			points[api->zpos - traj->starting_zpos].x - POINTSIZE,
			points[api->zpos - traj->starting_zpos].y - POINTSIZE,
			POINTSIZE * 2,
			POINTSIZE * 2, 
			0, 64*360);
	}
	
	DELETE(points);
      }
    }
  }
}


void trajs_free( XFLOW_DATA *pd) {
  if( pd->type == DATA_XFLOW && pd->data.xflow.trajs) {
    GSList *l;

    for( l = pd->data.xflow.trajs; l; l=l->next) {
      TRAJECTORY *traj = l -> data;
      DELETE( traj -> precise_coords);
      DELETE( traj);
    }
    g_slist_free( pd->data.xflow.trajs);
    pd->data.xflow.trajs = NULL;
  }
}


void trajs_update_list_store( XFLOW_API *api) {
  if( api->active && api->active->data.xflow.trajs) {
    GtkTreeIter iter;
    gboolean t;
    char text[30];
    GSList *l = api->active->data.xflow.trajs;
    GtkTreeModel *model = gtk_tree_view_get_model( 
            GTK_TREE_VIEW(lastchild(lookup_widget(api->mainwindow,
						  "xflow_main_vectors_paned_box"))));
 
    t = gtk_tree_model_get_iter_first ( model, &iter);
    while( t) {
      TRAJECTORY *traj = l->data;

      if( traj -> starting_zpos > api->zpos)
	*text = 0;
      else
	sprintf( text, "(%.1f,%.1f,%d)", 
		 traj->precise_coords[api->zpos-traj->starting_zpos].i,
		 traj->precise_coords[api->zpos-traj->starting_zpos].j,
		 (int)traj->precise_coords[api->zpos-traj->starting_zpos].t
		 );
      gtk_list_store_set ( GTK_LIST_STORE(model), &iter, 3, text, -1);       
      t = gtk_tree_model_iter_next( model, &iter);
      
      l = l -> next;
    }
  }
}
