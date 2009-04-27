#ifndef LXNM_THREAD_H
#define LXNM_THREAD_H

typedef unsigned int LXNMPID;

typedef struct {
	GIOChannel *gio;
	GIOChannel *gio_in;
	gchar *cmd;
} LxThread;

void lxnm_send_message(GIOChannel *gio, const gchar *msg);
LXNMPID lxnm_pid_register(GIOChannel *gio);
void lxnm_pid_unregister(GIOChannel *gio, LXNMPID id);

#endif
