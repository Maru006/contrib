#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>

// strict order
#include <bluetooth/bluetooth.h> 
#include <bluetooth/rfcomm.h>

#include <string.h>
#include <dbus/dbus.h>
#include "utils.h"

char *device_path = NULL;

int main(int argc, char *argv[])
{
	if (argc > 1 && strcmp(argv[1], "--stderr-only") == 0)
	{
		freopen("/dev/null", "w", stdout);
		setvbuf(stderr, NULL, _IONBF, 0);
	}
	else
	{
		setvbuf(stdout, NULL, _IONBF, 0);
		setvbuf(stderr, NULL, _IONBF, 0);
	}

	char device_name[] = "8BitDo Pro 2";
	size_t size = sizeof(device_name)/sizeof(device_name[0]);
	char ble_path[PATH_BUFFER] = {0};
	char *mask_path = NULL;
	char *event_buff = NULL;

	DBusError err; 
	dbus_error_init(&err); 
	DBusConnection *conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err); 
	if (!conn)
	{
		fprintf(stderr, "main: failed to connect to system bus: %s\n",
				err.message ? err.message : "(unknown)");
		goto clean;
	}
	device_path = dbusdiscover_device(conn, &err, device_name, size);

	if (device_path == NULL)
		goto clean;

	if(dbus_call(conn, &err, device_path, DEVICE_1, "Pair") < 0)
		goto clean;

	if(dbus_call(conn, &err, device_path, DEVICE_1, "Connect") < 0)
		goto clean;
	
	mask_path = read_device(device_name, DEVICES);
	if (!mask_path)
	{
		fprintf(stderr,"\nmain: Issue finding device input path");
		goto clean;
	}
	fprintf(stdout, "\nmain: mask_path: %s",mask_path);
	
	printf("\nBegin inputs");
	event_buff = read_event(mask_path);
	

clean:
	if(dbus_call(conn, &err, BLE_ADAPTER, ADAPTER_1, "RemoveDevice") < 0)	
		fprintf(stdout, "\nmain: Issue Disconnecting (Forgetting) Device");

	if(mask_path)
		free(mask_path);
	
	if(event_buff)
		free(event_buff);

	fprintf(stdout, "\nmain: Disconnected from: %s", device_path);
	fprintf(stdout, "\n");
	return 0;
}


