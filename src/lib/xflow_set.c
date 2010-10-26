/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <stdio.h>
#include <inrimage/image.h>
#include <inrimage/xflow.h>


/**
 * \brief Associe une image de référence à un flux XFLOW.
 *
 * \param xflow descripteur XFLOW,
 * \param image nom de l'image référente,
 * \return un code d'erreur.
 *
 * Associe une image de reference à un champ de vecteur. Les
 * dimensions doivent être compatible avec celle du descripteur
 * XFLOW. La fonction le vérifie et retourne éventuellement une
 * erreur.
 */

int xflow_set_image( XFLOW *xflow, char *image) {
  Fort_int lfmt[9];
  int i;

  struct image *im = image_( image, "e", " ", (void*)lfmt);
  fermnf_( &im);

  if( NDIMX == xflow->iuv->lfmt[I_NDIMX] &&
      NDIMY == xflow->iuv->lfmt[I_NDIMY] &&
      NDIMZ >= xflow->iuv->lfmt[I_NDIMZ] 
      ) {
    strcpy( xflow->i_name, image);
    iputhline( xflow->iuv, "XFLOW_IMAGE=", image);
    return XFLOW_ERR_OK;
  } else {
    fprintf( stderr, "xflow_set_image: dimensions incompatibles\n");
    return XFLOW_ERR_RANGE;  
  }
}

