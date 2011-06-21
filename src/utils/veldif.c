/**
 * Différence de deux champ de vecteur : mesure d'erreur
 * Voir aussi ''errors''
 * obsolete et jamais utilisé
 * va etre remplacé par velerr / velnorme (a voir)
 */

#include <inrimage/xflow.h>
#include <math.h>

int main( int argc, char **argv) {
  XFLOW *u1, *u2;
  XFLOW *err;
  char name[256];
  Fort_int lfmt[9];
  vel2d *uv1, *uv2;
  int i;

  inr_init( argc, argv, "", "", "");
  
  infileopt( name);
  u1 = xflow_open( name);
  xflow_get_lfmt( u1, lfmt);

  infileopt( name);
  u2 = xflow_open_verif( name, lfmt);
  
  outfileopt( name);
  err = xflow_create( name, NDIMX, NDIMY, NDIMZ);
  xflow_set_image( err, u1->i_name);

  uv1 = (vel2d *)i_malloc( sizeof(vel2d)*NDIMX);
  uv2 = (vel2d *)i_malloc( sizeof(vel2d)*NDIMX);

  while ( DIMY --) {
    xflow_read_v2d( u1, 1, uv1);
    xflow_read_v2d( u2, 1, uv2);

    for( i=0; i<NDIMX; i++) {
      float n1, n2, u1u2, n1n2;
      
      n1 = uv1[i].u * uv1[i].u + uv1[i].v * uv1[i].v;
      n2 = uv2[i].u * uv2[i].u + uv2[i].v * uv2[i].v;

      // cos(u1,u2)
      u1u2 = (uv1[i].u * uv2[i].u + uv1[i].v * uv2[i].v) / sqrt(n1*n2+0.0001);

      // ||u1-u2|| 
      n1n2 = (uv2[i].u-uv1[i].u)*(uv2[i].u-uv1[i].u) + (uv2[i].v-uv1[i].v)*(uv2[i].v-uv1[i].v);

      uv1[i].u = n1n2 * u1u2;
      uv1[i].v = n1n2 * sqrt(1-u1u2*u1u2);

    }
    
    xflow_write_v2d( err, 1, uv1);
  }
  xflow_close( err);
  xflow_close( u1);
  xflow_close( u2);
  i_Free( (void*)&uv1);
  i_Free( (void*)&uv2);
  return 0;
}
