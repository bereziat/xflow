
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
  api->export = NULL;
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
  GError *error = NULL;

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
#ifdef GTK_2_24
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
#else
    GtkWidget *combo = gtk_combo_box_entry_new_text ();
    
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Pdf");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Encapsuled postscript");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Postscript");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Tiff");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Jpeg");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Mpeg sequence");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Inrimage sequence");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Avi sequence");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Gif89 animation");
    gtk_combo_box_set_active( GTK_COMBO_BOX(combo), 0);
    gtk_widget_show(combo);
    gtk_container_add (GTK_CONTAINER (lookup_widget(api->export,"export_type_vbox")), 
		       combo);

    combo = gtk_combo_box_entry_new_text ();
    
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "centimeter");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "inch");
    gtk_combo_box_set_active( GTK_COMBO_BOX(combo), 0);
    gtk_widget_show(combo);
    gtk_container_add (GTK_CONTAINER (lookup_widget(api->export,"export_unit_vbox")), 
		       combo);


    combo = gtk_combo_box_entry_new_text ();
    
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Mjpeg");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Mpeg 1");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Mpeg 2");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Divx 3");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Divx 4/5");
    gtk_combo_box_set_active( GTK_COMBO_BOX(combo), 0);
    gtk_widget_show(combo);
    gtk_container_add (GTK_CONTAINER (lookup_widget(api->export,"export_codec_vbox")), 
		       combo);    
#endif
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

GtkWidget *firstchild( GtkWidget *wid) {
  GList *l = gtk_container_get_children( GTK_CONTAINER(wid));
  return l->data;
}

void
on_export_apply_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  XFLOW_API *api = (XFLOW_API*)user_data;
  XFLOW_DATA *pd;
  GtkWidget *widget;
#define MAXLEN 1024
  char *command = NEW(char,MAXLEN);
  const gchar *p;
  //  Fort_int *lfmt;
  char *type[] = { "pdf", "eps", "ps", "tiff", "jpeg"};
  int size;
  char *unit[] = { "cm", "in"};
  char *codec[] = { "mpjeg", "mpegvideo1", "mpegvideo1", "msmpeg4", "mpeg4"};
  
  widget = lookup_widget( api->export, "export_file");
  p = gtk_entry_get_text( GTK_ENTRY(widget));
  if( *p) {
    int export_type;
    
    /* options globales */
    widget = firstchild(lookup_widget( api->export, "export_type_vbox"));
    export_type = gtk_combo_box_get_active( GTK_COMBO_BOX(widget));
    //    printf( "export_type=%d\n", export_type);
    //    printf("%s\n", gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget)));

    switch( export_type) {
    case 5:
      snprintf( command, MAXLEN, "vel2mpg -o %s.mpg", p);
      break;
    case 6:
      snprintf( command, MAXLEN, "vel2mpg -o %s.inr", p);      
      break;
    case 7:
      snprintf( command, MAXLEN, "vel2mpg -o %s.avi -codec %s -vbr %d", p, 
		codec[gtk_combo_box_get_active( GTK_COMBO_BOX( lookup_widget( api->export, "export_codec")))],
		gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( lookup_widget( api->export, "export_vbitrate")))
	       );      
      break;
    case 8:
      snprintf( command, MAXLEN, "vel2mpg -o %s.gif", p);
      break;
    default:
      snprintf( command, MAXLEN, "vel2fig -o %s -iz %d -type %s", p, api->zpos, type[export_type]);
    }

    /* ces options peuvent passer en locale */
    snprintf( command, MAXLEN, "%s -scale %f -sample %d -tl %f -th %f ", command,
	     api->scale, api->sample, api->thresh, api->thresh_high);
    
    /* autres options globales */
    
    widget = lookup_widget( api->export, "export_jpeg_quality");
    p = gtk_entry_get_text( GTK_ENTRY(widget));
    size = atoi( p);
    if( size) snprintf( command, MAXLEN, "%s -q %d ", command, size);
    
    widget = lookup_widget( api->export, "export_size");
    p = gtk_entry_get_text( GTK_ENTRY(widget));
    size = atoi( p);
    if( size) {
      widget = firstchild(lookup_widget( api->export, "export_unit_vbox"));
      snprintf( command, MAXLEN, "%s -size %d%s ", command, size, 
	       unit[gtk_combo_box_get_active( GTK_COMBO_BOX(widget))]);
    }
    
    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(lookup_widget(api->export,"export_nf"))))
      snprintf( command, MAXLEN, "%s -nf ", command);
    
    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(lookup_widget(api->export,"export_nvo"))))
      snprintf( command, MAXLEN, "-nvo ", command);
    
    widget = lookup_widget( api->mainwindow, "xflow_main_notebook");
    
    switch( gtk_notebook_get_current_page( GTK_NOTEBOOK(widget))) {
    case 0:
      if( api->background)
	snprintf( command, MAXLEN, "%s %s ", command, api->background->data.image.file->nom);
      for( pd=api->data; pd; pd = pd->next)
	if( pd->type == DATA_XFLOW &&
	    pd->data.xflow.hide == FALSE
	    ) 
	  snprintf( command, MAXLEN, "%s %s -acolor %s -asize %s -awidth %d -astyle %d %s %s ", command,
		    pd->data.xflow.file->iuv->nom, 
		  color_name( pd->data.xflow.arrowcolor),
		    size_name( pd->data.xflow.arrowsize),
		    pd->data.xflow.arrowwidth, pd->data.xflow.arrowstyle,
		    pd->data.xflow.smooth?"-smooth":"",
		    pd->data.xflow.norma?"-norma":""
		    );
      break;
    default:
      pd = api->active;
      snprintf( command, MAXLEN, "%s %s -acolor %s -asize %s -awidth %d -astyle %d %s %s ", command,
		pd->data.xflow.file->iuv->nom, 
		color_name( pd->data.xflow.arrowcolor),
		size_name( pd->data.xflow.arrowsize),
		pd->data.xflow.arrowwidth, pd->data.xflow.arrowstyle,
		pd->data.xflow.smooth?"-smooth":"",
		pd->data.xflow.norma?"-norma":""
		);
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
  DELETE(command);
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

