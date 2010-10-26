/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <inrimage/image.h>
#include <inrimage/xflow.h>


#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))

/**
 * \brief Création d'un fichier XFLOW.
 *
 * Ouvre en écriture un fichier de champs de vecteur au format 
 * XFLOW.
 * 
 * \param fname : nom du fichier à créer.
 * \param x,y,z : dimension du champs de vecteur. Doit être compatible
 *        avec celle d'une image de référence qui sera affiché par le
 *        programme de visualisation xflow.
 * \return descripteur sur le fichier créé.
 * \sa xflow_close(), xflow_writeframe().
 */

XFLOW* xflow_create( char *fname, int x, int y, int z) {
  XFLOW *xflow;
  Fort_int lfmt[9];  
  char version[64];

  xflow = NEW(XFLOW,1);
  assert(xflow);
  
  NDIMX = x;
  NDIMY = y;
  NDIMZ = z;
  NDIMV = 2;
  DIMX  = NDIMX*NDIMV;
  DIMY  = NDIMY*NDIMZ;
  TYPE  = REELLE;
  BSIZE = sizeof(float);
  EXP = 262;
  
  xflow->version.major = XFLOW_MAJOR;
  xflow->version.minor = XFLOW_MINOR;
  xflow->version.release = XFLOW_RELEASE;

  xflow->iuv = image_( fname, "c", " ", (void*)lfmt);  
  sprintf( version, "%d.%d.%d", XFLOW_MAJOR, XFLOW_MINOR, XFLOW_RELEASE);
  iputhline( xflow->iuv, "XFLOW_VERSION=", version);

  *xflow->i_name = '\0';
  xflow->curr_l = 0;
  xflow->buff_l = NEW(vel2d, NDIMX);

  return xflow;   
}


