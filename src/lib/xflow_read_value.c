/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>

/**
 * \brief Lecture par pixel.
 * 
 * Cette fonction donne la valeur du champs de vecteur au point 
 * de cordonnée (x,y,z). L'accès aux valeurs se fait par pixel
 * et nom par ligne ou plan ce qui rend la fonction plus lente que
 * ses consoeurs xflow_read_*(). Toutefois, la ligne lue (qui dépend
 * de y et z) est tamponnée de façon à rendre la lecture efficace si
 * l'accès concerne une même ligne dans un même plan. 
 * 
 * \todo Choisir la taille (en ligne) du tampon interne de facon
 *       a permettre un acces plus efficace sur un voisinnage.
 *
 * \param xflow descripteur de flux xflow
 * \param x,y,z coordonnée du point à lire (convention Inrimage, commence
 *              à 1)
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
 * \brief Lecture par pixel.
 * 
 * Version en accès par composante de xflow_read_value_v2d().
 * \param xflow,x,y,z voir xflow_read_value_v2d()
 * \param u,v composantes à lire
 * \return code d'erreur negatif ou 0
 */
int xflow_read_value( XFLOW *xflow, int x, int y, int z, float *u, float *v) {
  vel2d uv;
  int res = xflow_write_value_v2d( xflow, x, y, z, &uv);
  *u = uv.u; *v = uv.v;
  return res;
}


