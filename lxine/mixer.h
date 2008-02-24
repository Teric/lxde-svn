#ifndef HAVE_MIXER_H
#define HAVE_MIXER_H

gboolean mixer_set(const gchar *dev, gint value);
gboolean mixer_get(const gchar *dev, gint *value);
gint init_mixer(void);
gint uninit_mixer(void);

#endif
