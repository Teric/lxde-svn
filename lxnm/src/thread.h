#ifndef LXNM_THREAD_H
#define LXNM_THREAD_H

typedef unsigned int LXNMPID;

LXNMPID lxnm_pid_register(GIOChannel *gio);
void lxnm_pid_unregister(GIOChannel *gio, LXNMPID id);

#endif
