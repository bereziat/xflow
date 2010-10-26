/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <string.h>
#include <stdlib.h>
#include <inrimage/image.h>
#include <inrimage/xflow.h>

/**
 * \brief Positionnement au début d'une ligne.
 *
 * Positionne un flux XFLOW au début d'une line.
 *
 * \param xflow flux à positionner,
 * \param line Numéro de la ligne (commence à 1)
 * \return code d'erreur négatif
 */

int xflow_seek( XFLOW *xflow, int line) {
  Fort_int *lfmt = xflow->iuv->lfmt;

  if( line > 0 && line <= DIMY) {
    if( xflow->iuv->f_type & FL_PIPE &&  line <= xflow->iuv->no_ligne ) {
      /* Réouverture du fichier en mode tube */
      char *str = strdup( xflow->iuv->nom);
      fermnf_( &xflow->iuv);
      xflow->iuv = image_( str, "e", "", (void*)lfmt);
      free( str);
    }      
    c_lptset( xflow->iuv, line);
    return XFLOW_ERR_OK;
  } else
    return XFLOW_ERR_RANGE;  /* plan non trouvé */
}

/**
 * \brief Positionnement au début d'un plan.
 *
 * Positionne un flux XFLOW au début d'un plan.
 *
 * \param xflow flux à positionner,
 * \param frame index du plan à chercher
 * \return code d'erreur négatif
 */

int xflow_seek_f( XFLOW *xflow, int frame) {
  Fort_int *lfmt = xflow->iuv->lfmt;

  if( frame > 0 && frame <= NDIMZ) {
    xflow_seek( xflow, (frame-1) * NDIMY + 1);
    //    c_lptset( xflow->iuv, (frame-1) * NDIMY + 1);
    return XFLOW_ERR_OK;
  } else
    return EDOM;  /* plan non trouvé */
}
