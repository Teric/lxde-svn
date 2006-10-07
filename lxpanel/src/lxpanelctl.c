/* Remote controller of lxpanel */

#include "lxpanelctl.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

static Display* dpy;

const char usage[] = "\nlxpanelctl - LXPanel Controller\n"
        "Usage: lxpanelctl <command>\n\n"
        "Available commands:\n"
        "menu\tshow system menu\n"
        "run\tshow run dialog\n"
        "config\tshow config dialog\n"
        "restart\trestart lxpanel\n"
        "exit\texit lxpanel\n\n";

int get_cmd( const char* cmd )
{
    if( ! strcmp( cmd, "menu") )
        return LXPANEL_CMD_SYS_MENU;
    else if( ! strcmp( cmd, "run") )
        return LXPANEL_CMD_RUN;
    else if( ! strcmp( cmd, "config") )
        return LXPANEL_CMD_CONFIG;
    else if( ! strcmp( cmd, "restart") )
        return LXPANEL_CMD_RESTART;
    else if( ! strcmp( cmd, "exit") )
        return LXPANEL_CMD_EXIT;
    return -1;
}

int main( int argc, char** argv )
{
    char *display_name = (char *)getenv("DISPLAY");
    XEvent ev;
    Window root;
    Atom cmd_atom;
    int cmd;

    if( argc < 2 )
    {
        printf( usage );
        return 1;
    }
    if( ( cmd = get_cmd( argv[1] ) ) == -1 )
        return 1;

    dpy = XOpenDisplay(display_name);
    if (dpy == NULL) {
        printf("Cant connect to display: %s\n", display_name);
        exit(1);
    }
    root = DefaultRootWindow(dpy);
    cmd_atom = XInternAtom(dpy, "_LXPANEL_CMD", False);
    memset(&ev, '\0', sizeof ev);
    ev.xclient.type = ClientMessage;
    ev.xclient.window = root;
    ev.xclient.message_type = cmd_atom;
    ev.xclient.format = 8;

    ev.xclient.data.l[0] = cmd;

    XSendEvent(dpy, root, False,
               SubstructureRedirectMask|SubstructureNotifyMask, &ev);
    XSync(dpy, False);
    XCloseDisplay(dpy);
    return 0;
}
