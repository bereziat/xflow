/*
 * xflow - (c) 2005 Dominique Bereziat 
 * $Id: data.h,v 1.16 2010/10/20 16:11:08 bereziat Exp $
 */

#include <gtk/gtk.h>
#include <inrimage/image.h>
#include <inrimage/xflow.h>
#include "lecscrn.h"

#define XFLOW_API_MAJOR   2
#define XFLOW_API_MINOR   3
#define XFLOW_API_RELEASE 3

#define XFLOW_API_BETA    0

#define DATA_IMAGE 1
#define DATA_XFLOW 2

typedef struct xflow_data {
  int type;  ///>* Type de donnée */
  union {
    struct {
      t_scrn display;    
      unsigned char *read;
      unsigned char *buf;
      struct image *file;
      unsigned char pal[256][4];
    } image;
    struct xflow_data_xflow {
      int hide;    /* True : le champ est masqué */
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
      unsigned char *divbuf;   /* Divergence */
      unsigned char *rotbuf;   /* Rotationel */
    } xflow;
  } data;
  struct xflow_data *next;
} XFLOW_DATA;


typedef struct xflow_api {
  int   sample;
  float thresh, scale, thresh_high;
  int   arrowsize;
  int   arrowcolor;
  float zoom;
  int   wimg, himg;   ///> Taille des images.
  int   wwin, hwin;   ///> Dimension de l'image dans le widget
  int   zpos;         ///> Index des plans en mémoire ou 0
  int   zmax;
  GtkWidget *mainwindow;
  GdkGC *gc;          ///> Contexte graphique pour la drawing area
  XFLOW_DATA *data;
  
  GList *trajs;    /* trajectoires affichées dans la fenêtre */ 
} XFLOW_API;


#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))
#define DELETE(var)    i_Free((char **)&var)


void data_insert( XFLOW_API *api, XFLOW_DATA *data, int mode);
void data_free( XFLOW_API *api);
void data_read( XFLOW_API *api, int z);
void data_get_settings( XFLOW_API *api, XFLOW_DATA *pd);

extern int debug;
