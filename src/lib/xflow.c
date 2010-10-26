/** 
 *  Dominique B�r�ziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 * 
 * $Id: xflow.c,v 1.10 2009/07/17 09:38:13 bereziat Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inrimage/image.h>
#include <assert.h>
#include <errno.h>

#include "../inrimage/xflow.h"

#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))
#define DELETE(var)    i_Free((char **)&var)

/* variables Inrimage */
extern int xargc;
extern char **xargv;

/* copie un tampon vel2d vers deux tampons float */
static 
inline void v2d2uv( vel2d *uv, float *u, float *v, int count) {
  while( count --) {
    *u ++ = uv->u;
    *v ++ = uv->v;
    uv ++;
  }
}

/* copie deux tampons float vers un tampon vel2d */
static 
inline void uv2v2d( float *u, float *v, vel2d *uv, int count) {
  while( count --) {
    uv->u = *u ++;
    uv->v = *v ++;
    uv++;
  }
}

/**
 * \brief Lecture ligne � ligne.
 *
 * Lecture dans un flux XFLOW. La lecture se fait par ligne (comme
 * INRIMAGE).
 * \param xflow descripteur XFLOW,
 * \param line nombre de lignes � lire,
 * \param u,v tampon des composantes du champs de vecteur.
 * \pre les tampons u et v doivent �tre suffisament grands pour accueillir
 *      les donn�es lues.
 * \return code d'erreur n�gatif.
 * \sa xflow_read_v2d()
 */

void xflow_read( XFLOW *xflow, int line, float *u, float *v) {
  int ndimx = xflow->iuv->lfmt[I_NDIMX];
  vel2d *buf = NEW(vel2d,ndimx);
  
  while( line -- ) {
    /* read one line */
    c_lect( xflow->iuv, 1, (void*)buf);
    /* and convert it */
    v2d2uv( buf, u, v, ndimx);
    u += ndimx; v += ndimx;
  }
  DELETE(buf);
}

/**
 * \brief Lecture ligne � ligne pour un tampon vel2d
 *
 * C'est la m�me fonction que xflow_read() mais elle fournit
 * le r�sultat sous la forme d'un tampon 'vel' au lieu de deux
 * tampon 'u' et 'v'. Cette fonction est la plus rapide car
 * vel est le format interne de XFLOW.
 *
 * \param xflow descripteur XFLOW,
 * \param line nombre de lignes � lire,
 * \param uv tampon des composantes du champs de vecteur.
 * \pre le tampon uv doit �tre suffisament grands pour accueillir
 *      les donn�es lues.
 * \return code d'erreur n�gatif.
 *
 * \sa xflow_read()
 */

void xflow_read_v2d( XFLOW *xflow, int line, vel2d *uv) {
  c_lect( xflow->iuv, line, (void*)uv); 
}

/**
 * \brief Lecture plan � plan 
 * 
 * \param xflow Descripteur XFLOW,
 * \param nframe nombre de plan � lire,
 * \param u,v tampons des composantes du champ de vecteurs.
 * Lit un certain nombre de plan dans un flux XFLOW.
 */

void xflow_read_f(XFLOW *xflow, int nframe, float *u, float *v) {
  xflow_read( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, u, v);
}

/**
 * \brief Lecture plan � plan pour un tampon vel2d.
 * \param xflow Descripteur XFLOW,
 * \param nframe nombre de plan � lire,
 * \param uv tampons du champ de vecteurs.
 * Version lecture plan � plan de la fonction xflow_read_v2d()
 */

void xflow_read_f_v2d(XFLOW *xflow, int nframe, vel2d *uv) {
  xflow_read_v2d( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, uv);
}

/** 
 * \brief Lecture par pixel.
 * 
 * Cette fonction donne la valeur du champs de vecteur au point 
 * de cordonn�e (x,y,z). L'acc�s aux valeurs se fait par pixel
 * et nom par ligne ou plan ce qui rend la fonction plus lente que
 * ses consoeurs xflow_read_*(). Toutefois, la ligne lue (qui d�pend
 * de y et z) est tamponn�e de fa�on � rendre la lecture efficace si
 * l'acc�s concerne une m�me ligne dans un m�me plan. 
 * 
 * \todo Choisir la taille (en ligne) du tampon interne de facon
 *       a permettre un acces plus efficace sur un voisinnage.
 *
 * \param xflow descripteur de flux xflow
 * \param x,y,z coordonn�e du point � lire (convention Inrimage, commence
 *              � 1)
 * \param val valeur lue
 * \return code d'erreur negatif ou 0
 */

