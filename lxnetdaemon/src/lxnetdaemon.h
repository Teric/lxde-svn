#ifndef LXNETDAEMON_H
#define LXNETDAEMON_H

#define LXNETDAEMON_SOCKET "/var/run/lxnetdaemon.socket"

/* Command */
#define LXND_VERSION                   0
#define LXND_ETHERNET_UP               1
#define LXND_ETHERNET_DOWN             2
#define LXND_ETHERNET_REPAIR           3
#define LXND_WIRELESS_UP               4
#define LXND_WIRELESS_DOWN             5
#define LXND_WIRELESS_REPAIR           6
#define LXND_WIRELESS_CONNECT          7

/* Encryption Type */
#define LXND_ENCRYPTION_OFF     0
#define LXND_ENCRYPTION_WEP     1
#define LXND_ENCRYPTION_WPA_PSK 2

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
