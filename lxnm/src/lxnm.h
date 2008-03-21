#ifndef LXNM_H
#define LXNM_H

#define LXNM_SOCKET "/var/run/lxnm.socket"

/* Command */
#define LXNM_VERSION                   0
#define LXNM_ETHERNET_UP               1
#define LXNM_ETHERNET_DOWN             2
#define LXNM_ETHERNET_REPAIR           3
#define LXNM_WIRELESS_UP               4
#define LXNM_WIRELESS_DOWN             5
#define LXNM_WIRELESS_REPAIR           6
#define LXNM_WIRELESS_CONNECT          7

/* Encryption Type */
#define LXNM_ENCRYPTION_OFF     0
#define LXNM_ENCRYPTION_WEP     1
#define LXNM_ENCRYPTION_WPA_PSK 2

typedef struct {
	gchar *eth_up;
	gchar *eth_down;
	gchar *eth_repair;
	gchar *wifi_up;
	gchar *wifi_down;
	gchar *wifi_repair;
	gchar *wifi_connect;
} Setting;

typedef struct {
	int sockfd;
	Setting *setting;
} LxND;

#endif
