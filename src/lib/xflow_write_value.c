/*
 *  Dominique Béréziat (c) 2001 INRIA/Projet AIR
 *  Librairie xflow : routines de lecture/sauvegarde des fichiers XFLOW
 *  version 1.00
 */

#include <inrimage/image.h>
#include <inrimage/xflow.h>

/**
 * \brief Ecriture par pixel. 
 *
 * Ecrit la valeur d'un pixel particulier. 
 * \sa xflow_read_value_v2d()
 * \todo On peut pas encore lire et écrire simultanement en accès pixel.
 *       Pour cela, un drapeau qui indique s'il y a eu un acces en écriture
 *       sera necessaire.
 */

int xflow_write_value_v2d( XFLOW *xflow, int x, int y, int z, vel2d *val) {
   Fort_int *lfmt = xflow->iuv->lfmt;
   if( x<1 || x>NDIMX ||
       y<1 || y>NDIMY ||
       z<1 || z>NDIMZ )
     return XFLOW_ERR_RANGE;

   /* On sauvegarde d'abord le tampon si la ligne est différente */
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
 * Version en accès par composante de xflow_write_value_v2d().
 * \param xflow,x,y,z voir xflow_write_value_v2d()
 * \param u,v composantes à écrire
 * \return code d'erreur negatif ou 0
 */
int xflow_write_value( XFLOW *xflow, int x, int y, int z, float u, float v) {
  vel2d uv = {u,v};
  return xflow_write_value_v2d( xflow, x, y, z, &uv);
}
