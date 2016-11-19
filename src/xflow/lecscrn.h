
typedef enum {
  SCRN_RGB,   /* triplet r,v,b */
  SCRN_GRAY,  /* 1 octet niveau de gris */
  SCRN_INDEXED, /* 1 octet avec palette */
  SCRN_UNSUPPORTED  /* codage non supporté */
} t_scrn;

void typescrn(  struct image *img, t_scrn *display, unsigned char pal[][4]);
void lecscrn( struct image *img, int nline, unsigned char *buf, t_scrn display, void *pal);
