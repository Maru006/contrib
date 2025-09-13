#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h> // probably for read().
#include <unistd.h>
#include <linux/input.h>
#include <bluetooth/bluetooth.h>
#include "utils.h"

void format(char *in)
{
	char sub = '_';
	for (int i = 0; in[i] != '\0'; i++)
	{
		if (in[i] == ':')
		{
			in[i] = sub;
		}
	}
}

int main(void)
{
	char device_mac[19] = {0};

	size_t size_mac = sizeof(device_mac)/sizeof(device_mac[0]);

	if (search_device("8BitDo Pro 2", device_mac, size_mac) < 0)
	{
		printf("\nFailed search for device");
		return 1;
	}
	format(device_mac);	
	printf("\nUser-Friendly Device Mac: %s", device_mac);

	char path[64];
	char adapter[] = "hci0";
	snprintf(path, sizeof(path), "/org/bluez/%s/dev_%s", adapter, device_mac);
	if(dbus_call(path, "Pair") < 0)
	{
		perror("Issue Pairing");
	}

	if(dbus_call(path, "Connect") < 0)

	{
		perror("Issue at Connecting");
	}
	printf("\nConnected to %s", device_mac);
	sleep(10);

clean:
	if (dbus_call(path, "Disconnect") < 0)
	{
		perror("Issue at DIsconnecting");
	}
	else printf("\ndisconnected from: %s", device_mac);

	return 0;
}


