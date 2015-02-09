/**
 * \file xflow.h
 * \brief Interface de la librairie XFLOW (écriture et lecture d'image
 *        de champs de vecteur).
 */

/* $Id: xflow.h,v 1.4 2009/07/17 09:38:13 bereziat Exp $ */

#ifndef _XFLOW_H_
#define _XFLOW_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <inrimage/image.h>

  /**> Numéro majeur de version de la librarie libxflow */
#define XFLOW_MAJOR 2   
  /**> Numéro mineur de version de la librarie libxflow */
#define XFLOW_MINOR 0   
  /**> Numéro de mise-à-jour dans la version de la librarie libxflow */
#define XFLOW_RELEASE 4

  /**> Par erreur */
#define XFLOW_ERR_OK     0
  /**> Erreur de paramétrage dans l'accès à une donnée */
#define XFLOW_ERR_RANGE -1
  /**> Erreur de paramétrage dans l'accès à une donnée */
#define XFLOW_ERR_UNSET -2

  /**> Acces par ligne */
#define SIZE_LINE 1
  /**> Acces par plan */
#define SIZE_FRAME 2

#define XFLOW_NAME_IMAGE 0
#define XFLOW_NAME_XFLOW 1

/**
 * Structure velocity 2D : un pixel possède une
 * composante horizontale (u) et verticale (v).
 * \struct vel2d
 */
typedef struct { 
  /** \brief Composante horizonte */
  float u; 
  /** \brief Composante verticale */
  float v; 
} vel2d;

typedef struct {
  float u,v,w;
} vel3d;

/**
 * Structure privée décrivant un flux de donnée XFLOW.
 * \struct XFLOW
 */

typedef struct {
  /** \brief Numéro de version de la librarie qui a généré ce flux */
  struct {
    int major, minor, release;
  } version;
  /** \brief nom de l'image référente */
  char i_name[256];   
  
  /** \brief descripteur INRIMAGE associé au flux XFLOW (accès en lecture) */
  struct image *iuv;  
  /** \brief index de la ligne courante ou 0 si pas de ligne courante (accès en lecture) */
  size_t curr_l; 
  /** \brief tampon de la ligne courante (accès en lecture) */
  vel2d *buff_l; 
} XFLOW;

/** \addtogroup libxflow
 *  \{ */

XFLOW* xflow_open    ( char *fname);
XFLOW* xflow_open_verif( char *fname, Fort_int lfmt[]);
void* xflow_open_test( char *fname, Fort_int lfmt[], int verif);
void  xflow_close    ( XFLOW *xflow);
XFLOW*xflow_create   ( char *fname, int x, int y, int z);
void  xflow_version  ( XFLOW *xflow, int *major, int *minor, int *release);
struct image* 
      xflow_get_image( XFLOW *xflow, Fort_int lfmt[]);
int   xflow_set_image( XFLOW *xflow, char *image);
void  xflow_get_dims ( XFLOW *xflow, int *x, int *y, int *z);
void  xflow_get_lfmt ( XFLOW *xflow, Fort_int lfmt[]);
void  xflow_get_name ( XFLOW *xflow, int mode, char name[]);

void  xflow_read     ( XFLOW *xflow, int line, float *u, float *v);
void  xflow_read_v2d ( XFLOW *xflow, int line, vel2d *uv);
void  xflow_write    ( XFLOW *xflow, int line, float *u, float *v);
void  xflow_write_v2d( XFLOW *xflow, int line, vel2d *uv);

void  xflow_read_f     ( XFLOW *xflow, int frame, float *u, float *v);
void  xflow_read_f_v2d ( XFLOW *xflow, int frame, vel2d *uv);
void  xflow_write_f    ( XFLOW *xflow, int frame, float *u, float *v);
void  xflow_write_f_v2d( XFLOW *xflow, int frame, vel2d *uv);

int   xflow_read_value     ( XFLOW *xflow, int x, int y, int z, float *u, float *v);
int   xflow_read_value_v2d ( XFLOW *xflow, int x, int y, int z, vel2d *val);
int   xflow_write_value    ( XFLOW *xflow, int x, int y, int z, float u, float v);
int   xflow_write_value_v2d( XFLOW *xflow, int x, int y, int z, vel2d *val);

int   xflow_seek     ( XFLOW *xflow, int line);
int   xflow_seek_f   ( XFLOW *xflow, int frame);

/** \} */



#ifdef __cplusplus
}
#endif
#endif /* _XFLOW_H_ */
