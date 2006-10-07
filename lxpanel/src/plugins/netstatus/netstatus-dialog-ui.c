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

#include "netstatus-dialog.h"
#include "netstatus-dialog-ui.h"
#include "glade-support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_network_status_dialog (void)
{
  GtkWidget *network_status_dialog;
  GtkWidget *dialog_vbox2;
  GtkWidget *notebook1;
  GtkWidget *vbox1;
  GtkWidget *connection_frame;
  GtkWidget *label3;
  GtkWidget *hbox6;
  GtkWidget *label43;
  GtkWidget *connection_hbox;
  GtkWidget *connection_table;
  GtkWidget *label5;
  GtkWidget *status_label;
  GtkWidget *label16;
  GtkWidget *name_combo;
  GtkWidget *combo_entry1;
  GtkWidget *activity_frame;
  GtkWidget *label4;
  GtkWidget *hbox8;
  GtkWidget *label44;
  GtkWidget *table2;
  GtkWidget *received_label;
  GtkWidget *sent_label;
  GtkWidget *label11;
  GtkWidget *label12;
  GtkWidget *signal_strength_frame;
  GtkWidget *label48;
  GtkWidget *hbox14;
  GtkWidget *label49;
  GtkWidget *hbox15;
  GtkWidget *signal_strength_bar;
  GtkWidget *signal_strength_label;
  GtkWidget *label1;
  GtkWidget *vbox2;
  GtkWidget *inet4_frame;
  GtkWidget *label25;
  GtkWidget *hbox11;
  GtkWidget *label45;
  GtkWidget *inet4_table;
  GtkWidget *inet4_addr_title;
  GtkWidget *inet4_dest_title;
  GtkWidget *inet4_bcast_title;
  GtkWidget *inet4_mask_title;
  GtkWidget *inet4_addr_label;
  GtkWidget *inet4_dest_label;
  GtkWidget *inet4_bcast_label;
  GtkWidget *inet4_mask_label;
  GtkWidget *inet6_frame;
  GtkWidget *label26;
  GtkWidget *hbox12;
  GtkWidget *label46;
  GtkWidget *table5;
  GtkWidget *label39;
  GtkWidget *label40;
  GtkWidget *inet6_addr_label;
  GtkWidget *inet6_scope_label;
  GtkWidget *dev_frame;
  GtkWidget *dev_label;
  GtkWidget *hbox13;
  GtkWidget *label47;
  GtkWidget *table6;
  GtkWidget *dev_addr_title;
  GtkWidget *dev_addr_label;
  GtkWidget *dev_type_label;
  GtkWidget *dev_type_title;
  GtkWidget *label2;
  GtkWidget *dialog_action_area2;
  GtkWidget *helpbutton1;
  GtkWidget *configure_button;
  GtkWidget *alignment2;
  GtkWidget *hbox5;
  GtkWidget *image2;
  GtkWidget *label42;
  GtkWidget *close_button;

  network_status_dialog = gtk_dialog_new ();
  gtk_container_set_border_width (GTK_CONTAINER (network_status_dialog), 5);
  gtk_window_set_position (GTK_WINDOW (network_status_dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (network_status_dialog), 300, -1);
  gtk_window_set_resizable (GTK_WINDOW (network_status_dialog), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (network_status_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_dialog_set_has_separator (GTK_DIALOG (network_status_dialog), FALSE);

  dialog_vbox2 = GTK_DIALOG (network_status_dialog)->vbox;
  gtk_widget_show (dialog_vbox2);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), notebook1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (notebook1), 5);

  vbox1 = gtk_vbox_new (FALSE, 18);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 12);

  connection_frame = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (connection_frame);
  gtk_box_pack_start (GTK_BOX (vbox1), connection_frame, FALSE, TRUE, 0);

  label3 = gtk_label_new (_("<b>Connection</b>"));
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (connection_frame), label3, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (label3), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  hbox6 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox6);
  gtk_box_pack_start (GTK_BOX (connection_frame), hbox6, TRUE, TRUE, 0);

  label43 = gtk_label_new (_("    "));
  gtk_widget_show (label43);
  gtk_box_pack_start (GTK_BOX (hbox6), label43, FALSE, FALSE, 0);

  connection_hbox = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (connection_hbox);
  gtk_box_pack_start (GTK_BOX (hbox6), connection_hbox, TRUE, TRUE, 0);

  connection_table = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (connection_table);
  gtk_box_pack_start (GTK_BOX (connection_hbox), connection_table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (connection_table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (connection_table), 12);

  label5 = gtk_label_new (_("Status:"));
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (connection_table), label5, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  status_label = gtk_label_new ("");
  gtk_widget_show (status_label);
  gtk_table_attach (GTK_TABLE (connection_table), status_label, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (status_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (status_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (status_label), 0, 0.5);

  label16 = gtk_label_new_with_mnemonic (_("_Name:"));
  gtk_widget_show (label16);
  gtk_table_attach (GTK_TABLE (connection_table), label16, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label16), 0, 0.5);

  name_combo = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (name_combo)->popwin),
                     "GladeParentKey", name_combo);
  gtk_widget_show (name_combo);
  gtk_table_attach (GTK_TABLE (connection_table), name_combo, 1, 2, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_combo_set_case_sensitive (GTK_COMBO (name_combo), TRUE);

  combo_entry1 = GTK_COMBO (name_combo)->entry;
  gtk_widget_show (combo_entry1);

  activity_frame = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (activity_frame);
  gtk_box_pack_start (GTK_BOX (vbox1), activity_frame, FALSE, TRUE, 0);

  label4 = gtk_label_new (_("<b>Activity</b>"));
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (activity_frame), label4, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (label4), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox8);
  gtk_box_pack_start (GTK_BOX (activity_frame), hbox8, TRUE, TRUE, 0);

  label44 = gtk_label_new (_("    "));
  gtk_widget_show (label44);
  gtk_box_pack_start (GTK_BOX (hbox8), label44, FALSE, FALSE, 0);

  table2 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table2);
  gtk_box_pack_start (GTK_BOX (hbox8), table2, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 12);

  received_label = gtk_label_new ("");
  gtk_widget_show (received_label);
  gtk_table_attach (GTK_TABLE (table2), received_label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (received_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (received_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (received_label), 0, 0.5);

  sent_label = gtk_label_new ("");
  gtk_widget_show (sent_label);
  gtk_table_attach (GTK_TABLE (table2), sent_label, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (sent_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (sent_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (sent_label), 0, 0.5);

  label11 = gtk_label_new (_("Received:"));
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table2), label11, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

  label12 = gtk_label_new (_("Sent:"));
  gtk_widget_show (label12);
  gtk_table_attach (GTK_TABLE (table2), label12, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label12), 0, 0.5);

  signal_strength_frame = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (signal_strength_frame);
  gtk_box_pack_start (GTK_BOX (vbox1), signal_strength_frame, FALSE, TRUE, 0);

  label48 = gtk_label_new (_("<b>Signal Strength</b>"));
  gtk_widget_show (label48);
  gtk_box_pack_start (GTK_BOX (signal_strength_frame), label48, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (label48), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label48), 0, 0.5);

  hbox14 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox14);
  gtk_box_pack_start (GTK_BOX (signal_strength_frame), hbox14, FALSE, TRUE, 0);

  label49 = gtk_label_new (_("    "));
  gtk_widget_show (label49);
  gtk_box_pack_start (GTK_BOX (hbox14), label49, FALSE, FALSE, 0);

  hbox15 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox15);
  gtk_box_pack_start (GTK_BOX (hbox14), hbox15, TRUE, TRUE, 0);

  signal_strength_bar = gtk_progress_bar_new ();
  gtk_widget_show (signal_strength_bar);
  gtk_box_pack_start (GTK_BOX (hbox15), signal_strength_bar, TRUE, TRUE, 0);

  signal_strength_label = gtk_label_new (_("0%"));
  gtk_widget_show (signal_strength_label);
  gtk_box_pack_start (GTK_BOX (hbox15), signal_strength_label, FALSE, TRUE, 0);

  label1 = gtk_label_new (_("General"));
  gtk_widget_show (label1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label1);

  vbox2 = gtk_vbox_new (FALSE, 18);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);

  inet4_frame = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (inet4_frame);
  gtk_box_pack_start (GTK_BOX (vbox2), inet4_frame, FALSE, TRUE, 0);

  label25 = gtk_label_new (_("<b>Internet Protocol (IPv4)</b>"));
  gtk_widget_show (label25);
  gtk_box_pack_start (GTK_BOX (inet4_frame), label25, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (label25), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label25), 0, 0.5);

  hbox11 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox11);
  gtk_box_pack_start (GTK_BOX (inet4_frame), hbox11, TRUE, TRUE, 0);

  label45 = gtk_label_new (_("    "));
  gtk_widget_show (label45);
  gtk_box_pack_start (GTK_BOX (hbox11), label45, FALSE, FALSE, 0);

  inet4_table = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (inet4_table);
  gtk_box_pack_start (GTK_BOX (hbox11), inet4_table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (inet4_table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (inet4_table), 12);

  inet4_addr_title = gtk_label_new (_("Address:"));
  gtk_widget_show (inet4_addr_title);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_addr_title, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (inet4_addr_title), 0, 0.5);

  inet4_dest_title = gtk_label_new (_("Destination:"));
  gtk_widget_show (inet4_dest_title);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_dest_title, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (inet4_dest_title), 0, 0.5);

  inet4_bcast_title = gtk_label_new (_("Broadcast:"));
  gtk_widget_show (inet4_bcast_title);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_bcast_title, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (inet4_bcast_title), 0, 0.5);

  inet4_mask_title = gtk_label_new (_("Subnet Mask:"));
  gtk_widget_show (inet4_mask_title);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_mask_title, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (inet4_mask_title), 0, 0.5);

  inet4_addr_label = gtk_label_new ("");
  gtk_widget_show (inet4_addr_label);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_addr_label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (inet4_addr_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (inet4_addr_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (inet4_addr_label), 0, 0.5);

  inet4_dest_label = gtk_label_new ("");
  gtk_widget_show (inet4_dest_label);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_dest_label, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (inet4_dest_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (inet4_dest_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (inet4_dest_label), 0, 0.5);

  inet4_bcast_label = gtk_label_new ("");
  gtk_widget_show (inet4_bcast_label);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_bcast_label, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (inet4_bcast_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (inet4_bcast_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (inet4_bcast_label), 0, 0.5);

  inet4_mask_label = gtk_label_new ("");
  gtk_widget_show (inet4_mask_label);
  gtk_table_attach (GTK_TABLE (inet4_table), inet4_mask_label, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (inet4_mask_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (inet4_mask_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (inet4_mask_label), 0, 0.5);

  inet6_frame = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox2), inet6_frame, FALSE, TRUE, 0);

  label26 = gtk_label_new (_("<b>Internet Protocol (IPv6)</b>"));
  gtk_widget_show (label26);
  gtk_box_pack_start (GTK_BOX (inet6_frame), label26, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (label26), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

  hbox12 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox12);
  gtk_box_pack_start (GTK_BOX (inet6_frame), hbox12, TRUE, TRUE, 0);

  label46 = gtk_label_new (_("    "));
  gtk_widget_show (label46);
  gtk_box_pack_start (GTK_BOX (hbox12), label46, FALSE, FALSE, 0);

  table5 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table5);
  gtk_box_pack_start (GTK_BOX (hbox12), table5, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table5), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table5), 12);

  label39 = gtk_label_new (_("Address:"));
  gtk_widget_show (label39);
  gtk_table_attach (GTK_TABLE (table5), label39, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label39), 0, 0.5);

  label40 = gtk_label_new (_("Scope:"));
  gtk_widget_show (label40);
  gtk_table_attach (GTK_TABLE (table5), label40, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label40), 0, 0.5);

  inet6_addr_label = gtk_label_new ("");
  gtk_widget_show (inet6_addr_label);
  gtk_table_attach (GTK_TABLE (table5), inet6_addr_label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (inet6_addr_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (inet6_addr_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (inet6_addr_label), 0, 0.5);

  inet6_scope_label = gtk_label_new ("");
  gtk_widget_show (inet6_scope_label);
  gtk_table_attach (GTK_TABLE (table5), inet6_scope_label, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (inet6_scope_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (inet6_scope_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (inet6_scope_label), 0, 0.5);

  dev_frame = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (dev_frame);
  gtk_box_pack_start (GTK_BOX (vbox2), dev_frame, FALSE, TRUE, 0);

  dev_label = gtk_label_new (_("<b>Network Device</b>"));
  gtk_widget_show (dev_label);
  gtk_box_pack_start (GTK_BOX (dev_frame), dev_label, FALSE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (dev_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (dev_label), 0, 0.5);

  hbox13 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox13);
  gtk_box_pack_start (GTK_BOX (dev_frame), hbox13, TRUE, TRUE, 0);

  label47 = gtk_label_new (_("    "));
  gtk_widget_show (label47);
  gtk_box_pack_start (GTK_BOX (hbox13), label47, FALSE, FALSE, 0);

  table6 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table6);
  gtk_box_pack_start (GTK_BOX (hbox13), table6, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table6), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table6), 12);

  dev_addr_title = gtk_label_new (_("Address:"));
  gtk_widget_show (dev_addr_title);
  gtk_table_attach (GTK_TABLE (table6), dev_addr_title, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (dev_addr_title), 0, 0.5);

  dev_addr_label = gtk_label_new ("");
  gtk_widget_show (dev_addr_label);
  gtk_table_attach (GTK_TABLE (table6), dev_addr_label, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (dev_addr_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (dev_addr_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (dev_addr_label), 0, 0.5);

  dev_type_label = gtk_label_new ("");
  gtk_widget_show (dev_type_label);
  gtk_table_attach (GTK_TABLE (table6), dev_type_label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  GTK_WIDGET_SET_FLAGS (dev_type_label, GTK_CAN_FOCUS);
  gtk_label_set_selectable (GTK_LABEL (dev_type_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (dev_type_label), 0, 0.5);

  dev_type_title = gtk_label_new (_("Type:"));
  gtk_widget_show (dev_type_title);
  gtk_table_attach (GTK_TABLE (table6), dev_type_title, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (dev_type_title), 0, 0.5);

  label2 = gtk_label_new (_("Support"));
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label2);

  dialog_action_area2 = GTK_DIALOG (network_status_dialog)->action_area;
  gtk_widget_show (dialog_action_area2);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);

  helpbutton1 = gtk_button_new_from_stock ("gtk-help");
  gtk_dialog_add_action_widget (GTK_DIALOG (network_status_dialog), helpbutton1, GTK_RESPONSE_HELP);
  GTK_WIDGET_SET_FLAGS (helpbutton1, GTK_CAN_DEFAULT);

  configure_button = gtk_button_new ();
  gtk_widget_show (configure_button);
  gtk_dialog_add_action_widget (GTK_DIALOG (network_status_dialog), configure_button, 0);
  GTK_WIDGET_SET_FLAGS (configure_button, GTK_CAN_DEFAULT);

  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (configure_button), alignment2);

  hbox5 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox5);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox5);

  image2 = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox5), image2, FALSE, FALSE, 0);

  label42 = gtk_label_new_with_mnemonic (_("Con_figure"));
  gtk_widget_show (label42);
  gtk_box_pack_start (GTK_BOX (hbox5), label42, FALSE, FALSE, 0);

  close_button = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (close_button);
  gtk_dialog_add_action_widget (GTK_DIALOG (network_status_dialog), close_button, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (close_button, GTK_CAN_DEFAULT);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label16), combo_entry1);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (network_status_dialog, network_status_dialog, "network_status_dialog");
  GLADE_HOOKUP_OBJECT_NO_REF (network_status_dialog, dialog_vbox2, "dialog_vbox2");
  GLADE_HOOKUP_OBJECT (network_status_dialog, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (network_status_dialog, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (network_status_dialog, connection_frame, "connection_frame");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label3, "label3");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label43, "label43");
  GLADE_HOOKUP_OBJECT (network_status_dialog, connection_hbox, "connection_hbox");
  GLADE_HOOKUP_OBJECT (network_status_dialog, connection_table, "connection_table");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label5, "label5");
  GLADE_HOOKUP_OBJECT (network_status_dialog, status_label, "status_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label16, "label16");
  GLADE_HOOKUP_OBJECT (network_status_dialog, name_combo, "name_combo");
  GLADE_HOOKUP_OBJECT (network_status_dialog, combo_entry1, "combo_entry1");
  GLADE_HOOKUP_OBJECT (network_status_dialog, activity_frame, "activity_frame");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label4, "label4");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox8, "hbox8");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label44, "label44");
  GLADE_HOOKUP_OBJECT (network_status_dialog, table2, "table2");
  GLADE_HOOKUP_OBJECT (network_status_dialog, received_label, "received_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, sent_label, "sent_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label11, "label11");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label12, "label12");
  GLADE_HOOKUP_OBJECT (network_status_dialog, signal_strength_frame, "signal_strength_frame");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label48, "label48");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox14, "hbox14");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label49, "label49");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox15, "hbox15");
  GLADE_HOOKUP_OBJECT (network_status_dialog, signal_strength_bar, "signal_strength_bar");
  GLADE_HOOKUP_OBJECT (network_status_dialog, signal_strength_label, "signal_strength_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label1, "label1");
  GLADE_HOOKUP_OBJECT (network_status_dialog, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_frame, "inet4_frame");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label25, "label25");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox11, "hbox11");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label45, "label45");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_table, "inet4_table");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_addr_title, "inet4_addr_title");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_dest_title, "inet4_dest_title");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_bcast_title, "inet4_bcast_title");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_mask_title, "inet4_mask_title");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_addr_label, "inet4_addr_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_dest_label, "inet4_dest_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_bcast_label, "inet4_bcast_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet4_mask_label, "inet4_mask_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet6_frame, "inet6_frame");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label26, "label26");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox12, "hbox12");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label46, "label46");
  GLADE_HOOKUP_OBJECT (network_status_dialog, table5, "table5");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label39, "label39");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label40, "label40");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet6_addr_label, "inet6_addr_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, inet6_scope_label, "inet6_scope_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, dev_frame, "dev_frame");
  GLADE_HOOKUP_OBJECT (network_status_dialog, dev_label, "dev_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox13, "hbox13");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label47, "label47");
  GLADE_HOOKUP_OBJECT (network_status_dialog, table6, "table6");
  GLADE_HOOKUP_OBJECT (network_status_dialog, dev_addr_title, "dev_addr_title");
  GLADE_HOOKUP_OBJECT (network_status_dialog, dev_addr_label, "dev_addr_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, dev_type_label, "dev_type_label");
  GLADE_HOOKUP_OBJECT (network_status_dialog, dev_type_title, "dev_type_title");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label2, "label2");
  GLADE_HOOKUP_OBJECT_NO_REF (network_status_dialog, dialog_action_area2, "dialog_action_area2");
  GLADE_HOOKUP_OBJECT (network_status_dialog, helpbutton1, "helpbutton1");
  GLADE_HOOKUP_OBJECT (network_status_dialog, configure_button, "configure_button");
  GLADE_HOOKUP_OBJECT (network_status_dialog, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (network_status_dialog, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (network_status_dialog, image2, "image2");
  GLADE_HOOKUP_OBJECT (network_status_dialog, label42, "label42");
  GLADE_HOOKUP_OBJECT (network_status_dialog, close_button, "close_button");

  gtk_widget_grab_default (helpbutton1);
  return network_status_dialog;
}

