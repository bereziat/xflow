/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>

#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))
#define DELETE(var)    i_Free((char **)&var)

/* copie deux tampons float vers un tampon vel2d */
static inline void uv2v2d( float *u, float *v, vel2d *uv, int count) {
  while( count --) {
    uv->u = *u ++;
    uv->v = *v ++;
    uv++;
  }
}

/**
 * \brief Ecriture ligne à ligne dans un tampon vel2d.
 *
 * \param xflow descripteur XFLOW,
 * \param line  nombre de lignes à écrire,
 * \param uv tampon du champ de vecteurs.
 *
 * C'est la même fonction que xflow_write() mais elle travaille
 * un tampon 'vel2d' au lieu de deux
 * tampon float 'u' et 'v'. Cette fonction est la plus rapide car
 * vel2d est le format interne de XFLOW.
 */

void xflow_write_v2d(XFLOW *xflow, int line, vel2d *uv) {
  c_ecr( xflow->iuv, line, (void*)uv); 
}

/**
 * \brief Ecriture ligne à ligne.
 *
 * Ecriture dans un flux XFLOW. L'écriture se fait par ligne (comme
 * INRIMAGE).
 * \param xflow flux ouvert par la fonction xflow_open(),
 * \param line nombre de lignes à écrire,
 * \param u,v tampon des composantes du champs de vecteur.
 * \return code d'erreur négatif.
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
 * \brief Ecriture plan à plan.
 *
 * \param xflow descripteur XFLOW,
 * \param nframe nombre de plans à écrire,
 * \param u,v tampons des composantes horizontales et verticales du champ de vecteurs.
 * Version écriture plan à plan de la fonction xflow_write_v2d()
 */

void xflow_write_f(XFLOW *xflow, int nframe, float *u, float *v) {
  xflow_write( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, u, v);
}

/**
 * \brief Ecriture plan à plan dans un tampon vel2d.
 *
 * \param xflow descripteur XFLOW,
 * \param nframe nombre de plans à écrire,
 * \param uv tampon du champ de vecteurs.
 * Version écriture plan à plan de la fonction xflow_write_v2d()
 */

void xflow_write_f_v2d(XFLOW *xflow, int nframe, vel2d *uv) {
  xflow_write_v2d( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, uv);
}

