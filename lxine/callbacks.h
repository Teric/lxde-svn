#ifndef HAVE_CALLBACKS_H
#define HAVE_CALLBACKS_H

void on_seeker_value_changed (GtkRange *range, gpointer user_data);
void on_vol_value_changed (GtkRange *range, gpointer user_data);
void play_clicked(GtkWidget *widget, gpointer data);
void stop_clicked(GtkWidget *widget, gpointer data);
gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void destroy(GtkWidget *widget, gpointer data);

#endif
