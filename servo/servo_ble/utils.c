#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <bluetooth/bluetooth.h>	
#include <bluetooth/hci.h>		
#include <bluetooth/hci_lib.h>
#include <dbus/dbus.h>
#include "utils.h"


int dbus_call(DBusConnection *conn, DBusError *err, const char *target, const char *interface, const char *method)
{
	int ret= -1;
	DBusMessage *msg = NULL;
	DBusPendingCall *pending = NULL;
	DBusMessage *reply = NULL;

	const char *err_key = NULL;
	const char *err_value = NULL;

	msg = dbus_message_new_method_call("org.bluez", target, interface, method);
	if (!msg) 
	{
		fprintf(stderr, "\ndbus_call: Failed to create message");
		goto clean;
	}
	
	if (strcmp(method, "RemoveDevice") == 0)
	{
		DBusMessageIter header;
		dbus_message_iter_init_append(msg, &header);

		if (!dbus_message_iter_append_basic(&header, DBUS_TYPE_OBJECT_PATH, &device_path))
		{
			fprintf(stderr, "\ndbus_call: Failed to append header");
			goto clean;
		}
	}

	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) 
	{
		fprintf(stderr, "\ndbus_call: Failed to send with reply");
		goto clean;
	}
	dbus_message_unref(msg);
	msg = NULL;

	dbus_pending_call_block(pending); 
	reply = dbus_pending_call_steal_reply(pending);
	if (!reply) 
	{
		fprintf(stderr, "\ndbus_call: Failed %s; No reply from D-Bus\n", method);
		goto clean;
	}

	if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
	{
		err_key = dbus_message_get_error_name(reply);
		fprintf(stderr,	"\ndbus_call: Failed %s {D-Bus returned error: %s}",
				method, err_key ? err_key : "\n (unknown error)");

		DBusMessageIter args;
		if (dbus_message_iter_init(reply, &args) && 
				dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_STRING)
		{
			dbus_message_iter_get_basic(&args, &err_value);
			fprintf(stderr, "\ndbus_call: Error Message %s", err_value ? err_value : "(none)");
		}
		goto clean;
	}
	fprintf(stdout, "\ndbus_call: %s success", method);
	ret = 0;

clean:
	if (dbus_error_is_set(err)) 
	{
		fprintf(stderr, "\ndbus_call: remaining error: %s", err->message);
		dbus_error_free(err);
	}
	if (reply)
		dbus_message_unref(reply);

	if (pending)
		dbus_pending_call_unref(pending);

	if (msg)
		dbus_message_unref(msg);

	fprintf(stdout, "\ndbus_call: clean sucess");
	return ret;
}

