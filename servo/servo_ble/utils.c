#include <stdio.h>        		
#include <string.h>       		
#include <stdlib.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>	//Bluez bluetooth stack: ba2str, bdaddr_t
#include <bluetooth/hci.h>		//Bluez Low level interface -> Bluetooth adapter 
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <dbus/dbus.h>
#include "utils.h"


int dbus_call(const char *target, const char *interface, const char *method)
{
	int ret= -1;

	DBusError error;
	DBusConnection *conn = NULL;
	DBusMessage *form = NULL;
	DBusPendingCall *send = NULL;
	DBusMessage *reply = NULL;

	dbus_error_init(&error);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if (!conn) 
	{
		fprintf(stderr, "\ndbus_call: Failed to connect to system bus");
		goto clean;
	}

	form = dbus_message_new_method_call("org.bluez", target, interface, method);
	if (!form) 
	{
		fprintf(stderr, "\ndbus_call: Failed to create message");
		goto clean;
	}

	if (!dbus_connection_send_with_reply(conn, form, &send, -1)) {
		fprintf(stderr, "\ndbus_call: Failed to send with reply");
		goto clean;
	}

	dbus_message_unref(form);
	form = NULL; // to prevent double unref at clean

	// dbus_connection_flush(conn); forces buffered event into standard output

	dbus_pending_call_block(send); // blocking will flush for each event
	reply = dbus_pending_call_steal_reply(send);
	if (!reply) 
	{
		fprintf(stderr, "\ndbus_call: No reply from D-Bus\n");
		goto clean;
	}

	if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
	{
		const char *err_name = dbus_message_get_error_name(reply);
		fprintf(stderr, "\ndbus_call: D-Bus returned error: %s\n", err_name ? err_name : "\n (unknown error)");
		goto clean;
	}

	fprintf(stdout, "%sdbus_call: success\n", method);

	ret = 0;

clean:
	if (dbus_error_is_set(&error)) 
	{
		fprintf(stderr, "\ndbus_call: remaining error: %s\n", error.message);
		dbus_error_free(&error);
	}
	if (reply)
		dbus_message_unref(reply);
	
	if (send)
		dbus_pending_call_unref(send);

	if (form)
		dbus_message_unref(form);

	fprintf(stdout, "\ndbus_call: clean sucess");

	return ret;
}

int search_device (char *target, char *device_mac, size_t mac_len)
{
	int max_ret = 10;
	int search_time = 5; //units of 1.28 seconds
	int flags = IREQ_CACHE_FLUSH; 	

	if(dbus_call(BLE_ADAPTER, ADAPTER_1, "StartDiscovery"))
	{
		fprintf(stderr, "\nsearch_device: Issue starting Discovery");
		goto clean;
	}
	else
		fprintf(stdout, "\nSearching for %s...", target);
	
	sleep(10);
	if(dbus_call(BLE_ADAPTER, ADAPTER_1, "StopDiscovery"))
	{
		fprintf(stderr, "\nsearch_device: Issue stopping Discovery");
		goto clean;
	}	

	int dev_id = hci_get_route(NULL); 
	if (dev_id <0)
	{
		fprintf(stderr, "\nsearch_device: Issue at: hci_get_route");
		goto clean;
	} 
	else 
	{
		fprintf(stdin, "\nsearch_device: Local adapter found");
	}

	int tunnel = hci_open_dev(dev_id);
	if (tunnel < 0)
	{
		fprintf(stderr, "\nsearch_device: Issue at: hci_open_dev");
		goto clean;
	}
	fprintf(stdout, "\nsearch_device: Searching for %s", target);
	inquiry_info *ret = NULL; 
	int count_ret = hci_inquiry(dev_id, search_time, max_ret, NULL, &ret, flags);
	if (count_ret < 0)
	{
		fprintf(stderr, "\nsearch_device: Issue searching");
		goto clean;
	}
	else
	{
		fprintf(stdout, "\nsearch_device: Total devices detected %d", count_ret);
	}

	char addr[19], name[248]; 
	int read_ret, user_input;

	for (int i = 0; i < count_ret; i++)
	{
		ba2str(&(ret[i].bdaddr), addr); 
		memset(name, 0, sizeof(name)); 
		read_ret = hci_read_remote_name(tunnel, &(ret[i].bdaddr), sizeof(name), name, 0);
		if (read_ret < 0)
		{
			printf("\nsearch_device: No device name retreived at read_ret");
			goto clean;
		}

		if (strcmp(name, target) == 0) 
		{
			fprintf(stdout, "\nsearch_device: Target Matched: Name: %s with MAC address: %s", name, addr);
			strncpy(device_mac, addr, mac_len- 1);
			goto clean;
		}
		fprintf(stdout, "\nsearch_device: Other Devices found: {Name: %s},{MAC: %s}", name, addr);
	}

clean:
	if (ret != NULL)
	{
		free(ret);
		fprintf(stdout, "\nsearch_device: Free results");
	}
	if (tunnel >= 0)
	{
		close(tunnel);
		fprintf(stdout, "\nsearch_device: Free tunnel");
	}
	if (read_ret < 0)
	{
		return -1;
	}
	if (read_ret > 0)
	{
		return 0;
	}
	fprintf(stdout, "\nsearch_device: clean sucess");
}




