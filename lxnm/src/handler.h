#ifndef LXNM_HANDLER_H
#define LXNM_HANDLER_H

#define LXNM_HANDLER_METHOD_INTERNAL	0
#define LXNM_HANDLER_METHOD_EXECUTE	1
#define LXNM_HANDLER_METHOD_MODULE	2

typedef struct {
	gint method;
	gchar *value;
} LXNMHandler;

#endif
