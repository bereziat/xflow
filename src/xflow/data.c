#include <stdio.h>
#include <math.h>
#include <string.h>
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

/* Lit un plan des données */
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
      if( pd->data.xflow.magbuf)
	utils_mag( pd->data.xflow.buf, pd->data.xflow.magbuf,
		   w, h, api->wwin, api->hwin, pd->data.xflow.normsup);
      if( pd->data.xflow.divbuf)
	utils_div( pd->data.xflow.buf, pd->data.xflow.divbuf,
		   w, h, api->wwin, api->hwin);
      if( pd->data.xflow.rotbuf)
	utils_rot( pd->data.xflow.buf, pd->data.xflow.rotbuf,
		   w, h, api->wwin, api->hwin);
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
      fermnf_( (void*)&curr->data.xflow.file->iuv);
      break;
    }
    next=curr->next;
    DELETE(curr);
    curr=next;
  }
}


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
