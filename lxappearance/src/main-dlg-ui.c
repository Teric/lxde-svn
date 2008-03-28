/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "main-dlg.h"
#include "main-dlg-ui.h"
#include "glade-support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_dlg (void)
{
  GtkWidget *dlg;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *notebook1;
  GtkWidget *vbox4;
  GtkWidget *label8;
  GtkWidget *scrolledwindow1;
  GtkWidget *gtk_theme_view;
  GtkWidget *hbox4;
  GtkWidget *label17;
  GtkWidget *font;
  GtkWidget *label2;
  GtkWidget *vbox3;
  GtkWidget *label7;
  GtkWidget *scrolledwindow2;
  GtkWidget *icon_theme_view;
  GtkWidget *hbox5;
  GtkWidget *install_theme;
  GtkWidget *alignment1;
  GtkWidget *hbox6;
  GtkWidget *image1;
  GtkWidget *label18;
  GtkWidget *remove_theme;
  GtkWidget *label3;
  GtkWidget *vbox5;
  GtkWidget *vbox6;
  GtkWidget *label19;
  GtkWidget *tb_style;
  GtkWidget *label5;
  GtkWidget *vbox1;
  GtkWidget *label15;
  GtkWidget *frame1;
  GtkWidget *demo_box;
  GtkWidget *dialog_action_area1;
  GtkWidget *apply;
  GtkWidget *closebutton1;

  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dlg), _("Appearance Settings"));
  gtk_window_set_type_hint (GTK_WINDOW (dlg), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dlg)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (hbox1), notebook1, FALSE, TRUE, 0);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox4);

  label8 = gtk_label_new (_("Available Window Themes"));
  gtk_widget_show (label8);
  gtk_box_pack_start (GTK_BOX (vbox4), label8, FALSE, FALSE, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (vbox4), scrolledwindow1, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gtk_theme_view = gtk_tree_view_new ();
  gtk_widget_show (gtk_theme_view);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), gtk_theme_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gtk_theme_view), FALSE);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox4, FALSE, FALSE, 4);

  label17 = gtk_label_new_with_mnemonic (_("_Font:"));
  gtk_widget_show (label17);
  gtk_box_pack_start (GTK_BOX (hbox4), label17, FALSE, TRUE, 4);

  font = gtk_font_button_new ();
  gtk_widget_show (font);
  gtk_box_pack_start (GTK_BOX (hbox4), font, TRUE, TRUE, 0);
  gtk_widget_set_size_request (font, 64, -1);
  gtk_container_set_border_width (GTK_CONTAINER (font), 2);
  gtk_font_button_set_use_font (GTK_FONT_BUTTON (font), TRUE);
  gtk_font_button_set_use_size (GTK_FONT_BUTTON (font), TRUE);

  label2 = gtk_label_new (_("Window"));
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label2);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox3);

  label7 = gtk_label_new (_("Available Icon Themes"));
  gtk_widget_show (label7);
  gtk_box_pack_start (GTK_BOX (vbox3), label7, FALSE, FALSE, 0);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow2);
  gtk_box_pack_start (GTK_BOX (vbox3), scrolledwindow2, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_IN);

  icon_theme_view = gtk_tree_view_new ();
  gtk_widget_show (icon_theme_view);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), icon_theme_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (icon_theme_view), FALSE);

  hbox5 = gtk_hbox_new (FALSE, 4);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox5, FALSE, TRUE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox5), 2);

  install_theme = gtk_button_new ();
  gtk_widget_show (install_theme);
  gtk_box_pack_start (GTK_BOX (hbox5), install_theme, TRUE, TRUE, 0);

  alignment1 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (install_theme), alignment1);

  hbox6 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox6);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox6);

  image1 = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox6), image1, FALSE, FALSE, 0);

  label18 = gtk_label_new_with_mnemonic (_("_Install"));
  gtk_widget_show (label18);
  gtk_box_pack_start (GTK_BOX (hbox6), label18, FALSE, FALSE, 0);

  remove_theme = gtk_button_new_from_stock ("gtk-remove");
  gtk_box_pack_start (GTK_BOX (hbox5), remove_theme, TRUE, TRUE, 0);

  label3 = gtk_label_new (_("Icon"));
  gtk_widget_show (label3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label3);

  vbox5 = gtk_vbox_new (FALSE, 4);
  gtk_widget_show (vbox5);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox5);

  vbox6 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox6);
  gtk_box_pack_start (GTK_BOX (vbox5), vbox6, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox6), 4);

  label19 = gtk_label_new (_("Toolbar Style: "));
  gtk_widget_show (label19);
  gtk_box_pack_start (GTK_BOX (vbox6), label19, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label19), 0, 0.5);

  tb_style = gtk_combo_box_new_text ();
  gtk_widget_show (tb_style);
  gtk_box_pack_start (GTK_BOX (vbox6), tb_style, TRUE, TRUE, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (tb_style), _("Icons only"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (tb_style), _("Text only"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (tb_style), _("Text below icons"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (tb_style), _("Text beside icons"));

  label5 = gtk_label_new (_("Other"));
  gtk_widget_show (label5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label5);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);

  label15 = gtk_label_new (_("<b>Preview</b>"));
  gtk_widget_show (label15);
  gtk_box_pack_start (GTK_BOX (vbox1), label15, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (label15), TRUE);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 6);

  demo_box = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (demo_box);
  gtk_container_add (GTK_CONTAINER (frame1), demo_box);
  gtk_alignment_set_padding (GTK_ALIGNMENT (demo_box), 0, 0, 4, 4);

  dialog_action_area1 = GTK_DIALOG (dlg)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  apply = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (apply);
  gtk_dialog_add_action_widget (GTK_DIALOG (dlg), apply, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (apply, GTK_CAN_DEFAULT);

  closebutton1 = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (closebutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dlg), closebutton1, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (closebutton1, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) dlg, "delete_event",
                    G_CALLBACK (gtk_main_quit),
                    NULL);
  g_signal_connect ((gpointer) font, "font_set",
                    G_CALLBACK (on_font_changed),
                    NULL);
  g_signal_connect ((gpointer) install_theme, "clicked",
                    G_CALLBACK (on_install_theme_clicked),
                    NULL);
  g_signal_connect ((gpointer) remove_theme, "clicked",
                    G_CALLBACK (on_remove_theme_clicked),
                    NULL);
  g_signal_connect ((gpointer) tb_style, "changed",
                    G_CALLBACK (on_tb_style_changed),
                    NULL);
  g_signal_connect ((gpointer) apply, "clicked",
                    G_CALLBACK (on_apply_clicked),
                    NULL);
  g_signal_connect ((gpointer) closebutton1, "clicked",
                    G_CALLBACK (gtk_main_quit),
                    NULL);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label17), font);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dlg, dlg, "dlg");
  GLADE_HOOKUP_OBJECT_NO_REF (dlg, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dlg, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dlg, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (dlg, vbox4, "vbox4");
  GLADE_HOOKUP_OBJECT (dlg, label8, "label8");
  GLADE_HOOKUP_OBJECT (dlg, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (dlg, gtk_theme_view, "gtk_theme_view");
  GLADE_HOOKUP_OBJECT (dlg, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (dlg, label17, "label17");
  GLADE_HOOKUP_OBJECT (dlg, font, "font");
  GLADE_HOOKUP_OBJECT (dlg, label2, "label2");
  GLADE_HOOKUP_OBJECT (dlg, vbox3, "vbox3");
  GLADE_HOOKUP_OBJECT (dlg, label7, "label7");
  GLADE_HOOKUP_OBJECT (dlg, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT (dlg, icon_theme_view, "icon_theme_view");
  GLADE_HOOKUP_OBJECT (dlg, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (dlg, install_theme, "install_theme");
  GLADE_HOOKUP_OBJECT (dlg, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (dlg, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (dlg, image1, "image1");
  GLADE_HOOKUP_OBJECT (dlg, label18, "label18");
  GLADE_HOOKUP_OBJECT (dlg, remove_theme, "remove_theme");
  GLADE_HOOKUP_OBJECT (dlg, label3, "label3");
  GLADE_HOOKUP_OBJECT (dlg, vbox5, "vbox5");
  GLADE_HOOKUP_OBJECT (dlg, vbox6, "vbox6");
  GLADE_HOOKUP_OBJECT (dlg, label19, "label19");
  GLADE_HOOKUP_OBJECT (dlg, tb_style, "tb_style");
  GLADE_HOOKUP_OBJECT (dlg, label5, "label5");
  GLADE_HOOKUP_OBJECT (dlg, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dlg, label15, "label15");
  GLADE_HOOKUP_OBJECT (dlg, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (dlg, demo_box, "demo_box");
  GLADE_HOOKUP_OBJECT_NO_REF (dlg, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dlg, apply, "apply");
  GLADE_HOOKUP_OBJECT (dlg, closebutton1, "closebutton1");

  return dlg;
}

