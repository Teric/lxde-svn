/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "main-dlg-ui.h"
#include "glade-support.h"

int
main (int argc, char *argv[])
{
  GtkWidget *dlg;

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  dlg = create_dlg ();
  main_dlg_init( dlg );
  gtk_widget_show (dlg);

  gtk_main ();
  return 0;
}

