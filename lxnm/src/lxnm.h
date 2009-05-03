#ifndef LXNM_H
#define LXNM_H

#include "port.h"

#define LXNM_SOCKET "/var/run/lxnm.socket"

#define LXNM_PROTOCOL "1.0"

/* Command */
typedef enum {
	LXNM_VERSION,
	LXNM_DEVICE_LIST,
	LXNM_DEVICE_STATUS,
	LXNM_DEVICE_INFORMATION,
	LXNM_ETHERNET_UP,
	LXNM_ETHERNET_DOWN,
	LXNM_ETHERNET_REPAIR,
	LXNM_WIRELESS_UP,
	LXNM_WIRELESS_DOWN,
	LXNM_WIRELESS_REPAIR,
	LXNM_WIRELESS_CONNECT,
	LXNM_WIRELESS_SCAN
} LXNMCommand;

typedef unsigned int LXNMPID;
typedef unsigned int LXNMClientID;

typedef enum {
	LXNM_ENCRYPTION_OFF,
	LXNM_ENCRYPTION_WEP,
	LXNM_ENCRYPTION_WPA
} EncryptionMethod;

typedef enum {
	LXNM_KEYMGMT_NONE,
	LXNM_KEYMGMT_8021X,
	LXNM_KEYMGMT_PSK
} IEKeyMgmt;

typedef enum {
	LXNM_CYPHER_NONE,
	LXNM_CYPHER_WEP40,
	LXNM_CYPHER_WEP104,
	LXNM_CYPHER_TKIP,
	LXNM_CYPHER_WRAP,
	LXNM_CYPHER_CCMP
} IECypher;

typedef struct _LXNMHandler LXNMHandler;
typedef struct {
	LXNMHandler *iflist;
	LXNMHandler *eth_up;
	LXNMHandler *eth_down;
	LXNMHandler *eth_repair;
	LXNMHandler *wifi_up;
	LXNMHandler *wifi_down;
	LXNMHandler *wifi_repair;
	LXNMHandler *wifi_connect;
	LXNMHandler *wifi_scan;
} Setting;

typedef struct {
	LXNMClientID  id;
	GIOChannel   *gio;
} LXNMClient;

typedef struct {
	LXNMPID       cur_id;
	LXNMClientID  cur_cid;
	int      sockfd;
	Setting *setting;
	GList   *ifstatus;
} LxND;

typedef struct {
	char *ifname;
	char *essid;
	char *apaddr;
	char *key;
	char *protocol;
	char *key_mgmt;
	char *group;
	char *pairwise;
} wificonn;

#endif
