/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Webpage plug-in.
 * Copyright (C) 2011 Mukund Sivaraman <muks@banu.com>.
 * Portions are copyright of the author of the
 * file-open-location-dialog.c code.
 *
 * TODO:
 * - Add a font scale combo: default, larger, smaller etc.
 * - Save/restore URL and width
 * - Set GIMP as user agent
 */

#include "config.h"

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include <webkit/webkit.h>

#include "libgimp/stdplugins-intl.h"

/* Defines */
#define PLUG_IN_PROC   "plug-in-web-page"
#define PLUG_IN_BINARY "web-page"

typedef struct
{
  char   *url;
  gint32  width;
} WebpageVals;

static WebpageVals webpagevals =
  {
    NULL,
    1024
  };

static GdkPixbuf *webpixbuf;
static GError *weberror;

static void     query           (void);
static void     run             (const gchar      *name,
                                 gint              nparams,
                                 const GimpParam  *param,
                                 gint             *nreturn_vals,
                                 GimpParam       **return_vals);
static gboolean webpage_dialog  (void);
static gint32   webpage_capture (void);


/* Global Variables */
const GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run    /* run_proc   */
};


/* Functions */

MAIN ()

static void
query (void)
{
  static const GimpParamDef args[] =
  {
    { GIMP_PDB_INT32,  "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { GIMP_PDB_STRING, "url",      "URL of the webpage to screenshot"                             },
    { GIMP_PDB_INT32,  "width",    "The width of the screenshot"                                  }
  };

  static const GimpParamDef return_vals[] =
  {
    { GIMP_PDB_IMAGE, "image", "Output image" }
  };

  gimp_install_procedure (PLUG_IN_PROC,
                          N_("Create an image of a webpage"),
                          "The plug-in allows you to take a screenshot "
                          "of a webpage.",
                          "Mukund Sivaraman <muks@banu.com>",
                          "2011",
                          "2011",
                          N_("From _Webpage..."),
                          NULL,
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (args),
                          G_N_ELEMENTS (return_vals),
                          args, return_vals);

  gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/File/Create/Acquire");
}

static void
run (const gchar      *name,
     gint             nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  GimpRunMode        run_mode = param[0].data.d_int32;
  GimpPDBStatusType  status   = GIMP_PDB_EXECUTION_ERROR;
  gint32             image_id = -1;

  static GimpParam   values[2];

  INIT_I18N ();

  /* initialize the return of the status */
  *nreturn_vals = 1;
  *return_vals  = values;
  values[0].type = GIMP_PDB_STATUS;

  /* MUST call this before any RSVG funcs */
  g_type_init ();

  /* how are we running today? */
  switch (run_mode)
    {
    case GIMP_RUN_INTERACTIVE:
      if (webpage_dialog ())
        status = GIMP_PDB_SUCCESS;
      else
        status = GIMP_PDB_CANCEL;
      break;

    case GIMP_RUN_WITH_LAST_VALS:
      /* This is currently not supported. */
      break;

    case GIMP_RUN_NONINTERACTIVE:
      webpagevals.url = param[1].data.d_string;
      webpagevals.width = param[2].data.d_int32;
      status = GIMP_PDB_SUCCESS;
      break;

    default:
      break;
    }

  if (status == GIMP_PDB_SUCCESS)
    {
      image_id = webpage_capture ();

      if (image_id == -1)
        {
          status = GIMP_PDB_EXECUTION_ERROR;

          if (weberror)
            {
              *nreturn_vals = 2;

              values[1].type = GIMP_PDB_STRING;
              values[1].data.d_string = weberror->message;
            }
        }
      else
        {
          if (run_mode == GIMP_RUN_INTERACTIVE)
            gimp_display_new (image_id);

          *nreturn_vals = 2;

          values[1].type         = GIMP_PDB_IMAGE;
          values[1].data.d_image = image_id;
        }
    }

  values[0].data.d_status = status;
}

static gboolean
webpage_dialog (void)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *entry;
  GtkObject *adjustment;
  GtkWidget *spinbutton;
  gint status;
  gboolean ret = FALSE;

  gimp_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = gimp_dialog_new (_("Create from webpage"), PLUG_IN_BINARY,
                            NULL, 0,
                            gimp_standard_help_func, PLUG_IN_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            _("_Create"),     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);
  gimp_window_set_transient (GTK_WINDOW (dialog));

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  image = gtk_image_new_from_stock (GIMP_STOCK_WEB, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  label = gtk_label_new (_("Enter location (URI):"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  gtk_widget_set_size_request (entry, 400, -1);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

  if (webpagevals.url)
    gtk_entry_set_text (GTK_ENTRY (entry),
                        webpagevals.url);
  else
    gtk_entry_set_text (GTK_ENTRY (entry),
                        "http://www.gimp.org/");

  gtk_widget_show (entry);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox),
                      hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Width (pixels):"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  spinbutton = gimp_spin_button_new (&adjustment, webpagevals.width,
                                     1, 8192,
                                     1, 10, 0, 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);

  status = gimp_dialog_run (GIMP_DIALOG (dialog));
  if (status == GTK_RESPONSE_OK)
    {
      g_free (webpagevals.url);
      webpagevals.url = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

      webpagevals.width = (gint) gtk_adjustment_get_value
        (GTK_ADJUSTMENT (adjustment));

      ret = TRUE;
    }

  gtk_widget_destroy (dialog);

  return ret;
}

static void
notify_progress_cb (WebKitWebView  *view,
                    GParamSpec     *pspec,
                    gpointer        user_data)
{
  static gdouble old_progress = 0.0;
  gdouble progress;

  g_object_get (view,
                "progress", &progress,
                NULL);

  if ((progress - old_progress) > 0.01)
    {
      gimp_progress_update (progress);
      old_progress = progress;
    }
}

static gboolean
load_error_cb (WebKitWebView  *view,
               WebKitWebFrame *web_frame,
               gchar          *uri,
               gpointer        web_error,
               gpointer        user_data)
{
  weberror = g_error_copy ((GError *) web_error);

  gtk_main_quit ();

  return TRUE;
}

static void
notify_load_status_cb (WebKitWebView  *view,
                       GParamSpec     *pspec,
                       gpointer        user_data)
{
  WebKitLoadStatus status;

  g_object_get (view,
                "load-status", &status,
                NULL);

  if (status == WEBKIT_LOAD_FINISHED)
    {
      if (!weberror)
        {
          webpixbuf = gtk_offscreen_window_get_pixbuf
            (GTK_OFFSCREEN_WINDOW (user_data));
        }

      gtk_main_quit ();
    }
}

static gint32
webpage_capture (void)
{
  gint32 image = -1;
  gchar *scheme;
  GtkWidget *window;
  GtkWidget *view;

  if (webpixbuf)
    {
      g_object_unref (webpixbuf);
      webpixbuf = NULL;
    }
  if (weberror)
    {
      g_error_free (weberror);
      weberror = NULL;
    }

  if ((!webpagevals.url) ||
      (strlen (webpagevals.url) == 0))
    {
      g_set_error (&weberror, 0, 0, _("No URL was specified"));
      return -1;
    }

  scheme = g_uri_parse_scheme (webpagevals.url);
  if (!scheme)
    {
      char *url;

      /* If we were not given a well-formed URL, make one. */

      url = g_strconcat ("http://", webpagevals.url, NULL);
      g_free (webpagevals.url);
      webpagevals.url = url;

      g_free (scheme);
    }

  if (webpagevals.width < 32)
    {
      g_warning ("Width `%d' is too small. Clamped to 32.",
                 webpagevals.width);
      webpagevals.width = 32;
    }
  else if (webpagevals.width > 8192)
    {
      g_warning ("Width `%d' is too large. Clamped to 8192.",
                 webpagevals.width);
      webpagevals.width = 8192;
    }

  window = gtk_offscreen_window_new ();
  gtk_widget_show (window);

  view = webkit_web_view_new ();
  gtk_widget_show (view);

  gtk_widget_set_size_request (view, webpagevals.width, -1);
  gtk_container_add (GTK_CONTAINER (window), view);

  g_signal_connect (view, "notify::progress",
                    G_CALLBACK (notify_progress_cb),
                    window);
  g_signal_connect (view, "load-error",
                    G_CALLBACK (load_error_cb),
                    window);
  g_signal_connect (view, "notify::load-status",
                    G_CALLBACK (notify_load_status_cb),
                    window);

  gimp_progress_init_printf (_("Downloading webpage `%s'"), webpagevals.url);

  webkit_web_view_open (WEBKIT_WEB_VIEW (view),
                        webpagevals.url);

  gtk_main ();

  gtk_widget_destroy (window);

  if (webpixbuf)
    {
      gint width;
      gint height;
      gint32 layer;

      gimp_progress_init_printf (_("Transferring webpage image for `%s'"),
                                 webpagevals.url);

      width  = gdk_pixbuf_get_width (webpixbuf);
      height = gdk_pixbuf_get_height (webpixbuf);

      image = gimp_image_new (width, height, GIMP_RGB);

      gimp_image_undo_disable (image);
      layer = gimp_layer_new_from_pixbuf (image, _("Webpage"), webpixbuf,
                                          100, GIMP_NORMAL_MODE, 0.0, 1.0);
      gimp_image_insert_layer (image, layer, -1, 0);
      gimp_image_undo_enable (image);

      g_object_unref (webpixbuf);
      webpixbuf = NULL;
    }

  return image;
}