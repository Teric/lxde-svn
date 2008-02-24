#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include <gtk/gtk.h>

/* mixer */
static gint mixer_fd;
static gint mixer_devmask;
static gint mixer_stereodevs;
static const gchar *mixer_devices[] = SOUND_DEVICE_NAMES;

static gboolean open_mixer(void)
{
    /* open mixer device */
    if((mixer_fd = open("/dev/mixer", O_RDONLY)) == -1)
        return FALSE;

    /* get mixer device mask */
    if(ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &mixer_devmask) == -1)
        return FALSE;

    if(ioctl(mixer_fd, SOUND_MIXER_READ_STEREODEVS, &mixer_stereodevs) == -1)
        return FALSE;

    return TRUE;
}

static void close_mixer(void)
{
    close(mixer_fd);
}

gboolean mixer_set(const gchar *dev, gint value)
{
    gint device;
    gint level;
    gint i;

    if(dev[0])
        for(i = 0; i < SOUND_MIXER_NRDEVICES; i++)
            if((1 << i) & mixer_devmask)
                if(strcmp(dev, mixer_devices[i]) == 0)
                    device = i;

    level = (value << 8) + value;

    if (ioctl(mixer_fd, MIXER_WRITE(device), &level) == -1)
        return FALSE;

    return TRUE;
}

gboolean mixer_get(const gchar *dev, gint *value)
{
    gint device;
    gint level;
    gint i;

    if(dev[0])
        for(i = 0; i < SOUND_MIXER_NRDEVICES; i++)
            if((1 << i) & mixer_devmask)
                if(strcmp(dev, mixer_devices[i]) == 0)
                    device = i;

    if(ioctl(mixer_fd, MIXER_READ(device), &level) == -1)
        return FALSE;

    *value = level >> 8;

    return TRUE;
}

gint init_mixer(void)
{
    gint vol;
    open_mixer();

    return TRUE;
}

gint uninit_mixer(void)
{
    close_mixer();

    return TRUE;
}
