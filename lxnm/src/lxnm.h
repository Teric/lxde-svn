#ifndef LXNM_H
#define LXNM_H

#include "thread.h"
#include "handler.h"

#define LXNM_SOCKET "/var/run/lxnm.socket"

#define LXNM_PROTOCOL "1.0"

/* Command */
#define LXNM_VERSION                   0
#define LXNM_DEVICE_STATUS             1
#define LXNM_DEVICE_INFORMATION        2
#define LXNM_ETHERNET_UP               3
#define LXNM_ETHERNET_DOWN             4
#define LXNM_ETHERNET_REPAIR           5
#define LXNM_WIRELESS_UP               6
#define LXNM_WIRELESS_DOWN             7
#define LXNM_WIRELESS_REPAIR           8
#define LXNM_WIRELESS_CONNECT          9
#define LXNM_WIRELESS_SCAN             10

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

typedef struct {
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
	LXNMPID  cur_id;
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
