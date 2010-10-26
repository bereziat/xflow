/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <string.h>
#include <inrimage/image.h>
#include <inrimage/xflow.h>

/**
 * Retourne la vrai taille du tampon pour stocker un
 * plan du champ de vecteurs. Pour un tampon de type
 * vel2d, il faut multiplier cette taille par sizeof(float)
 * ou sizeof(vel2d) selon le type de tampon.
 *
 * \param xflow flux concerné,
 * \param mode SIZE_FRAME ou SIZE_LINE
 * \return la taille en octets.
 */

size_t xflow_get_size( XFLOW *xflow, int mode) {
  switch( mode) {
  case SIZE_LINE:
	return xflow->iuv->lfmt[I_NDIMX];
  case SIZE_FRAME:
	return xflow->iuv->lfmt[I_NDIMX]*xflow->iuv->lfmt[I_NDIMY];
  }
}


/**
 * \brief Dimension d'un flux XFLOW.
 *
 * Retourne la dimension d'un flux XFLOW.
 * \param xflow descripteur du flux
 * \param x nombre de ligne
 * \param y nombre de colonne
 * \param z nombre de plans
 *
 * \sa xflow_get_lfmt()
 */

void xflow_get_dims( XFLOW *xflow, int *x, int *y, int *z) {
  if( x) *x = xflow->iuv->lfmt[I_NDIMX];
  if( y) *y = xflow->iuv->lfmt[I_NDIMY];
  if( z) *z = xflow->iuv->lfmt[I_NDIMZ];
}

/**
 * \brief Format d'un flux XFLOW.
 *
 * Retourne le format INRIMAGE d'un flux XFLOW.
 * \param xflow descripteur du flux
 * \param lfmt tableau de 9 entier fortran (voir man Intro).
 * \sa xflow_get_dims().
 */

void xflow_get_lfmt( XFLOW *xflow, Fort_int lfmt[]) {
  memcpy( lfmt, xflow->iuv->lfmt, sizeof(Fort_int)*9);
}

/**
 * \brief Donne le numéro de version d'un fichier XFLOW.
 *
 * Retourne le numéro de version du format Xflow d'un flux pour tester
 * des éventuelles évolutions possible du format. Le fichier
 * entête de xflow (xflow.h) défini trois constantes qui 
 * caractérise la version actuelle de la librairie.
 */

void xflow_version( XFLOW *xflow, int *major, int *minor, int *release) {
  /* TODO devrait s'appeler xflow_get_version() */
  *major = xflow->version.major;
  *minor = xflow->version.minor;
  *release = xflow->version.release;
}

/**
 * \brief Ouverture de l'image référente.
 *
 * Ouvre l'image référente, si elle existe, associée à un descripteur XFLO
 * \param xflow descripteur XFLOW,
 * \param lfmt tableau du format de l'image,
 * \return descripteur INRIMAGE de l'image référente ou NULL si erreur.
 */

struct image * xflow_get_image( XFLOW *xflow, Fort_int lfmt[]) {
  struct image *img;
  
  if (is_inrimage(xflow->i_name, 2))
    return image_(xflow->i_name, "e", " ", (void*)lfmt);
  else
    return NULL;
}


/**
 * \brief Noms des fichiers associés.
 *
 * Retourne les noms des fichiers associés à un flux XFLOW.
 *
 * \param xflow flux concernée,
 * \param mode 
 *    - XFLOW_NAME_IMAGE nom du fichier image associé
 *    - XFLOW_NAME_XFLOW nom du fichier XFLOW
 * \param name tampon ou sera copier le nom du fichier
 */

void xflow_get_name( XFLOW *xflow, int mode, char name[]) {
  switch( mode) {
  case XFLOW_NAME_IMAGE:
    strcpy( name, xflow->i_name);
    break;
  case XFLOW_NAME_XFLOW:
    strcpy( name, xflow->iuv->nom);
    break;
  }
}
