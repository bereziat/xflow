#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "data.h"
#include "utils.h"

/**
 * ajout d'un descripteur de donnée dans la liste
 * des données de l'API.
 * @param mode ajoute à la fin (1) ou au debut (0) de la liste chainée
 */

void data_insert( XFLOW_API *api, XFLOW_DATA *data, int mode) {
  if( mode == 0) {
    data->next = api->data;
    api->data = data;
  } else {
    data->next = NULL;
    if( api->data == NULL) {
      api->data = data;
    } else {
      XFLOW_DATA *pdata = api->data;
      
      while( pdata->next) pdata = pdata->next;
      pdata->next = data;
    }
  }
}

static int test_dims( XFLOW_API *api, int dimx, int dimy) {
  if( api->wimg ==  0) {
    api->wimg = api->wwin = dimx;
    api->himg = api->hwin = dimy;
    return 1;
  }
  if( api->wimg != dimx || api->himg != dimy)
    return 0;
}

/**
 * Ajoute un champ/une image dans 
 * retourne :
 *  -3: erreur: dimensions dimx, dimy, incompatibles
 *  -2: erreur, filename n'est pas une image valide.
 * @FIXME -1: erreur, image déjà chargée. 
 *   0: filename est une image de fond.
 *   1: filename est un champ de vecteurs.
 */ 

int data_addimage( XFLOW_API *api, char *filename) {
  Fort_int lfmt[9];
  XFLOW *xfl = xflow_open_test( filename, lfmt, 0);
  XFLOW_DATA *data;
  static int colordef = RED;

  if( !xfl) return -2;
  
  if( *lfmt) { /* C'est une image de fond  */
    struct image *img;
    img = (struct image *)xfl;

    if( test_dims( api, NDIMX, NDIMY) == 0) return -3;
    if( api->zmax < NDIMZ) api->zmax = NDIMZ;

    data = NEW(XFLOW_DATA,1);
    data->type = DATA_IMAGE;
    typescrn( img, &data->data.image.display, data->data.image.pal);	
    assert( data->data.image.display != SCRN_UNSUPPORTED);
    data->data.image.file = img;
    data->data.image.read = NEW(unsigned char, DIMX*NDIMY);
    data->data.image.buf = NEW(unsigned char,api->wwin*api->hwin*NDIMV);
        
    /* Les images sont insérées en début */
    data_insert( api, data, 1);
    return 0;    
  } else {    
    struct image *img = xflow_get_image( xfl, lfmt);
    int w,h,z;

    if( img) {
      if( api->zmax < NDIMZ) api->zmax = NDIMZ;
      if( test_dims( api, NDIMX, NDIMY) == 0) return -3;
      
      data = NEW(XFLOW_DATA,1);
      data->type = DATA_IMAGE;
      data->data.image.file = img;
      typescrn( img, &data->data.image.display, data->data.image.pal);
      assert( data->data.image.display != SCRN_UNSUPPORTED);
      data->data.image.read = NEW(unsigned char, DIMX*NDIMY);
      data->data.image.buf  = NEW(unsigned char,api->wwin*api->hwin*NDIMV);	  
      
      data_insert( api, data, 1);
    } else
      if(debug) 
	fprintf( stderr, "Attention : fichier image associé non trouvé\n");
    
    xflow_get_dims( xfl, &w, &h, &z);
    
    if( test_dims( api, w, h) == 0) return -3;
    if( api->zmax < z) api->zmax = z;
    
    data = NEW(XFLOW_DATA,1);
    data->type = DATA_XFLOW;
    data->data.xflow.hide   = FALSE;
    data->data.xflow.norma  = FALSE;
    data->data.xflow.smooth = FALSE;
    data->data.xflow.arrowcolor = (colordef++) % MAX_COLORS; // FIXME colordef
    data->data.xflow.arrowsize  = 1; 
    data->data.xflow.arrowwidth = 1;
    data->data.xflow.arrowstyle = 0;
    data->data.xflow.file = xfl;
    xflow_get_lfmt( xfl, lfmt);

    data->data.xflow.buf    = NEW(vel2d,NDIMX*NDIMY);   
    data->data.xflow.magbuf = NEW(unsigned char,NDIMX*NDIMY);
    data->data.xflow.rotbuf = NEW(unsigned char,NDIMX*NDIMY);
    data->data.xflow.divbuf = NEW(unsigned char,NDIMX*NDIMY);
    data->data.xflow.hsvbuf = NEW(unsigned char,3*NDIMX*NDIMY);
    
    /* Les données sont insérées en fin */
    data_get_settings( api, data);
    data_insert( api, data, 1);  

    return 1;
  }
}

