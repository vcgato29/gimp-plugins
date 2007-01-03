/*
 * This is a plug-in for the GIMP.
 *
 * Generates clickable image maps.
 *
 * Copyright (C) 1998-2002 Maurits Rijk  lpeek.mrijk@consunet.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "imap_default_dialog.h"


static void
dialog_response (GtkWidget       *widget,
                 gint             response_id,
                 DefaultDialog_t *dialog)
{
  switch (response_id)
    {
    case GTK_RESPONSE_APPLY:
      if (dialog->apply_cb)
        dialog->apply_cb (dialog->apply_cb_data);
      else if (dialog->ok_cb)
        dialog->ok_cb (dialog->ok_cb_data);
      break;

    case GTK_RESPONSE_OK:
      gtk_widget_hide (dialog->dialog);
      if (dialog->ok_cb)
        dialog->ok_cb (dialog->ok_cb_data);
      break;

    default:
      gtk_widget_hide (dialog->dialog);
      if (dialog->cancel_cb)
        dialog->cancel_cb (dialog->cancel_cb_data);
      break;
    }
}

void
default_dialog_set_ok_cb(DefaultDialog_t *dialog, void (*ok_cb)(gpointer),
			 gpointer ok_cb_data)
{
   dialog->ok_cb = ok_cb;
   dialog->ok_cb_data = ok_cb_data;
}

void
default_dialog_set_apply_cb(DefaultDialog_t *dialog,
			    void (*apply_cb)(gpointer),
			    gpointer apply_cb_data)
{
   dialog->apply_cb = apply_cb;
   dialog->apply_cb_data = apply_cb_data;
}

void
default_dialog_set_cancel_cb(DefaultDialog_t *dialog,
			     void (*cancel_cb)(gpointer),
			     gpointer cancel_cb_data)
{
   dialog->cancel_cb = cancel_cb;
   dialog->cancel_cb_data = cancel_cb_data;
}

DefaultDialog_t *
make_default_dialog (const gchar *title)
{
   DefaultDialog_t *data = g_new0 (DefaultDialog_t, 1);

   data->ok_cb = NULL;
   data->apply_cb = NULL;
   data->cancel_cb = NULL;

   data->dialog = gimp_dialog_new (title, "imagemap",
                                   get_dialog(), GTK_DIALOG_DESTROY_WITH_PARENT,
                                   /* gimp_standard_help_func,
                                      "plug-in-imagemap", */
                                   gimp_standard_help_func, NULL,
                                   NULL);

   data->apply = gtk_dialog_add_button (GTK_DIALOG (data->dialog),
                                        GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);

   data->cancel = gtk_dialog_add_button (GTK_DIALOG (data->dialog),
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

   data->ok = gtk_dialog_add_button (GTK_DIALOG (data->dialog),
                                     GTK_STOCK_OK, GTK_RESPONSE_OK);

   gtk_dialog_set_alternative_button_order (GTK_DIALOG (data->dialog),
                                            GTK_RESPONSE_OK,
                                            GTK_RESPONSE_APPLY,
                                            GTK_RESPONSE_CANCEL,
                                            -1);

   g_signal_connect (data->dialog, "response",
                     G_CALLBACK (dialog_response),
                     data);
   g_signal_connect (data->dialog, "destroy",
		     G_CALLBACK (gtk_widget_destroyed),
                     &data->dialog);

   data->vbox = gtk_vbox_new (FALSE, 12);
   gtk_container_set_border_width (GTK_CONTAINER (data->vbox), 12);
   gtk_box_pack_start (GTK_BOX (GTK_DIALOG (data->dialog)->vbox), data->vbox,
                       TRUE, TRUE, 0);
   gtk_widget_show (data->vbox);

   return data;
}

void
default_dialog_show(DefaultDialog_t *dialog)
{
   gtk_widget_show(dialog->dialog);
}

void
default_dialog_hide_cancel_button(DefaultDialog_t *dialog)
{
  gtk_widget_hide(dialog->cancel);
}

void
default_dialog_hide_apply_button(DefaultDialog_t *dialog)
{
  gtk_widget_hide(dialog->apply);
}

void
default_dialog_hide_help_button(DefaultDialog_t *dialog)
{
  /* gtk_widget_hide(dialog->help); */
}

void
default_dialog_set_title(DefaultDialog_t *dialog, const gchar *title)
{
   gtk_window_set_title(GTK_WINDOW(dialog->dialog), title);
}

void
default_dialog_set_label(DefaultDialog_t *dialog, gchar *text)
{
   GtkWidget *label = gtk_label_new(text);

   gtk_box_pack_start (GTK_BOX (dialog->vbox), label, TRUE, TRUE, 0);
   gtk_widget_show(label);
}

GtkWidget*
default_dialog_add_table(DefaultDialog_t *dialog, gint rows, gint cols)
{
   GtkWidget *table = gtk_table_new(rows, cols, FALSE);

   gtk_table_set_row_spacings(GTK_TABLE(table), 6);
   gtk_table_set_col_spacings(GTK_TABLE(table), 6);

   gtk_container_add (GTK_CONTAINER(dialog->vbox), table);
   gtk_widget_show(table);

   return table;
}
