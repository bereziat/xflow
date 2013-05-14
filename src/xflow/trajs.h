/*
 * 
 */

typedef struct {
  float x, y;
  
} POINT2D;

typedef struct {
  float t, i, j;
  float dt;
} POINT3D;

typedef  struct {
  POINT3D* precise_coords;
  unsigned int num_points;
  unsigned int starting_zpos; 
} TRAJECTORY;


int trajs_add    ( XFLOW_DATA *pd, int x, int y, int z, float dt, float tol);
void trajs_print ( XFLOW_DATA *pd);
void trajs_draw  ( XFLOW_API *api, GtkWidget *widget, XFLOW_DATA *pd);
void trajs_free  ( XFLOW_DATA *pd);
void trajs_update_list_store( XFLOW_API *api);
