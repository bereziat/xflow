/*
 * Lecture inrimage avec conversion pour affichage à l'écran.
 */

#include <inrimage/image.h>
#include "lecscrn.h"

#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))
#define DELETE(var)    i_Free((char **)&var)

/**
 * Vérification du format et retour du type d'affichage (pour GTK)
 */

void typescrn(  struct image *img, t_scrn *display, unsigned char pal[][4]) {
  Fort_int *lfmt = img -> lfmt;

  if ( TYPE == REELLE && BSIZE == 8)  // peut être levé facilement
    *display = SCRN_UNSUPPORTED;
  else
    switch( NDIMV) {
    case 1:       
      if( utils_pal_from_image( img, pal))
	*display = SCRN_INDEXED;
      else
	*display = SCRN_GRAY;       
      break;
    case 3: 
      *display = SCRN_RGB; 
      break;
    default: 
      *display = SCRN_UNSUPPORTED;
    }
 }

/**
 * \param img descripteur d'image INRIMAGE
 * \param nline nombre de ligne à lire
 * \param buf tampon où sont copiés les lignes
 *
 * Lit 'nline' lignes dans un fichier image et retourne
 * un buffer pret à être affiché à l'écran
 * c'est-à-dire qu'il est soit 1 octet en niveau de gris
 * soit 3 octets en RBG. Hors ces formats natifs, les autres
 * formats sont convertis en 1 octet. 
 *
 * Formats non supportés :
 *   - image vectorielles (sauf -v 3)
 *   - image RGB codés avec -z 3
 *   - manque les images à palette
 * 
 * \bug Pas de vérification sur les formats supportés
 */

void lecscrn( struct image *img, int nline, unsigned char *buf, t_scrn display, void *pal) {
  Fort_int *lfmt = img->lfmt;

  if( TYPE == FIXE && BSIZE == 1)
    /* Cas de lecture directe */
    c_lect( img, nline, buf);
  else {
    /* conversion au format 1 octet (écran) */
    int n;
    float *fbuf, *pfbuf;
    unsigned char *pbuf;
    float min,max;

    /* Cas de lecture par lecflt : je normalise pour l'écran */
    /* limitation: les images RGB sont normalisées globalement */
    
    fbuf = NEW( float, DIMX*nline);
    c_lecflt( img, nline, fbuf);
    min = max = *fbuf;
    for( pfbuf = fbuf, n = nline*DIMX; n>0; n --, pfbuf ++) {
      if( *pfbuf > max) max = *pfbuf;
      if( *pfbuf < min) min = *pfbuf;
    }
    max -= min;
    for( pfbuf = fbuf, n = nline*DIMX, pbuf = buf; n>0; 
	 n --, pfbuf ++, pbuf ++)
      *pbuf = (unsigned char) 255*(*pfbuf-min)/max;
    DELETE(fbuf);
  }
}

