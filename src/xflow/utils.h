
void utils_arrow( GdkDrawable *draw, GdkGC *gc, int xa, int ya, int xb, int yb, float size, int style);
void utils_zoom( unsigned char *in, unsigned char *out, Fort_int lfmt[], int ow, int oh);
void utils_mag( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh, float normsup);
void utils_div( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh);
void utils_rot( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh);
void utils_normsup( vel2d *buf, int count, float *max) ;
void utils_mesag( XFLOW_API *api, char *fmt, ...);

/* List of colors : see utils.c : TODO */
struct xflow_colors {
  char *name;
  GdkColor gdkcolor;
};
/* Gestion des couleurs */
enum color {
  DEFAULT,
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  YELLOW,
  WHITE
};

void color_init( XFLOW_API *api);
void color_set( XFLOW_API *api, int index);
char*color_name( XFLOW_API *api, int index);
char*size_name ( XFLOW_API *api, int size);
void set_range_up( XFLOW_API *api, char *wgname, float upper);

void utils_pal_init(void);
void* utils_pal_get(void);
char* utils_basename( char *p);
int utils_pal_from_image( struct image *nf, unsigned char pal[][4]);

void check_version( void);




