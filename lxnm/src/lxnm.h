#ifndef LXNM_H
#define LXNM_H

#include "thread.h"
#include "handler.h"

#define LXNM_SOCKET "/var/run/lxnm.socket"

#define LXNM_PROTOCOL "1.0"

/* Command */
#define LXNM_VERSION                   0
#define LXNM_ETHERNET_UP               1
#define LXNM_ETHERNET_DOWN             2
#define LXNM_ETHERNET_REPAIR           3
#define LXNM_WIRELESS_UP               4
#define LXNM_WIRELESS_DOWN             5
#define LXNM_WIRELESS_REPAIR           6
#define LXNM_WIRELESS_CONNECT          7
#define LXNM_WIRELESS_SCAN             8

/* Encryption Type */
#define LXNM_ENCRYPTION_OFF     0
#define LXNM_ENCRYPTION_WEP     1
#define LXNM_ENCRYPTION_WPA_PSK 2

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
	LXNMPID cur_id;
	int sockfd;
	Setting *setting;
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
