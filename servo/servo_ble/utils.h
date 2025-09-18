#ifndef UTILS_H
#define UTILS_H

#define BLE_ADAPTER "/org/bluez/hci0"
#define DEVICE_1 "org.bluez.Device1"
#define ADAPTER_1 "org.bluez.Adapter1"

#define AXIS_DEADZONE 2000000
#define DEVICE_PATH "/proc/bus/input/devices"
#define DEVICE_INPUT_PATH "/dev/input/event"

#define PATH_BUFFER 126
#define BUFF_SIZE 4096
#define BLE_PATH_BUFFER 256
#define DEVICE_MAC_BUFFER 19

/* 		----------Events----------
 * Event type 0 (EV_SYN)
  Event type 1 (EV_KEY)
    Event code 304 (BTN_SOUTH) 	A
    Event code 305 (BTN_EAST) 	B
    Event code 306 (BTN_C)	LEG RIGHT
    Event code 307 (BTN_NORTH) 	X
    Event code 308 (BTN_WEST) 	Y
    Event code 309 (BTN_Z)	LEG LEFT

   
    --------------------BUTTON TRIGGERS-------------------
		----------LEFT TRIGGER BUTTON----------
    Event code 310 (BTN_TL)
		----------RIGHT TRIGGER BUTTON----------
    Event code 311 (BTN_TR)
    --------------------------------------------------

    Event code 312 (BTN_TL2)
    Event code 313 (BTN_TR2)
    Event code 314 (BTN_SELECT)
    Event code 315 (BTN_START)
    Event code 316 (BTN_MODE)
    Event code 317 (BTN_THUMBL)
    Event code 318 (BTN_THUMBR)
    Event code 319 (?)


    --------------------JOYSTICKS-------------------
  Event type 3 (EV_ABS)
   		 ----------LEFT JOYSTICKS---------
    Event code 0 (ABS_X)	
      Value    127 		UP
      Min        0
      Max      255		UP-RIGHT
      Flat      15		
    Event code 1 (ABS_Y)	
      Value    127 		LEFT
      Min        0
      Max      255		DOWN-RIGHT
      Flat      15
		----------RIGHT JOYSTICK-----------
    Event code 2 (ABS_Z)
      Value    127		DOWN
      Min        0		UP-LEFT
      Max      255		UP-RIGHT
      Flat      15		
    Event code 5 (ABS_RZ)
      Value    127		RIGHT
      Min        0		UP
      Max      255		DOWN-LEFT
      Flat      15
      Resolution      46
    --------------------------------------------------
    

    --------------------TRIGGERS-------------------
		----------RIGHT TRIGGER----------
      Event code 9 (ABS_GAS)
      Value      0
      Min        0
      Max      255
      Flat      15
		----------LEFT TRIGGER----------
    Event code 10 (ABS_BRAKE)
      Value      0
      Min        0
      Max      255
      Flat      15
    --------------------------------------------------


    --------------------ARROW KEYS-------------------
    Event code 16 (ABS_HAT0X)
      Value      0 (!= TOGGLE)
      Min       -1 (TOGGLE LEFT)
      Max        1 (TOGGLE RIGHT)
    Event code 17 (ABS_HAT0Y)
      Value      0 (!= TOGGLE)
      Min       -1 (TOGGLE UP)
      Max        1 (TOGGLE DOWN)
 --------------------------------------------------
  

    Event type 4 (EV_MSC)
    Event code 4 (MSC_SCAN)
*/

int search_device(char *target, char *device_mac, size_t mac_len);

int dbus_call(const char *target, const char* interface, const char *method);

#endif
