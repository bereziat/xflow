#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <math.h>
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "data.h"


extern XFLOW_API api;

static int export = FALSE;

void
on_export_destroy                      (GtkObject       *object,
                                        gpointer         user_data)
{
  export = FALSE;
}


void
on_export_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *widget, *txt;
  GtkWidget *obj;
  XFLOW_DATA *pd;
  char name[256], *p;

  if( export == TRUE) return;
  export = TRUE;
  
  widget = create_export();
  
  gtk_widget_hide( lookup_widget( widget, "export_separ1"));
  gtk_widget_hide( lookup_widget( widget, "export_separ2"));

  gtk_widget_hide( lookup_widget( widget, "export_jpeg_quality"));
  gtk_widget_hide( lookup_widget( widget, "export_jpeg_label"));
  gtk_widget_hide( lookup_widget( widget, "export_vbitrate"));
  gtk_widget_hide( lookup_widget( widget, "export_vbitrate_label"));
  gtk_widget_hide( lookup_widget( widget, "export_codec"));
  gtk_widget_hide( lookup_widget( widget, "export_codec_label"));

  txt = lookup_widget( GTK_WIDGET(widget), "export_file");
  gtk_combo_box_set_active( GTK_COMBO_BOX(lookup_widget(widget, "export_unit")), 0);
  gtk_combo_box_set_active( GTK_COMBO_BOX(lookup_widget(widget, "export_type")), 0);
  gtk_combo_box_set_active( GTK_COMBO_BOX(lookup_widget(widget, "export_codec")), 0);
  
  for( pd = api.data; pd; pd  = pd->next) {   
    
    switch( pd->type) {
    case DATA_XFLOW:
      strcpy( name, pd->data.xflow.file->iuv->nom);
      utils_basename( name);
      gtk_entry_set_text( GTK_ENTRY(txt), name);
      break;
    case DATA_IMAGE:
      strcpy( name, pd->data.image.file->nom);
      utils_basename( name);
      gtk_entry_set_text( GTK_ENTRY(txt), name);
      break;
    }
  }

  gtk_widget_show(widget); 
}

void
on_export_apply_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *widget;
  XFLOW_DATA *pd;
  char command[512];
  const gchar *p;
  Fort_int *lfmt;
  char *type[] = { "eps", "ps", "tiff", "jpeg"};
  int size;
  char *unit[] = { "cm", "in"};
  char *codec[] = { "mpjeg", "mpegvideo1", "mpegvideo1", "msmpeg4", "mpeg4"};
  int bgimg = 0;

  widget = lookup_widget( GTK_WIDGET(button), "export_file");
  p = gtk_entry_get_text( GTK_ENTRY(widget));
  if( *p) {
    int export_type;
    
    /* options globales */
    widget = lookup_widget( GTK_WIDGET(button), "export_type");
    export_type = gtk_combo_box_get_active( GTK_COMBO_BOX(widget));
    // printf( "export_type=%d\n", export_type);

    switch( export_type) {
    case 4:
      sprintf( command, "vel2mpg -o %s.mpg", p);
      break;
    case 5:
      sprintf( command, "vel2mpg -o %s.inr", p);      
      break;
    case 6:
      sprintf( command, "vel2mpg -o %s.avi -codec %s -vbr %d", p, 
	       codec[gtk_combo_box_get_active( GTK_COMBO_BOX( lookup_widget( GTK_WIDGET(button), "export_codec")))],
	       gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( GTK_WIDGET(button), "export_vbitrate")))
	       );      
      break;
    case 7:
      sprintf( command, "vel2mpg -o %s.gif", p);
      break;
    default:
      sprintf( command, "vel2fig -o %s -iz %d -type %s", p, api.zpos, type[export_type]);
    }

    /* ces options peuvent passer en locale */
    sprintf( command, "%s -scale %f -sample %d -tl %f -th %f", command,
	     api.scale, api.sample, api.thresh, api.thresh_high);
    
    /* autres options globales */
    
    widget = lookup_widget( GTK_WIDGET(button), "export_jpeg_quality");
    p = gtk_entry_get_text( GTK_ENTRY(widget));
    size = atoi( p);
    if( size) sprintf( command, "%s -q %d", command, size);
    
    widget = lookup_widget( GTK_WIDGET(button), "export_size");
    p = gtk_entry_get_text( GTK_ENTRY(widget));
    size = atoi( p);
    if( size) {
      widget = lookup_widget( GTK_WIDGET(button), "export_unit");
      sprintf( command, "%s -size %d%s", command, size, 
	       unit[gtk_combo_box_get_active( GTK_COMBO_BOX(widget))]);
    }
    strcat( command, " ");
    
    for( pd = api.data; pd; pd  = pd->next) {   
      switch( pd->type) {
      case DATA_IMAGE:
	if( bgimg == 0 ) {
	  lfmt = pd->data.image.file->lfmt;
	  sprintf( command, "%s %s", command, pd->data.image.file->nom);
	  bgimg = 1;
	}
	break;
      case DATA_XFLOW:
	if( pd->data.xflow.hide) continue;
	
	sprintf( command, "%s %s -acolor %s -asize %s -awidth %d -astyle %d %s", command,
		 pd->data.xflow.file->iuv->nom, 
		 color_name( &api, pd->data.xflow.arrowcolor),
		 size_name( &api, pd->data.xflow.arrowsize),
		 pd->data.xflow.arrowwidth, pd->data.xflow.arrowstyle,
		 pd->data.xflow.smooth?"-smooth":"");
	
	break;
      }
    }
    
    puts( command);
    system ( command);
    
    /* Post traitement */
    
    widget = lookup_widget( GTK_WIDGET(button), "export_inter");
    if( !gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget))) {
      widget = lookup_widget( GTK_WIDGET(button), "export_file");
      p = gtk_entry_get_text( GTK_ENTRY(widget));
      sprintf( command, "%s-%d.gif", p,api.zpos);
      unlink( command);
      sprintf( command, "%s-%d.fig", p,api.zpos);
      unlink( command);      
    }
    
  } else
    puts( "DONNEZ UN NOM DE FICHIER");

  // widget = lookup_widget( GTK_WIDGET(button), "export");
  // gtk_widget_destroy( widget);
}

void
on_export_cancel_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *widget = lookup_widget( GTK_WIDGET(button), "export");
  gtk_widget_destroy( widget);
}

void
on_export_unit_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data)
{

}

void
on_export_codec_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data)
{

}

void 
on_export_type_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
  switch( gtk_combo_box_get_active (combobox)) {
  case 0: /* eps */
  case 1: /* ps */
  case 2: /* tiff */
  case 4: /* mpeg */
  case 5: /* inr */
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_separ1"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_separ2"));

    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_jpeg_quality"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_jpeg_label"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_vbitrate"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_vbitrate_label"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_codec"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_codec_label"));
    break;
  case 3: /* jpeg */
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_separ1"));
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_separ2"));

    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_jpeg_quality"));
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_jpeg_label"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_vbitrate"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_vbitrate_label"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_codec"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_codec_label"));
    break;
  case 6: /* avi */
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_separ1"));
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_separ2"));

    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_jpeg_quality"));
    gtk_widget_hide( lookup_widget( GTK_WIDGET( combobox), "export_jpeg_label"));
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_vbitrate"));
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_vbitrate_label"));
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_codec"));
    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_codec_label"));
    break;
  }
}

