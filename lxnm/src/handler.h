#ifndef LXNM_HANDLER_H
#define LXNM_HANDLER_H

#define LXNM_HANDLER_METHOD_INTERNAL	0
#define LXNM_HANDLER_METHOD_EXECUTE	1
#define LXNM_HANDLER_METHOD_MODULE	2

struct _LXNMHandler {
	gint method;
	gchar *value;
};

LXNMHandler *lxnm_handler_new(const gchar *strings);
int lxnm_handler_version(LxThread *lxthread);
int lxnm_handler_device_status(LxThread *lxthread);
int lxnm_handler_ethernet_up(LxThread *lxthread);
int lxnm_handler_ethernet_down(LxThread *lxthread);
int lxnm_handler_ethernet_repair(LxThread *lxthread);
int lxnm_handler_wireless_up(LxThread *lxthread);
int lxnm_handler_wireless_down(LxThread *lxthread);
int lxnm_handler_wireless_repair(LxThread *lxthread);
int lxnm_handler_wireless_scan(LxThread *lxthread);

#endif
