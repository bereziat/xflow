/*
 * xflow - (c) 2013 Dominique Bereziat 
 */

#include <gtk/gtk.h>
#include <inrimage/image.h>
#include <inrimage/xflow.h>
#include "lecscrn.h"


#define DATA_IMAGE 1
#define DATA_XFLOW 2

typedef struct xflow_data {
  int type;  ///>* Type de donnée */
  union {
    struct {
      t_scrn display;    
      unsigned char *read; /* tampon du plan lu */
      unsigned char *buf;  /* tampon de l'écran */
      struct image *file;
      unsigned char pal[256][4];
    } image;
    struct xflow_data_xflow {
      int hide;    /* True : le champ est masqué */
      GtkWidget *check;
      int norma;   /* True : le champ est normalisé */
      int smooth;  /* True : le champ est lissé */
      float normsup;   /* Norme sup du champ */
      int arrowcolor;   /* Si -1, on prend la valeur par défaut (api.arrowcolor) */
      int arrowsize;    /* Si -1, on prend la valeur par défaut */
      int arrowwidth;
      int arrowstyle;
      vel2d *buf;    /* Champs de vecteur */
      XFLOW *file;
      unsigned char *magbuf;   /* Magnétudes */
      float magmin, magmax;
      unsigned char *divbuf;   /* Divergence */
      float divmin, divmax;
      unsigned char *rotbuf;   /* Rotationel */
      float rotmin, rotmax;
      unsigned char *hsvbuf;
      float hsv_sat;

      GSList *trajs;    /* trajectoires! */ 
    } xflow;
  } data;
  struct xflow_data *next;
} XFLOW_DATA;


typedef struct xflow_api {
  int   sample;
  float thresh, scale, thresh_high;
  float zoom;
  int   wimg, himg;   ///> Taille des images.
  int   wwin, hwin;   ///> Dimension de l'image dans le widget
  int   zpos;         ///> Index des plans en mémoire ou 0
  int   zmax;
  GtkBuilder *mainwindow, *export, *about;
  int paned;
  GdkGC *gc;          ///> Contexte graphique pour la drawing area
  XFLOW_DATA *data;   ///> Liste des images
  XFLOW_DATA *background;   ///> Image d'arriere plan courant
  XFLOW_DATA *active;  ///> Champs de vecteurs courant  @TODO
} XFLOW_API;


#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))
#define DELETE(var)    i_Free((char **)&var)

int  data_addimage ( XFLOW_API *api, char *filename);
void data_insert   ( XFLOW_API *api, XFLOW_DATA *data, int mode);
void data_free     ( XFLOW_API *api);
void data_read     ( XFLOW_API *api, int z);
void data_get_settings( XFLOW_API *api, XFLOW_DATA *pd);
void data_set_settings( XFLOW_API *api, XFLOW_DATA *pd);
void data_del_settings( XFLOW_API *api, XFLOW_DATA *pd);

extern int debug;
