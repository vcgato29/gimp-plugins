#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "gimpressionist.h"

#include "libgimp/stdplugins-intl.h"

static GtkWidget *orientradio[NUMORIENTRADIO];
static GtkObject *orientnumadjust = NULL;
static GtkObject *orientfirstadjust = NULL;
static GtkObject *orientlastadjust = NULL;


static void orientation_store(GtkWidget *wg, void *d)
{
    pcvals.orienttype = GPOINTER_TO_INT (d);
}

void orientation_restore(void)
{
  gtk_toggle_button_set_active (
      GTK_TOGGLE_BUTTON(orientradio[pcvals.orienttype]), 
      TRUE
      );
  gtk_adjustment_set_value (
      GTK_ADJUSTMENT(orientnumadjust), 
      pcvals.orientnum
      );
  gtk_adjustment_set_value(
      GTK_ADJUSTMENT(orientfirstadjust), 
      pcvals.orientfirst
      );
  gtk_adjustment_set_value(
      GTK_ADJUSTMENT(orientlastadjust), 
      pcvals.orientlast
      );
}

static void create_orientmap_dialog_helper(void)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (orientradio[7]), TRUE);
    create_orientmap_dialog();
    return;
}


static void create_orientradio_button (GtkWidget *box, int orienttype, 
                                       gchar *label, gchar *help_string,
                                       GSList **radio_group
                                       )
{
  create_radio_button (box, orienttype, orientation_store, label, 
                       help_string, radio_group, orientradio);
  return;
}

void
create_orientationpage (GtkNotebook *notebook)
{
  GtkWidget *box2, *box3, *box4, *thispage;
  GtkWidget *label, *tmpw, *table;
  GSList *radio_group = NULL;

  label = gtk_label_new_with_mnemonic (_("Or_ientation"));

  thispage = gtk_vbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (thispage), 12);
  gtk_widget_show (thispage);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (thispage), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  orientnumadjust =
    gimp_scale_entry_new (GTK_TABLE (table), 0, 0,
			  _("Directions:"),
			  150, -1, pcvals.orientnum,
			  1.0, 30.0, 1.0, 1.0, 0,
			  TRUE, 0, 0,
			  _("The number of directions (i.e. brushes) to use"),
			  NULL);
  g_signal_connect (orientnumadjust, "value_changed",
                    G_CALLBACK (gimp_int_adjustment_update),
                    &pcvals.orientnum);

  orientfirstadjust =
    gimp_scale_entry_new (GTK_TABLE (table), 0, 1,
			  _("Start angle:"),
			  150, -1, pcvals.orientfirst,
			  0.0, 360.0, 1.0, 10.0, 0,
			  TRUE, 0, 0,
			  _("The starting angle of the first brush to create"),
			  NULL);
  g_signal_connect (orientfirstadjust, "value_changed",
                    G_CALLBACK (gimp_double_adjustment_update),
                    &pcvals.orientfirst);

  orientlastadjust =
    gimp_scale_entry_new (GTK_TABLE (table), 0, 2,
			  _("Angle span:"),
			  150, -1, pcvals.orientlast,
			  0.0, 360.0, 1.0, 10.0, 0,
			  TRUE, 0, 0,
			  _("The angle span of the first brush to create"),
			  NULL);
  g_signal_connect (orientlastadjust, "value_changed",
                    G_CALLBACK (gimp_double_adjustment_update),
                    &pcvals.orientlast);

  box2 = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (thispage), box2, FALSE, FALSE, 0);
  gtk_widget_show (box2);

  box3 = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (box2), box3, FALSE, FALSE, 0);
  gtk_widget_show (box3);

  tmpw = gtk_label_new (_("Orientation:"));
  gtk_box_pack_start (GTK_BOX (box3), tmpw, FALSE, FALSE, 0);
  gtk_widget_show (tmpw);

  box3 = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (box2), box3, FALSE, FALSE, 0);
  gtk_widget_show (box3);

  create_orientradio_button (box3, ORIENTATION_VALUE, _("Value"),
          _("Let the value (brightness) of the region determine the direction of the stroke"),
          &radio_group
          );
  
  create_orientradio_button(box3, ORIENTATION_RADIUS, _("Radius"),
          _("The distance from the center of the image determines the direction of the stroke"),
          &radio_group
          );
  
  create_orientradio_button(box3, ORIENTATION_RANDOM, _("Random"),
          _("Selects a random direction of each stroke"),
          &radio_group
          );
    
  create_orientradio_button(box3, ORIENTATION_RADIAL, _("Radial"),
          _("Let the direction from the center determine the direction of the stroke"),
          &radio_group
          );
  
  box3 = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (box2), box3, FALSE, FALSE, 0);
  gtk_widget_show (box3);

  create_orientradio_button(box3, ORIENTATION_FLOWING, _("Flowing"),
          _("The strokes follow a \"flowing\" pattern"),
          &radio_group
          );

  create_orientradio_button(box3, ORIENTATION_HUE, _("Hue"),
          _("The hue of the region determines the direction of the stroke"),
          &radio_group
          );

  create_orientradio_button(box3, ORIENTATION_ADAPTIVE, _("Adaptive"),
          _("The direction that matches the original image the closest is selected"),
          &radio_group
          );
  
  box4 = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (box3), box4, FALSE, FALSE, 0);
  gtk_widget_show (box4);

  create_orientradio_button(box4, ORIENTATION_MANUAL, _("Manual"),
          _("Manually specify the stroke orientation"),
          &radio_group
          );

  orientation_restore();

  tmpw = gtk_button_new_from_stock (GIMP_STOCK_EDIT);
  gtk_box_pack_start (GTK_BOX (box4), tmpw, FALSE, FALSE, 0);
  gtk_widget_show (tmpw);
  g_signal_connect (tmpw, "clicked",
		    G_CALLBACK (create_orientmap_dialog_helper), NULL);
  gimp_help_set_help_data
    (tmpw, _("Opens up the Orientation Map Editor"), NULL);

  gtk_notebook_append_page_menu (notebook, thispage, label, NULL);
}
