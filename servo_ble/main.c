#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <bluetooth/bluetooth.h>
#include <string.h>
#include "utils.h"

void format_path(char *in)
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
char *read_device(char *target)
{
	char file_contents[BUFF_SIZE];
	char event_num [8] = { 0 };
	int device_flag = 0;
	char *event_str = NULL;

	int device_file = open(DEVICE_PATH, O_RDONLY);
	if (device_file < 0)
	{
		fprintf(stderr, "\nread_device: Failed to read device path");
		goto clean;
	}

	ssize_t read_contents = read(device_file, file_contents, BUFF_SIZE - 1); 
	if (read_contents < 0)
	{
		fprintf(stderr, "\nread_device: Issue at reading contents at file");
		goto clean;
	}
	file_contents[read_contents] = '\0';
	char *line = strtok(file_contents, "\n");
	
	while (line != NULL)
	{
		if (strstr(line, "Name=") && strstr(line, target))
		{
			device_flag = 1;
		}
		else if (device_flag && strstr(line, "Handlers")) 
		{
			char *event = strstr(line, "event");
			if (event)
			{
				sscanf(event, "event%7s", event_num);
				break;
			}
		}
		if(line[0] == '\0')
		{
			device_flag = 0;
		}
		line = strtok(NULL, "\n");
	}

	event_str = malloc(PATH_BUFFER);
	if(!event_str)
	{
		fprintf(stderr, "\nFailed to allocate memory");
		goto clean;
	}

	snprintf(event_str, PATH_BUFFER, "%s%s", DEVICE_INPUT_PATH, event_num);

	if (device_file >= 0)
	{
		close(device_file);
	}

	return event_str;

clean:
	if (device_file >= 0)
	{
		close(device_file);
	}
	if (event_str)
	{
		free(event_str);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc > 1 && strcmp(argv[1], "--stderr-only") == 0)
	{
		freopen("dev/null", "w", stdout);
		setvbuf(stderr, NULL, _IONBF, 0);
	}
	else
	{
		setvbuf(stdout, NULL, _IONBF, 0);
		setvbuf(stderr, NULL, _IONBF, 0);
	}

	char device_name[] = "8BitDo Pro 2";
	char device_mac[DEVICE_MAC_BUFFER] = {0};
	char ble_path[BLE_PATH_BUFFER] = {0};
	char* mask_path = NULL;
	size_t size_mac = sizeof(device_mac)/sizeof(device_mac[0]);
	
	if (search_device(device_name, device_mac, DEVICE_MAC_BUFFER) < 0)
	{
		fprintf(stderr, "\nmain: Failed search for device");
		goto clean;
	}

	format_path(device_mac);	
	snprintf(ble_path, sizeof(ble_path), "%s/dev_%s", BLE_ADAPTER, device_mac);
	fprintf(stdout, "\nmain: device path: is %s", ble_path);
	
	if(dbus_call(ble_path, DEVICE_1, "Pair") < 0)
	{
		fprintf(stderr, "\nIssue Pairing");
		goto clean;
	}

	if(dbus_call(ble_path, DEVICE_1, "Connect") < 0)
	{
		fprintf(stderr, "\nmain: Issue at Connecting");
		goto clean;
	}
	fprintf(stdout, "\nmain: Connected to %s", ble_path);


	mask_path = read_device(device_name);

	if (!mask_path)
	{
		fprintf(stderr,"\nmain: Issue finding device input path");
		goto clean;
	}
	printf("\nmain: Imask_path: %s",mask_path);	
clean:
	if (mask_path)
	{
		free(mask_path);
	}
	if (dbus_call(ble_path, DEVICE_1,"Disconnect") < 0)
	{
		fprintf(stderr, "\nmain: Issue disconnecting");
	}
	else fprintf(stdout, "\nmain: Disconnected from: %s", device_mac);
	
	fprintf(stdout, "\n");
	return 0;
}


