#ifndef LXNM_STATUS_H
#define LXNM_STATUS_H

typedef enum {
	LXNM_CONNECTION_TYPE_ETHERNET,
	LXNM_CONNECTION_TYPE_WIRELESS,
	LXNM_CONNECTION_TYPE_PPP
} DeviceType;

typedef struct {
	gint        ref;
	gchar      *ifname;
	DeviceType  type; 
	gpointer    info;
	GList      *clients; /* who is listening for this interface */
} InterfaceStatus;

typedef struct {
	gboolean enable;
	gboolean plugged;
	gboolean connected;

	gchar *ifname;
	gchar *mac;
	gchar *ipaddr;
	gchar *dest;
	gchar *bcast;
	gchar *mask;

	gulong recv_bytes;
	gulong recv_packets;
	gulong trans_bytes;
	gulong trans_packets;
} EthernetInfo;

typedef struct {
	gboolean enable;
	gboolean plugged;
	gboolean connected;

	gchar *ifname;
	gchar *mac;
	gchar *ipaddr;
	gchar *dest;
	gchar *bcast;
	gchar *mask;

	gulong recv_bytes;
	gulong recv_packets;
	gulong trans_bytes;
	gulong trans_packets;

	/* AP */
	gchar            *essid;
	gchar            *bssid;
	gint              quality;
	EncryptionMethod  encryption;
	IEKeyMgmt         keymgmt;
	IECypher          group;
	IECypher          pairwise;
} WirelessInfo;

void lxnm_status_register(const gchar *ifname, DeviceType devtype);
void lxnm_status_unregister(const gchar *ifname);
DeviceType lxnm_status_get_device_type(const gchar *ifname);

#endif
