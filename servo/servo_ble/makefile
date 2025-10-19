CC = gcc
BLE = $(shell pkg-config --cflags --libs dbus-1)
LINK = -l bluetooth

all : joy_bt

joy_bt: main.c utils.c 
	$(CC) $^ $(BLE) $(LINK) -o $@
clean:
	rm -r joy_bt