int xflow_read_value_v2d( XFLOW *xflow, int x, int y, int z, vel2d *val) {
  Fort_int *lfmt = xflow->iuv->lfmt;

  if( x<1 || x>NDIMX ||
      y<1 || y>NDIMY ||
      z<1 || z>NDIMZ )
    return XFLOW_ERR_RANGE;

  if( xflow->curr_l != y+(z-1)*NDIMY) {
    xflow->curr_l = y+(z-1)*NDIMY;
    c_lptset( xflow->iuv, xflow->curr_l);
    xflow_read_v2d( xflow, 1, xflow->buff_l);    
  }
  *val = xflow->buff_l[x-1];
  return XFLOW_ERR_OK;
}


/**
 * \brief Ecriture par pixel. 
 *
 * Ecrit la valeur d'un pixel particulier. 
 * \sa xflow_read_value_v2d()
 * \todo On peut pas encore lire et �crire simultanement en acc�s pixel.
 *       Pour cela, un drapeau qui indique s'il y a eu un acces en �criture
 *       sera necessaire.
 */

int xflow_write_value_v2d( XFLOW *xflow, int x, int y, int z, vel2d *val) {
   Fort_int *lfmt = xflow->iuv->lfmt;
   if( x<1 || x>NDIMX ||
       y<1 || y>NDIMY ||
       z<1 || z>NDIMZ )
     return XFLOW_ERR_RANGE;

   /* On sauvegarde d'abord le tampon si la ligne est diff�rente */
   if( xflow->curr_l != y+(z-1)*NDIMY) {
     if( xflow->curr_l) {
       c_lptset( xflow->iuv, xflow->curr_l);
       xflow_write_v2d( xflow, 1, xflow->buff_l);
     }
     xflow->curr_l = y+(z-1)*NDIMY;
     /* puis on la lit */
     c_lptset( xflow->iuv, xflow->curr_l);
     xflow_read_v2d( xflow, 1, xflow->buff_l);
   }

   xflow->buff_l[x-1] = *val;   
   return XFLOW_ERR_OK;
}

/** 
 * \brief Ecriture par pixel.
 * 
 * Version en acc�s par composante de xflow_write_value_v2d().
 * \param xflow,x,y,z voir xflow_write_value_v2d()
 * \param u,v composantes � �crire
 * \return code d'erreur negatif ou 0
 */
int xflow_write_value( XFLOW *xflow, int x, int y, int z, float u, float v) {
  vel2d uv = {u,v};
  return xflow_write_value_v2d( xflow, x, y, z, &uv);
}

/** 
 * \brief Lecture par pixel.
 * 
 * Version en acc�s par composante de xflow_read_value_v2d().
 * \param xflow,x,y,z voir xflow_read_value_v2d()
 * \param u,v composantes � lire
 * \return code d'erreur negatif ou 0
 */
int xflow_read_value( XFLOW *xflow, int x, int y, int z, float *u, float *v) {
  vel2d uv;
  int res = xflow_write_value_v2d( xflow, x, y, z, &uv);
  *u = uv.u; *v = uv.v;
  return res;
}

/**
 * \brief Positionnement au d�but d'une ligne.
 *
 * Positionne un flux XFLOW au d�but d'une line.
 *
 * \param xflow flux � positionner,
 * \param line Num�ro de la ligne (commence � 1)
 * \return code d'erreur n�gatif
 */

int xflow_seek( XFLOW *xflow, int line) {
  Fort_int *lfmt = xflow->iuv->lfmt;

  if( line > 0 && line <= DIMY) {
    if( xflow->iuv->f_type & FL_PIPE &&  line <= xflow->iuv->no_ligne ) {
      /* R�ouverture du fichier en mode tube */
      char *str = strdup( xflow->iuv->nom);
      fermnf_( &xflow->iuv);
      xflow->iuv = image_( str, "e", "", (void*)lfmt);
      free( str);
    }      
    c_lptset( xflow->iuv, line);
    return XFLOW_ERR_OK;
  } else
    return XFLOW_ERR_RANGE;  /* plan non trouv� */
}

/**
 * \brief Positionnement au d�but d'un plan.
 *
 * Positionne un flux XFLOW au d�but d'un plan.
 *
 * \param xflow flux � positionner,
 * \param frame index du plan � chercher
 * \return code d'erreur n�gatif
 */

