/*
 * Lecture inrimage avec conversion pour affichage à l'écran.
 * $Id: lecscrn.c,v 1.4 2005/04/08 15:05:41 bereziat Exp $
 */

#include <inrimage/image.h>
#include "lecscrn.h"

#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))
#define DELETE(var)    i_Free((char **)&var)

/**
 * Vérification du format et retour du type d'affichage.
 */

void typescrn(  struct image *img, t_scrn *display, unsigned char pal[][4]) {
  Fort_int *lfmt = img -> lfmt;

  if( TYPE == FIXE && BSIZE == 1) {
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
  } else
    *display = SCRN_FLOAT;
  
  /* Image double précision non supporté */
  if( TYPE == REELLE && BSIZE == 8)
    *display = SCRN_UNSUPPORTED;
  
  /* Image packée non supporté (format obsolete) */
  /* if( TYPE == PACKEE) 
    *display = SCRN_UNSUPPORTED;
    */
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
 *   - image vectorielles (sauf -v 3 -o 1 -f)
 *   - image RGB codés avec -z 3 -o 1 -v 1
 *   - manque les images a palette
 *
 * \bug Pas de vérification sur les formats supportés
 */

void lecscrn( struct image *img, int nline, unsigned char *buf, t_scrn display, void *pal) {
  Fort_int *lfmt = img->lfmt;
  int n;
  float *fbuf, *pfbuf;
  unsigned char *pbuf;
  float min,max;

  switch( display) {
  case SCRN_RGB:
  case SCRN_GRAY:
  case SCRN_INDEXED:
    /* Cas de lecture directe */
    c_lect( img, nline, buf);
    break;
  case SCRN_FLOAT:
    /* Cas de lecture par lecflt : je normalise pour l'écran */
    fbuf = NEW( float, NDIMX*nline);
    c_lecflt( img, nline, fbuf);
    min = max = *fbuf;
    for( pfbuf = fbuf, n = nline*NDIMX; n>0; n --, pfbuf ++) {
      if( *pfbuf > max) max = *pfbuf;
      if( *pfbuf < min) min = *pfbuf;
    }
    max -= min;
    for( pfbuf = fbuf, n = nline*NDIMX, pbuf = buf; n>0; 
	 n --, pfbuf ++, pbuf ++)
      *pbuf = (unsigned char) 255*(*pfbuf-min)/max;
    DELETE(fbuf);
    break;
  default:    
    break;
  }  

}

