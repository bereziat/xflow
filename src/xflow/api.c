/*
 * Fonctions pour l'API xflow
 * $Id: api.c,v 1.6 2010/10/18 19:47:42 bereziat Exp $
 */
#include <math.h>
#include "support.h"
#include "data.h"

/*
 * met a jour les widgets selon les paramètres de l'API.
 */

void xflow_api_update_widget( XFLOW_API *api) {
  GtkWidget *widget;
  GtkAdjustment *adj;
  float digit, normsup;
  XFLOW_DATA *pd;
  Fort_int lfmt[9];

  normsup = 0;
  for( pd=api->data; pd; pd=pd->next)
    if( pd->type == DATA_XFLOW && normsup < pd->data.xflow.normsup) {
      xflow_get_lfmt( pd->data.xflow.file, lfmt);
      normsup = MIN(pd->data.xflow.normsup, MAX(NDIMX,NDIMY));
    }
  /* sample */
  adj = gtk_range_get_adjustment( GTK_RANGE(lookup_widget(api->mainwindow,"xflow_sample")));
  adj -> value = api->sample;
  gtk_adjustment_changed( adj);

  /* scale */
  adj = gtk_range_get_adjustment( GTK_RANGE(lookup_widget(api->mainwindow,"xflow_scale")));
  adj -> value = api->scale;
  gtk_adjustment_changed( adj);

  /* seuillage bas */
  widget = lookup_widget( api->mainwindow, "xflow_thresh");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));

  adj -> upper = normsup+0.01; 
  adj -> lower = 0;
  adj -> step_increment = normsup / 100;
  adj -> page_increment = normsup / 10;
  adj -> value = api->thresh;
  if( adj->value < 0) adj->value = adj->lower;

  gtk_scale_set_digits (GTK_SCALE (widget), 3);
  gtk_adjustment_changed( adj);

  /* seuillage haut */
  widget = lookup_widget( api->mainwindow, "xflow_thresh_high");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));

  adj -> upper = normsup+0.01;
  adj -> lower = 0;
  adj -> step_increment = normsup / 100;
  adj -> page_increment = normsup / 10;
  adj -> value = api->thresh_high;
  if( adj->value > adj->upper ) adj->value = adj->upper;

  // printf( "%f\n", digit = log(api->normsup)/log(2.));
  gtk_scale_set_digits (GTK_SCALE (widget), 3);
  gtk_adjustment_changed( adj);

  /* scroller vertical des plans */
  widget = lookup_widget( api->mainwindow, "xflow_zscroll");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  adj -> upper = api->zmax+1;
  gtk_adjustment_changed( adj);
  
  /* scroller vertical des plans */
  widget = lookup_widget( api->mainwindow, "xflow_zscroll");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  adj -> upper = api->zmax+1;
  gtk_adjustment_changed( adj);

  /* zoom */  
  widget = lookup_widget( api->mainwindow, "xflow_zoom");
  adj = gtk_range_get_adjustment( GTK_RANGE(widget));
  adj -> value = api->zoom;
  gtk_adjustment_value_changed( adj);
}


void xflow_api_refresh_drawing_areas( XFLOW_API *api) {
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_thresh"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_thresh_high"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_drawing"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_mag"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_div"));
  gtk_widget_queue_draw( lookup_widget(api->mainwindow, "xflow_rot"));
}