/* Lit un plan des données;
 * z commence à 1 */
void data_read( XFLOW_API *api, int z) {
  XFLOW_DATA *pd;
  
  if( z == api->zpos)
    return;

  for( pd = api->data; pd; pd = pd->next) {
    if( pd->type == DATA_IMAGE) {
      Fort_int *lfmt, ifmt[9];   
      int do_zoom;

      lfmt = pd->data.image.file->lfmt;
      do_zoom = (NDIMX != api->wwin) || (NDIMY != api->hwin);

      if( NDIMZ>1 || z == 1) {
      /* Lecture du tampon */
      if( z-1 != api->zpos) { 
	/* réouverture de l'image si on est en tube */
	if( z<=api->zpos && pd->data.image.file->f_type & FL_PIPE) {
	  char *str = strdup(pd->data.image.file->nom);

	  fermnf_( &pd->data.image.file);
	  pd->data.image.file = image_( str, "e", " ", (void*)ifmt);
	  lfmt = pd->data.image.file->lfmt;
	  free(str);
	}
      	c_lptset( pd->data.image.file, 1 + (z-1)*NDIMY);
      }
      
      lecscrn( pd->data.image.file, NDIMY, pd->data.image.read,
	       pd->data.image.display, pd->data.image.pal);
      }
      if( do_zoom) {
	if(debug) fputs( "data_read: zoom", stderr);
	memcpy( ifmt, lfmt, 9*sizeof(Fort_int));
	ifmt[I_TYPE] = FIXE;
	ifmt[I_BSIZE] = 1;
	utils_zoom( pd->data.image.read, pd->data.image.buf, 
		    ifmt, api->wwin, api->hwin);	
      } else 
	memcpy( pd->data.image.buf, pd->data.image.read, DIMX*NDIMY);

    } else { /* DATA_XFLOW */
      int w, h, d;

      pd->data.xflow.normsup = 0;
      if( z-1 != api->zpos ) 
	xflow_seek_f( pd->data.xflow.file, z);
      xflow_read_f_v2d( pd->data.xflow.file, 1, pd->data.xflow.buf);
      xflow_get_dims( pd->data.xflow.file, &w, &h, &d);
      utils_normsup( pd->data.xflow.buf, w*h, &pd->data.xflow.normsup);
      
      utils_mag( pd->data.xflow.buf, pd->data.xflow.magbuf,
		 w, h, api->wwin, api->hwin, 
		 &pd->data.xflow.magmin, &pd->data.xflow.magmax);

      utils_div( pd->data.xflow.buf, pd->data.xflow.divbuf,
		 w, h, api->wwin, api->hwin,
		 &pd->data.xflow.divmin, &pd->data.xflow.divmax);

      utils_rot( pd->data.xflow.buf, pd->data.xflow.rotbuf,
		 w, h, api->wwin, api->hwin,
		 &pd->data.xflow.rotmin, &pd->data.xflow.rotmax);

      pd->data.xflow.hsv_sat = pd->data.xflow.magmax;
      utils_hsv( pd->data.xflow.buf, pd->data.xflow.hsvbuf,
		 w, h, api->wwin, api->hwin,
		 pd->data.xflow.hsv_sat); 
    }
  }
  api->zpos = z;
}

void data_free( XFLOW_API *api) {
  XFLOW_DATA *next,*curr;

  curr = api->data;
  while( curr) {
    switch( curr->type) {
    case DATA_IMAGE:
      DELETE( curr->data.image.buf);
      fermnf_( &curr->data.image.file);
      break;
    case DATA_XFLOW:
      DELETE( curr->data.xflow.buf);
      DELETE( curr->data.xflow.magbuf);
      DELETE( curr->data.xflow.divbuf);
      DELETE( curr->data.xflow.rotbuf);
      DELETE( curr->data.xflow.hsvbuf);
      fermnf_( (void*)&curr->data.xflow.file->iuv);
      break;
    }
    next=curr->next;
    DELETE(curr);
    curr=next;
  }
}


/** 
 * retreive settings from a vector file
 */
void data_get_settings( XFLOW_API *api, XFLOW_DATA *pd) {
  struct image *nf ;
  char buf[64];
  int dum;

  if( pd->type == DATA_XFLOW) {
    nf = pd->data.xflow.file->iuv;
    if( igethline( nf, "XFLOW_PARAMS=", 1, buf,64,&dum) >= 0)
      sscanf( buf, "%d %f %f %f", &api->sample, &api->scale,
	      &api->thresh, &api->thresh_high);
    /* IL EN MANQUE */
  }  
}
