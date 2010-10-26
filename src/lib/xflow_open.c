/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <stdio.h>
#include <string.h>
#include <inrimage/image.h>
#include <inrimage/xflow.h>

#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))

/**
 * \brief Ouverture en lecture/écriture d'un fichier XFLOW.
 *
 * Ouverture d'un fichier XFLOW déjà existant. On peut y écrire
 * et y lire sans toucher aux lignes ou pixels non accédés. Pour
 * créer et écrire exclusivement dans un fichier XFLOW, on utilise
 * la fonction xflow_create().
 *
 * \param fname nom de l'image à la convention XFLOW2,
 * \return le descripteur de flux XFLOW ou \b NULL si image non
 *   présente ou format incompatible.
 *
 * \sa xflow_read(), xflow_close(), xflow_create().
 */

XFLOW* xflow_open( char *fname) {
  Fort_int lfmt[9];
  XFLOW *xflow;
  
  /* ouverture image en mode test */
  xflow = xflow_open_test( fname, lfmt, 0);

  /* retour OK si l'image est bien un flux XFLOW */
  if( xflow && DIMX == 0 ) 
    return xflow;
  else {
    if( xflow) fermnf_( (void*)&xflow);
    return NULL;
  }
}


/**
 * \brief Ouverture en lecture/écriture d'un fichier XFLOW avec
 *        vérification des dimensions.
 *
 * \param fname nom de l'image à la convention XFLOW2,
 * \param lfmt format d'une image dont on comparera les dimensions,
 * \return le descripteur de flux XFLOW ou \b NULL si image non
 *   présente ou format incompatible ou dimensions incompatibles.
 * \sa xflow_open(), xflow_open_test().
 */

XFLOW *xflow_open_verif( char *fname, Fort_int lfmt[]) {
  XFLOW *xflow;
  struct image *nf;
  Fort_int gfmt[9]; /* lfmt peut être modifié par xflow_open_test()
		     * on en fait une copie */

  /* ouverture image en mode vérification de la taille */
  memcpy( gfmt, lfmt, sizeof(Fort_int)*9);
  xflow = xflow_open_test( fname, gfmt, 1);

  if( xflow && gfmt[I_DIMX] == 0)
    return xflow;
  else {
    if( xflow) fermnf_( (void*)&xflow);
    return NULL;
  }
}

/**
 * \brief Ouverture et teste d'une image au format XFLOW ou INRIMAGE
 *        avec vérification des dimensions si demandée.
 * 
 * \param fname nom du fichier image
 * \param gfmt tableau de format (voir explication ci-dessous)
 * \param verif si \b TRUE, les dimensions de l'image ouverte
 *              sont comparées à celles fournit par le paramètre <i>gfmt</i>.
 * \return un descripteur XFLOW ou INRIMAGE ou \b NULL.
 * \sa xflow_open()
 *
 * Cette fonction est similaire à xflow_open(). Elle ouvre dabord
 * l'image. Si cette image est dans un format compatible avec XFLOW
 * alors elle retourne un descripteur \b XFLOW sinon elle retourne
 * un descripteur INRIMAGE classique (\b struct \b image *). 
 *
 * Le paramètre <i>gfmt</i> a donc deux rôles :
 *   - en entrée : si le paramètre <i>verif</i> est mis a \b TRUE,
 *     l'image est ouverte en mode vérification des dimensions. Le
 *     paramètre <i>gfmt</i> doit donc contenir des dimensions cohérentes.
 *   - en sortie : si l'image ouverte est au format XFLOW, le premier
 *     indice est nul. Si l'image est au format INRIMAGE et <i>gfmt</i> 
 *     contient son format.
 *
 * Les fonctions xflow_open() xflow_open_verif() utilisent directement
 * cette fonction.
 */

void* xflow_open_test( char *fname, Fort_int gfmt[], int verif) {
  XFLOW *xflow;
  struct image *ptrimg;
  Fort_int lfmt[9];
  char buff[256];
  int dum, 
    inrtype = *fname=='<'?IV_INR4:is_inrimage( fname, 2);

  if( inrtype) 
    ptrimg = image_( fname, "e", "", (void*)lfmt);
  else
    return NULL;
  
  /* vérifier les dimensions sauf composantes */
  if( verif && !(NDIMX == gfmt[I_NDIMX] &&
		 NDIMY == gfmt[I_NDIMY] &&
		 NDIMZ == gfmt[I_NDIMZ] )) {
    fprintf( stderr, 
	     "xflow_open_test: codage incompatible\n");
    fermnf_( (void*)&ptrimg);
    return NULL;
  }

  /* format XFLOW = (INR4,NDIMV==2,REELLE) */
  if( (inrtype & 0xFF) == IV_INR4 && NDIMV == 2 && TYPE == REELLE) {    
    xflow = NEW(XFLOW,1);
    xflow->iuv = ptrimg;

    c_lptset( xflow->iuv, 1);
    xflow->curr_l = 0;
    xflow->buff_l = NEW( vel2d, NDIMX);
    
    if( igethline( xflow->iuv, "XFLOW_VERSION=", 0, buff, 255, &dum) != -1) {
      sscanf( buff, "%d.%d.%d", &xflow->version.major, &xflow->version.minor, &xflow->version.release);
      // verif numéro de version a faire
    } else {
      // Fichier sans numéro de version (créé à la main)
      // on force un numéro de version
      xflow->version.major = XFLOW_MAJOR;
      xflow->version.minor = XFLOW_MINOR;
      xflow->version.release = XFLOW_RELEASE;
    }
    
    if( igethline( xflow->iuv, "XFLOW_IMAGE=", 0, buff, 255, &dum) != -1)
      strcpy( xflow->i_name, buff);
    
    gfmt[I_DIMX] = 0; /* pour décider de la nature du retour */
    return xflow;
  } else {
    memcpy( gfmt, lfmt, sizeof(Fort_int)*9);
    return ptrimg; 
  }
}
