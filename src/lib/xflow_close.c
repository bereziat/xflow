/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>


#define DELETE(var)    i_Free((char **)&var)

/**
 * \brief Fermeture d'un flux XFLOW.
 *
 * \param xflow descripteur XFLOW.
 *
 * Ferme un flux ouvert par xflow_create() ou xflow_open().
 *
 */

void xflow_close( XFLOW *xflow) {
  /* Important : sauvegarde ligne tampon si acces pixel */
  if( xflow->curr_l) {
    c_lptset( xflow->iuv, xflow->curr_l);
    xflow_write_v2d( xflow, 1, xflow->buff_l);
  }
  DELETE( xflow->buff_l);
  fermnf_( &xflow->iuv);
}
