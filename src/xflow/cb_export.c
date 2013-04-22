
#include <gtk/gtk.h>
#include <math.h>
#include <string.h>

#include "data.h"
#include "utils.h"

void 
on_export_type_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data);



void
on_export_destroy                      (GtkObject       *object,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*)user_data;
  g_object_unref( api->export);
  api->export = FALSE;
}

void
on_xflow_main_menu_export_activate    (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API *) user_data;
  GtkWidget *widget, *txt;
  GtkWidget *obj;
  XFLOW_DATA *pd;
  char name[256], *p;
  GError *error;

  if( api->export) {
    gtk_window_present( GTK_WINDOW(lookup_widget(api->export,"export")));
    return;
  }

  api->export = gtk_builder_new();
  
  if( gtk_builder_add_from_file (api->export, "export.glade", NULL) == 0 &&
      gtk_builder_add_from_file (api->export,
				 PACKAGE_DATA_DIR "/xflow/export.glade", 
				 &error) == 0 ) {
    printf("gtk_builder:fatal error:%d:%s\n", error->code, error->message);
    return;
  }
    
  gtk_builder_connect_signals (api->export, api);
  widget = lookup_widget( api->export, "export");
  
  /* Impossible de définir correctement un GtkComboBox avec glade-3
   * et GtkComboBoxText n'existe pas !, on le fait à la main, ca va
   * plus vite */
  {
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Pdf");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Encapsuled postscript");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Postscript");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Tiff");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Jpeg");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Mpeg sequence");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Inrimage sequence");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Avi sequence");
    gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Gif89 animation");
    gtk_combo_box_set_active( GTK_COMBO_BOX(combo), 0);
    gtk_widget_show( combo);
    //gtk_table_attach_defaults( GTK_TABLE(lookup_widget(api->export, "table1")),
    //			       combo, 1, 2, 1, 2);
    gtk_container_add( GTK_CONTAINER(lookup_widget(api->export,"export_type_vbox")), combo);
    g_signal_connect ((gpointer) combo, "changed",
		      G_CALLBACK (on_export_type_changed),
		      api);



    combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), "centimeter");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), "inch");
    gtk_combo_box_set_active( GTK_COMBO_BOX(combo), 0);
    gtk_widget_show( combo);
    //    gtk_table_attach_defaults( GTK_TABLE(lookup_widget(api->export, "table1")),
    //			       combo, 1, 2, 3, 4);
    gtk_container_add( GTK_CONTAINER(lookup_widget(api->export,"export_unit_vbox")), combo);

    combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), "Mjpeg");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), "Mpeg 1");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), "Mpeg 2");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), "Divx 3");
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), "Divx 4/5");
    gtk_combo_box_set_active( GTK_COMBO_BOX(combo), 0);
    gtk_widget_show( combo);
    //    gtk_table_attach_defaults( GTK_TABLE(lookup_widget(api->export, "table1")),
    //			       combo, 1, 2, 3, 4);
    gtk_container_add( GTK_CONTAINER(lookup_widget(api->export,"export_codec_vbox")), combo);
  } 

  /*  gtk_widget_hide( lookup_widget( api->export, "export_separ1"));
      gtk_widget_hide( lookup_widget( api->export, "export_separ2")); */

  gtk_widget_hide( lookup_widget( api->export, "export_jpeg_quality"));
  gtk_widget_hide( lookup_widget( api->export, "export_jpeg_label"));
  gtk_widget_hide( lookup_widget( api->export, "export_vbitrate"));
  gtk_widget_hide( lookup_widget( api->export, "export_vbitrate_label"));
  gtk_widget_hide( lookup_widget( api->export, "export_codec_vbox"));
  gtk_widget_hide( lookup_widget( api->export, "export_codec_label"));

  txt = lookup_widget( api->export, "export_file");
  
  for( pd = api->data; pd; pd  = pd->next) {      
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
  XFLOW_API *api = (XFLOW_API*)user_data;
  XFLOW_DATA *pd;
  GtkWidget *widget;

  char command[512];
  const gchar *p;
  Fort_int *lfmt;
  char *type[] = { "pdf", "eps", "ps", "tiff", "jpeg"};
  int size;
  char *unit[] = { "cm", "in"};
  char *codec[] = { "mpjeg", "mpegvideo1", "mpegvideo1", "msmpeg4", "mpeg4"};
  int bgimg = 0;

  widget = lookup_widget( api->export, "export_file");
  p = gtk_entry_get_text( GTK_ENTRY(widget));
  if( *p) {
    int export_type;
    
    /* options globales */
    widget = lookup_widget( api->export, "export_type");
    export_type = gtk_combo_box_get_active( GTK_COMBO_BOX(widget));
    //    printf( "export_type=%d\n", export_type);
    //    printf("%s\n", gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget)));

    switch( export_type) {
    case 5:
      sprintf( command, "vel2mpg -o %s.mpg", p);
      break;
    case 6:
      sprintf( command, "vel2mpg -o %s.inr", p);      
      break;
    case 7:
      sprintf( command, "vel2mpg -o %s.avi -codec %s -vbr %d", p, 
	       codec[gtk_combo_box_get_active( GTK_COMBO_BOX( lookup_widget( api->export, "export_codec")))],
	       gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( api->export, "export_vbitrate")))
	       );      
      break;
    case 8:
      sprintf( command, "vel2mpg -o %s.gif", p);
      break;
    default:
      sprintf( command, "vel2fig -o %s -iz %d -type %s", p, api->zpos, type[export_type]);
    }

    /* ces options peuvent passer en locale */
    sprintf( command, "%s -scale %f -sample %d -tl %f -th %f ", command,
	     api->scale, api->sample, api->thresh, api->thresh_high);
    
    /* autres options globales */
    
    widget = lookup_widget( api->export, "export_jpeg_quality");
    p = gtk_entry_get_text( GTK_ENTRY(widget));
    size = atoi( p);
    if( size) sprintf( command, "%s -q %d ", command, size);
    
    widget = lookup_widget( api->export, "export_size");
    p = gtk_entry_get_text( GTK_ENTRY(widget));
    size = atoi( p);
    if( size) {
      widget = lookup_widget( api->export, "export_unit");
      sprintf( command, "%s -size %d%s ", command, size, 
	       unit[gtk_combo_box_get_active( GTK_COMBO_BOX(widget))]);
    }
    
    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(lookup_widget(api->export,"export_nf"))))
      strcat( command, "-nf ");
    
    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(lookup_widget(api->export,"export_nvo"))))
      strcat( command, "-nvo ");

    for( pd = api->data; pd; pd  = pd->next) {   
      switch( pd->type) {
      case DATA_IMAGE:
	if( bgimg == 0 ) {
	  lfmt = pd->data.image.file->lfmt;
	  sprintf( command, "%s %s ", command, pd->data.image.file->nom);
	  bgimg = 1;
	}
	break;
      case DATA_XFLOW:
	if( pd->data.xflow.hide) continue;
	
	sprintf( command, "%s %s -acolor %s -asize %s -awidth %d -astyle %d %s %s ", command,
		 pd->data.xflow.file->iuv->nom, 
		 color_name( pd->data.xflow.arrowcolor),
		 size_name( pd->data.xflow.arrowsize),
		 pd->data.xflow.arrowwidth, pd->data.xflow.arrowstyle,
		 pd->data.xflow.smooth?"-smooth":"",
		 pd->data.xflow.norma?"-norma":""
		 
		 );
		

	break;
      }
    }
    
    puts( command);
    system ( command);
    
    /* Post traitement */
    
    widget = lookup_widget( api->export, "export_inter");
    if( !gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget))) {
      widget = lookup_widget( api->export, "export_file");
      p = gtk_entry_get_text( GTK_ENTRY(widget));
      sprintf( command, "%s-%d.gif", p, api->zpos);
      unlink( command);
      sprintf( command, "%s-%d.fig", p, api->zpos);
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
  XFLOW_API *api = (XFLOW_API*)user_data;
  GtkWidget *widget = lookup_widget( api->export, "export");
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
  XFLOW_API *api = (XFLOW_API *)user_data;
  switch( gtk_combo_box_get_active (combobox)) {
   
  case 4: /* jpeg */
    //    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_separ1"));
    //    gtk_widget_show( lookup_widget( GTK_WIDGET( combobox), "export_separ2"));

    gtk_widget_show( lookup_widget( api->export, "export_jpeg_quality"));
    gtk_widget_show( lookup_widget( api->export, "export_jpeg_label"));
    gtk_widget_hide( lookup_widget( api->export, "export_vbitrate"));
    gtk_widget_hide( lookup_widget( api->export, "export_vbitrate_label"));
    gtk_widget_hide( lookup_widget( api->export, "export_codec_vbox"));
    gtk_widget_hide( lookup_widget( api->export, "export_codec_label"));
    break;

  case 7: /* avi */
    //    gtk_widget_show( lookup_widget( GTK_WIDGET( api->export), "export_separ1"));
    //    gtk_widget_show( lookup_widget( GTK_WIDGET( api->export), "export_separ2"));

    gtk_widget_hide( lookup_widget( api->export, "export_jpeg_quality"));
    gtk_widget_hide( lookup_widget( api->export, "export_jpeg_label"));
    gtk_widget_show( lookup_widget( api->export, "export_vbitrate"));
    gtk_widget_show( lookup_widget( api->export, "export_vbitrate_label"));
    gtk_widget_show( lookup_widget( api->export, "export_codec_vbox"));
    gtk_widget_show( lookup_widget( api->export, "export_codec_label"));
    break;

  default:
    //    gtk_widget_hide( lookup_widget( GTK_WIDGET( api->export), "export_separ1"));
    //    gtk_widget_hide( lookup_widget( GTK_WIDGET( api->export), "export_separ2"));

    gtk_widget_hide( lookup_widget( api->export, "export_jpeg_quality"));
    gtk_widget_hide( lookup_widget( api->export, "export_jpeg_label"));
    gtk_widget_hide( lookup_widget( api->export, "export_vbitrate"));
    gtk_widget_hide( lookup_widget( api->export, "export_vbitrate_label"));
    gtk_widget_hide( lookup_widget( api->export, "export_codec_vbox"));
    gtk_widget_hide( lookup_widget( api->export, "export_codec_label"));
    break;
  }

}

