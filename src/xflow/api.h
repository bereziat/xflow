/*
 * fusionner data.h et api.h puisque data.h utilise XFLOW_API
 */

enum {
  API_SAMPLE,
  API_SCALE,
  API_TLOW,
  API_THIGH
};

XFLOW_API* 
     xflow_api_new( void);
void xflow_api_delete( XFLOW_API *api);

void xflow_api_set_global( XFLOW_API *api, int mode, float val);

void xflow_api_show_window( XFLOW_API *api);
void xflow_api_update_widget( XFLOW_API *api);
void xflow_api_update_menu( XFLOW_API *api);
void xflow_api_refresh_drawing_areas( XFLOW_API *api);
void xflow_api_set_title( XFLOW_API *api);


/* callbacks non définies par Glade */
void
on_xflow_main_menu_activefield_activate            (GtkMenuItem     *menuitem,
						    gpointer         user_data);
void
on_xflow_main_menu_activebg_activate            (GtkMenuItem     *menuitem,
						 gpointer         user_data);

void 
on_xflow_main_vectors_checks_toggled  (GtkToggleButton *button, gpointer user_data);
