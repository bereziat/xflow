
void utils_arrow( GdkDrawable *draw, GdkGC *gc, int xa, int ya, int xb, int yb, float size, int style);
void utils_zoom( unsigned char *in, unsigned char *out, Fort_int lfmt[], int ow, int oh);
void utils_mag( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh, 
		float *vmin, float *vmax);
void utils_div( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh,
		float *vmin, float *vmax);
void utils_rot( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh,
		float *vmin, float *vmax);
void utils_hsv( vel2d *in, unsigned char *out, int iw, int ih, int ow, int oh,
		float maxmotion);

int  utils_removenan( vel2d *buf, int size);
void utils_normsup( vel2d *buf, int count, float *max) ;
void utils_mesag( XFLOW_API *api, char *fmt, ...);

typedef struct { unsigned char b,g,r,dum;} t_col32;
typedef struct {
  t_col32 *mag_pal;
  t_col32 *div_pal;
  t_col32 *rot_pal;
  unsigned char *deg128;
  GdkFont* font;  
} UTILS_DATA;
extern UTILS_DATA utils_data;

/* Gestion des couleurs */
#define MAX_COLORS 8
enum color {
  RED,
  BLUE,
  GREEN,
  CYAN,
  BLACK,
  MAGENTA,
  YELLOW,
  WHITE
};

void utils_init(void);
void utils_free(void);

void  color_init( XFLOW_API *api);
char* color_set_by_id( XFLOW_API *api, enum color id);
int   color_set_by_name( XFLOW_API *api, char *name);
char* color_name ( int id);
int   get_size_by_name( char *name);
char* size_name( int id);

void set_range_up( XFLOW_API *api, char *wgname, float upper);

GdkFont *utils_font_get( void);
void utils_pal_init(void);
void utils_pal_stretch( t_col32 *pal, float vmin, float vmax);

char* utils_basename( char *p);
int utils_pal_from_image( struct image *nf, unsigned char pal[][4]);

void check_version( XFLOW_API *api);

GtkWidget *lookup_widget( GtkBuilder *bld, char *id);
GtkWidget *firstchild( GtkWidget *wid);
GtkWidget *lastchild( GtkWidget *wid);