int xflow_seek_f( XFLOW *xflow, int frame) {
  Fort_int *lfmt = xflow->iuv->lfmt;

  if( frame > 0 && frame <= NDIMZ) {
    xflow_seek( xflow, (frame-1) * NDIMY + 1);
    //    c_lptset( xflow->iuv, (frame-1) * NDIMY + 1);
    return XFLOW_ERR_OK;
  } else
    return EDOM;  /* plan non trouv� */
}

/**
 * Retourne la vrai taille du tampon pour stocker un
 * plan du champ de vecteurs. Pour un tampon de type
 * vel2d, il faut multiplier cette taille par sizeof(float)
 * ou sizeof(vel2d) selon le type de tampon.
 *
 * \param xflow flux concern�,
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
 * \brief Ouverture en lecture/�criture d'un fichier XFLOW.
 *
 * Ouverture d'un fichier XFLOW d�j� existant. On peut y �crire
 * et y lire sans toucher aux lignes ou pixels non acc�d�s. Pour
 * cr�er et �crire exclusivement dans un fichier XFLOW, on utilise
 * la fonction xflow_create().
 *
 * \param fname nom de l'image � la convention XFLOW2,
 * \return le descripteur de flux XFLOW ou \b NULL si image non
 *   pr�sente ou format incompatible.
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
 * \brief Ouverture en lecture/�criture d'un fichier XFLOW avec
 *        v�rification des dimensions.
 *
 * \param fname nom de l'image � la convention XFLOW2,
 * \param lfmt format d'une image dont on comparera les dimensions,
 * \return le descripteur de flux XFLOW ou \b NULL si image non
 *   pr�sente ou format incompatible ou dimensions incompatibles.
 * \sa xflow_open(), xflow_open_test().
 */

XFLOW *xflow_open_verif( char *fname, Fort_int lfmt[]) {
  XFLOW *xflow;
  struct image *nf;
  Fort_int gfmt[9]; /* lfmt peut �tre modifi� par xflow_open_test()
		     * on en fait une copie */

  /* ouverture image en mode v�rification de la taille */
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
 *        avec v�rification des dimensions si demand�e.
 * 
 * \param fname nom du fichier image
 * \param gfmt tableau de format (voir explication ci-dessous)
 * \param verif si \b TRUE, les dimensions de l'image ouverte
 *              sont compar�es � celles fournit par le param�tre <i>gfmt</i>.
 * \return un descripteur XFLOW ou INRIMAGE ou \b NULL.
 * \sa xflow_open()
 *
 * Cette fonction est similaire � xflow_open(). Elle ouvre dabord
 * l'image. Si cette image est dans un format compatible avec XFLOW
 * alors elle retourne un descripteur \b XFLOW sinon elle retourne
 * un descripteur INRIMAGE classique (\b struct \b image *). 
 *
 * Le param�tre <i>gfmt</i> a donc deux r�les :
 *   - en entr�e : si le param�tre <i>verif</i> est mis a \b TRUE,
 *     l'image est ouverte en mode v�rification des dimensions. Le
 *     param�tre <i>gfmt</i> doit donc contenir des dimensions coh�rentes.
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
  
  /* v�rifier les dimensions sauf composantes */
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
      // verif num�ro de version a faire
    } else {
      // Fichier sans num�ro de version (cr�� � la main)
      // on force un num�ro de version
      xflow->version.major = XFLOW_MAJOR;
      xflow->version.minor = XFLOW_MINOR;
      xflow->version.release = XFLOW_RELEASE;
    }
    
    if( igethline( xflow->iuv, "XFLOW_IMAGE=", 0, buff, 255, &dum) != -1)
      strcpy( xflow->i_name, buff);
    
    gfmt[I_DIMX] = 0; /* pour d�cider de la nature du retour */
    return xflow;
  } else {
    memcpy( gfmt, lfmt, sizeof(Fort_int)*9);
    return ptrimg; 
  }
}

/**
 * \brief Donne le num�ro de version d'un fichier XFLOW.
 *
 * Retourne le num�ro de version du format Xflow d'un flux pour tester
 * des �ventuelles �volutions possible du format. Le fichier
 * ent�te de xflow (xflow.h) d�fini trois constantes qui 
 * caract�rise la version actuelle de la librairie.
 */

