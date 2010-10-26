/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>

#define NEW(type,elem) (type *)i_malloc(sizeof(type)*(elem))
#define DELETE(var)    i_Free((char **)&var)

/* copie un tampon vel2d vers deux tampons float */
static inline 
void v2d2uv( vel2d *uv, float *u, float *v, int count) {
  while( count --) {
    *u ++ = uv->u;
    *v ++ = uv->v;
    uv ++;
  }
}

/**
 * \brief Lecture ligne à ligne.
 *
 * Lecture dans un flux XFLOW. La lecture se fait par ligne (comme
 * INRIMAGE).
 * \param xflow descripteur XFLOW,
 * \param line nombre de lignes à lire,
 * \param u,v tampon des composantes du champs de vecteur.
 * \pre les tampons u et v doivent être suffisament grands pour accueillir
 *      les données lues.
 * \return code d'erreur négatif.
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
 * \brief Lecture ligne à ligne pour un tampon vel2d
 *
 * C'est la même fonction que xflow_read() mais elle fournit
 * le résultat sous la forme d'un tampon 'vel' au lieu de deux
 * tampon 'u' et 'v'. Cette fonction est la plus rapide car
 * vel est le format interne de XFLOW.
 *
 * \param xflow descripteur XFLOW,
 * \param line nombre de lignes à lire,
 * \param uv tampon des composantes du champs de vecteur.
 * \pre le tampon uv doit être suffisament grands pour accueillir
 *      les données lues.
 * \return code d'erreur négatif.
 *
 * \sa xflow_read()
 */

void xflow_read_v2d( XFLOW *xflow, int line, vel2d *uv) {
  c_lect( xflow->iuv, line, (void*)uv); 
}

/**
 * \brief Lecture plan à plan 
 * 
 * \param xflow Descripteur XFLOW,
 * \param nframe nombre de plan à lire,
 * \param u,v tampons des composantes du champ de vecteurs.
 * Lit un certain nombre de plan dans un flux XFLOW.
 */

void xflow_read_f(XFLOW *xflow, int nframe, float *u, float *v) {
  xflow_read( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, u, v);
}

/**
 * \brief Lecture plan à plan pour un tampon vel2d.
 * \param xflow Descripteur XFLOW,
 * \param nframe nombre de plan à lire,
 * \param uv tampons du champ de vecteurs.
 * Version lecture plan à plan de la fonction xflow_read_v2d()
 */

void xflow_read_f_v2d(XFLOW *xflow, int nframe, vel2d *uv) {
  xflow_read_v2d( xflow, xflow->iuv->lfmt[I_NDIMY]*nframe, uv);
}
