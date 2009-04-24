#ifndef LXNM_THREAD_H
#define LXNM_THREAD_H

typedef unsigned int LXNMPID;

typedef struct {
	GIOChannel *gio;
	gchar *cmd;
} LxThread;

LXNMPID lxnm_pid_register(GIOChannel *gio);
void lxnm_pid_unregister(GIOChannel *gio, LXNMPID id);

#endif