void xflow_version( XFLOW *xflow, int *major, int *minor, int *release) {
  *major = xflow->version.major;
  *minor = xflow->version.minor;
  *release = xflow->version.release;
}

/**
 * \brief Cr�ation d'un fichier XFLOW.
 *
 * Ouvre en �criture un fichier de champs de vecteur au format 
 * XFLOW.
 * 
 * \param fname : nom du fichier � cr�er.
 * \param x,y,z : dimension du champs de vecteur. Doit �tre compatible
 *        avec celle d'une image de r�f�rence qui sera affich� par le
 *        programme de visualisation xflow.
 * \return descripteur sur le fichier cr��.
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

/**
 * \brief Associe une image de r�f�rence � un flux XFLOW.
 *
 * \param xflow descripteur XFLOW,
 * \param image nom de l'image r�f�rente,
 * \return un code d'erreur.
 *
 * Associe une image de reference � un champ de vecteur. Les
 * dimensions doivent �tre compatible avec celle du descripteur
 * XFLOW. La fonction le v�rifie et retourne �ventuellement une
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

/**
 * \brief Ouverture de l'image r�f�rente.
 *
 * Ouvre l'image r�f�rente, si elle existe, associ�e � un descripteur XFLO
 * \param xflow descripteur XFLOW,
 * \param lfmt tableau du format de l'image,
 * \return descripteur INRIMAGE de l'image r�f�rente ou NULL si erreur.
 */

struct image * xflow_get_image( XFLOW *xflow, Fort_int lfmt[]) {
  struct image *img;
  
  if (is_inrimage(xflow->i_name, 2))
    return image_(xflow->i_name, "e", " ", (void*)lfmt);
  else
    return NULL;
}

/**
 * \brief Noms des fichiers associ�s.
 *
 * Retourne les noms des fichiers associ�s � un flux XFLOW.
 *
 * \param xflow flux concern�e,
 * \param mode 
 *    - XFLOW_NAME_IMAGE nom du fichier image associ�
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

/**
 * \brief Ecriture ligne � ligne dans un tampon vel2d.
 *
 * \param xflow descripteur XFLOW,
 * \param line  nombre de lignes � �crire,
 * \param uv tampon du champ de vecteurs.
 *
 * C'est la m�me fonction que xflow_write() mais elle travaille
 * un tampon 'vel2d' au lieu de deux
 * tampon float 'u' et 'v'. Cette fonction est la plus rapide car
 * vel2d est le format interne de XFLOW.
 */

void xflow_write_v2d(XFLOW *xflow, int line, vel2d *uv) {
  c_ecr( xflow->iuv, line, (void*)uv); 
}

/**
 * \brief Ecriture ligne � ligne.
 *
 * Ecriture dans un flux XFLOW. L'�criture se fait par ligne (comme
 * INRIMAGE).
 * \param xflow flux ouvert par la fonction xflow_open(),
 * \param line nombre de lignes � �crire,
 * \param u,v tampon des composantes du champs de vecteur.
 * \return code d'erreur n�gatif.
 */

void xflow_write( XFLOW *xflow, int line, float *u, float *v) {
  Fort_int *lfmt = xflow->iuv->lfmt;
  vel2d *buf = NEW( vel2d, NDIMX);
  
  while( line -- ) {
    /* Convert line to vel2d format */
    uv2v2d( u, v, buf, NDIMX);
    /* and write one line */
    c_ecr( xflow->iuv, 1, (void*)buf); 
    u += NDIMX;
    v += NDIMX;
  }
  DELETE(buf);
}

/**
 * \brief Ecriture plan � plan.
 *
 * \param xflow descripteur XFLOW,
 * \param nframe nombre de plans � �crire,
 * \param u,v tampons des composantes horizontales et verticales du champ de vecteurs.
 * Version �criture plan � plan de la fonction xflow_write_v2d()
 */

void xflow_write_f(XFLOW *xflow, int nframe, float *u, float *v) {
  xflow_write( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, u, v);
}

/** 
 * \brief Ecriture plan � plan dans un tampon vel2d.
 *
 * \param xflow descripteur XFLOW,
 * \param nframe nombre de plans � �crire,
 * \param uv tampon du champ de vecteurs.
 * Version �criture plan � plan de la fonction xflow_write_v2d()
 */

void xflow_write_f_v2d(XFLOW *xflow, int nframe, vel2d *uv) {
  xflow_write_v2d( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, uv);
}

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

/* EOF */