int hcisearch_device (DBusConnection *conn, DBusError *err, char *target, char *device_mac, size_t mac_len)
{
	fprintf(stdout, "\nsearch_device: HCI searching for %s", target);

	int max_ret = 10;
	int search_time = 5; //units of 1.28 seconds
	int flags = IREQ_CACHE_FLUSH; 	
	char dev_addr[19], name[248]; 
	int read_ret = -1;
	int user_input;

	int dev_id = hci_get_route(NULL); 
	if (dev_id <0)
	{
		fprintf(stderr, "\nsearch_device: Issue at: hci_get_route");
		goto clean;
	} 
	else 
	{
		fprintf(stdout, "\nsearch_device: Local adapter found");
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

	for (int i = 0; i < count_ret; i++)
	{
		ba2str(&(ret[i].bdaddr), dev_addr); 
		memset(name, 0, sizeof(name)); 
		read_ret = hci_read_remote_name(tunnel, &(ret[i].bdaddr), sizeof(name), name, 0);
		if (read_ret < 0)
		{
			printf("\nsearch_device: No device name retreived at read_ret");
			goto clean;
		}

		if (strcmp(name, target) == 0) 
		{
			fprintf(stdout, "\nsearch_device: Target Matched: Name: %s with MAC dev_address: %s", name, dev_addr);
			strncpy(device_mac, dev_addr, mac_len- 1);
			goto clean;
		}
		fprintf(stdout, "\nsearch_device: Other Devices found: {Name: %s},{MAC: %s}", name, dev_addr);
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
char* dbusdiscover_device(DBusConnection *conn, DBusError *err, const char* target, size_t size)
{
	fprintf(stdout, "\ndbusdiscover_device: Searching for %s", target);

	DBusMessage *msg = NULL;
	DBusMessageIter crawl_path, dict_1, string_val, array_iter, dict_2, variant;
	const char *object_path = NULL;
	const char *iface = NULL;
	const char *key_str = NULL;
	const char *val_str = NULL;
	char *object_path_result = NULL;

	dbus_bus_add_match(conn,
			"type='signal',interface='org.freedesktop.DBus.ObjectManager',member='InterfacesAdded'",
			err);
	dbus_connection_flush(conn);

	dbus_call(conn, err, BLE_ADAPTER, ADAPTER_1, "StartDiscovery");

	while (dbus_connection_read_write(conn, -1))
	{
		msg = dbus_connection_pop_message(conn);
		dbus_connection_dispatch(conn);
		if (!msg)
		{
			fprintf(stdout, "\nWaiting for connection");
			continue;
		}

		if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		{
			dbus_message_unref(msg);
			continue;
		}

		if (!dbus_message_iter_init(msg, &crawl_path))
		{
			dbus_message_unref(msg);
			continue;
		}

		if (dbus_message_iter_get_arg_type(&crawl_path) != DBUS_TYPE_OBJECT_PATH)
		{
			dbus_message_unref(msg);
			continue;
		}

		dbus_message_iter_get_basic(&crawl_path, &object_path);
		fprintf(stdout, "\ndbusdiscover_device: Object Path %s", object_path);

		dbus_message_iter_next(&crawl_path);
		if (dbus_message_iter_get_arg_type(&crawl_path) != DBUS_TYPE_ARRAY)
		{
			dbus_message_unref(msg);
			continue;
		}

		dbus_message_iter_recurse(&crawl_path, &dict_1);
		while (dbus_message_iter_get_arg_type(&dict_1) == DBUS_TYPE_DICT_ENTRY)
		{
			dbus_message_iter_recurse(&dict_1, &string_val);
			if (dbus_message_iter_get_arg_type(&string_val) != DBUS_TYPE_STRING)
			{
				dbus_message_iter_next(&dict_1);
				continue;
			}

			dbus_message_iter_get_basic(&string_val, &iface);
			if (strncmp(iface, DEVICE_1, sizeof(DEVICE_1)-1) == 0)
			{
				dbus_message_iter_next(&string_val);
				if (dbus_message_iter_get_arg_type(&string_val) != DBUS_TYPE_ARRAY)
				{
					dbus_message_iter_next(&dict_1);
					continue;
				}

				dbus_message_iter_recurse(&string_val, &array_iter);
				while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_DICT_ENTRY)
				{
					dbus_message_iter_recurse(&array_iter, &dict_2);
					if (dbus_message_iter_get_arg_type(&dict_2) != DBUS_TYPE_STRING)
					{
						dbus_message_iter_next(&array_iter);
						continue;
					}

					dbus_message_iter_get_basic(&dict_2, &key_str);
					dbus_message_iter_next(&dict_2);
					if (dbus_message_iter_get_arg_type(&dict_2) != DBUS_TYPE_VARIANT)
					{
						dbus_message_iter_next(&array_iter);
						continue;
					}

					dbus_message_iter_recurse(&dict_2, &variant);
					if (dbus_message_iter_get_arg_type(&variant) != DBUS_TYPE_STRING)
					{
						dbus_message_iter_next(&array_iter);
						continue;
					}

					dbus_message_iter_get_basic(&variant, &val_str);
					printf("\ndbusdiscover_device: %s : %s", key_str, val_str);

					if (strncmp(key_str, "Name", 4) == 0)
					{
						if (strncmp(val_str, target, size - 1) == 0)
						{
							object_path_result = strdup(object_path);
							dbus_message_unref(msg);
							dbus_call(conn, err, BLE_ADAPTER, ADAPTER_1, "StopDiscovery");
							return object_path_result;
						}
					}
					dbus_message_iter_next(&array_iter);
				}
			}
			dbus_message_iter_next(&dict_1);
		}
		dbus_message_unref(msg);
	}
	return NULL;
}


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

char *read_device(char *target, char* path)
{
	char file_contents[BUFF_SIZE];
	char event_num [8] = { 0 };
	int device_flag = 0;
	char *event_str = NULL;

	int device_file = open(path, O_RDONLY);
	if (device_file < 0)
	{
		fprintf(stderr, "\nread_device: Failed to read device path");
		goto clean;
	}

	ssize_t read_contents;
	while ((read_contents = read(device_file, file_contents, BUFF_SIZE - 1)) > 0) 
	{
		file_contents[read_contents] = '\0';

		char *line = strtok(file_contents, "\n");
		while (line != NULL)
		{
			printf("\nLine: %s", line);
			if (strstr(line, "Name=") && strstr(line, target))
			{
				device_flag = 1;
			}
			else  if (device_flag && strstr(line, "Handlers")) 
			{
				char *event = strstr(line, "event");
				if (event)
				{
					sscanf(event, "event%7s", event_num);
					device_flag = 0;
					break;
				}
			}
			line = strtok(NULL, "\n");
		}
		if (event_num[0] != 0)
		{
			break;
		}
	}
	if (event_num[0] == 0)
	{
		fprintf(stderr, "\nread_device: Device descriptor not found");
		goto clean;
	}
	
	event_str = malloc(PATH_BUFFER);
	if(!event_str)
	{
		fprintf(stderr, "\nread_device: Failed to allocate buffer"); 
		goto clean;
	}
	snprintf(event_str, PATH_BUFFER, "%sevent%s", DEVICE_INPUT_PATH, event_num);

clean:
	if (device_file >= 0)
	{
		close(device_file);
	}

	return event_str;
}

char *read_event(const char *path)
{
	char *event_buff = NULL;
	int file = open(path, RDONLY);
	if (file < 0)
	{
		fprintf(stderr, "\nread_event: Failed to open event file");
		goto clean;
	}

clean:
	if (file >= 0)
		close(file);

	return event_buff;
}
