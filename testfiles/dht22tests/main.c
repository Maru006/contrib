#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>
#include <error.h>
#include <stdlib.h>
#include "utils.h"

#define CHIP0 "/dev/gpiochip0"
#define HANDSHAKE 3
#define BUFFER 80
#define THRESHOLD 60000

int main(void)
{
	//hardware definitions
	int powerlines[] = {17};
	int datalines[] = {27};
	int size = 1;
	int powerlinessize = sizeof(powerlines) / sizeof(powerlines[0]);
	int datalinessize = sizeof(datalines) / sizeof(datalines[0]);
	//timing handling
	struct timespec pause, start;
	//event handling
	int gpiod_wait, gpiod_read;
	//gpiod.h event buffer handling
	enum gpiod_edge_event_type event_type;
	struct gpiod_edge_event_buffer *handshakebuffer = gpiod_edge_event_buffer_new(HANDSHAKE);
	struct gpiod_edge_event_buffer *buffer = gpiod_edge_event_buffer_new(BUFFER);
	//gpiod.h event handling
	struct gpiod_edge_event *event;
	
	//Allow for Capacitor stablisation
	sleep(1);

	/*STEP 1: Supply power to DHT22. 
	 * When power is supplied to sensor, dont't send any instructions to the sensor within one second. This is in order to pass an unstable period*/

	//utils.h: Configure power and data lines as output
	struct output_requests *dht11power = request_out(powerlines, size, CHIP0, "DHTPower");
	struct output_requests *dht11data_configure = request_out(datalines, size, CHIP0, "DHTInput"); 
	//gpiod.h: Toggle lines HIGH
	gpiod_line_request_set_value(dht11power->gpiodline, powerlines[0], GPIOD_LINE_VALUE_ACTIVE);
	printf("\nPower line Toggled HIGH");
	gpiod_line_request_set_value(dht11data_configure-> gpiodline, datalines[0], GPIOD_LINE_VALUE_ACTIVE);
	printf("\nData line Toggled HIGH");
	//Wait 1 second to pass unstable period
	pause.tv_sec = 1;
	pause.tv_nsec = 0;
	nanosleep(&pause, NULL);

	/*STEP 2: MCU send out START SIGNAL to DHT22. 
	 * When communication between MCU and DHT22 begins...
	 * MCU must transform the datalines from HIGH to LOW for at least 1ms to ensure DHT22's detection.*/
	//gpiod.h: Toggle dataline ouput as LOW
	gpiod_line_request_set_value(dht11data_configure-> gpiodline, datalines[0], GPIOD_LINE_VALUE_INACTIVE);
	printf("\nData Line Toggled LOW");
	//Wait for 1ms
	pause.tv_sec = 0;
	pause.tv_nsec = 1000000;
	nanosleep(&pause, NULL);

	/*STEP 3: Then, MCU will wait between 20-40us for DHT22's response. 
	 * When DHT22 detect the acknowkedge signal from MCU. 
	 * If sucessful DHT22 will then pull down the line (from HIGH to LOW) for at least 80us in a response.*/

	//First, release dataline resources, which is currently providing output, into data to receive input gpiod_line_request_release(dht11data_configure->gpiodline);
	gpiod_line_settings_free(dht11data_configure->gpiodline_settings);
	gpiod_line_config_free(dht11data_configure->gpiodline_config);
	gpiod_request_config_free(dht11data_configure->gpiodreq_config);
	gpiod_line_request_release(dht11data_configure->gpiodline);

	//Configure line as as input line
	struct input_requests *dht11data_acknowledge = request_in(datalines, size, CHIP0, "DHTdata");

	//Perform configuration: Watch for data FALLING between 20-40us; 
	//It is best to expecting the max delay + DHT22 holding line for atleast 80us
	
	printf("\nHandshake Start");
	uint64_t time_handshaketimestamps, diff;
	for (int x = 0; x < 2; x++)
	{
		gpiod_wait = gpiod_line_request_wait_edge_events(dht11data_acknowledge->gpiodline, 40000);
		clock_gettime(CLOCK_MONOTONIC, &start);	
		gpiod_read = gpiod_line_request_read_edge_events(dht11data_acknowledge->gpiodline, handshakebuffer, HANDSHAKE);
		event = gpiod_edge_event_buffer_get_event(handshakebuffer, 0);
		event_type = gpiod_edge_event_get_event_type(event);
		time_handshaketimestamps = gpiod_edge_event_get_timestamp_ns(event);
		printf("\n%s : %llu",line_edge(event_type), time_handshaketimestamps);
		diff = elapsed(start);
		printf("\nElapsed %llu", diff);
	}
	printf("\nHandshake Verified");
	/*STEP4: Then MCU must transform datalines from LOW to HIGH for another 80us for DHT22's preperation to send data.*/ 

	//First, release dataline resources, which is currently data, to output
	gpiod_line_request_release(dht11data_acknowledge->gpiodline);
	gpiod_line_settings_free(dht11data_acknowledge->gpiodline_settings);
	gpiod_line_config_free(dht11data_acknowledge->gpiodline_config);
	gpiod_request_config_free(dht11data_acknowledge->gpiodreq_config);
	free(dht11data_acknowledge);

	//Configure dataline as output 
	struct output_requests *dht11data_start = request_out(datalines, size, CHIP0, "DHTdata");

	//Perform configuration: Toggle dataline HIGH
	gpiod_line_request_set_value(dht11data_start-> gpiodline, datalines[0], GPIOD_LINE_VALUE_ACTIVE);
	printf("\nData Line toggled HIGH");

	//wait for 80us
	pause.tv_sec = 0;
	pause.tv_nsec = 80000;
	nanosleep(&pause, NULL);

	//gpiod_line_request_set_value(dht11data_start-> gpiodline, datalines[0], GPIOD_LINE_VALUE_INACTIVE);

	/*STEP 5: MCU receives data from DHT22. 
	 * WHen DHT22 sends data to MCU, a START is determined by a LOW voltage that lasts 50us. 
	 * This is then before it releases the line: 
	 * either 26-28us (which means bit 0);	
	 * or 70us (which means bit 1) [release period]*/

	//First, release dataline resources, which is currently ouput, to data
	gpiod_line_request_release(dht11data_start->gpiodline);
	gpiod_line_settings_free(dht11data_start->gpiodline_settings);
	gpiod_line_config_free(dht11data_start->gpiodline_config);    
	gpiod_request_config_free(dht11data_start->gpiodreq_config);

	//Configure data as input
	struct input_requests *dht11data_listen = request_in(datalines, size, CHIP0, "DHTdata");
	printf("\nData Line changed as INPUT");

	/* Translate DHT22 release periods that follow. 
	 * Expect a total bit transfer consisting of: 
	 * 1: 8 bit integral Relative Humidity data, 
	 * 2: 8 bit decimal Relative Humidity data, 
	 * 3: 8 bit integral temperature data, 
	 * 4: 8 bit decimal temperature data, 
	 * 5: 8 bit check-sum data
	 * A total of 40 pair: LOW 50us with a HIGH 26-28 ('0') or 70 ('1') afterwards = 80 bits in total*/

	//gpiod.h: Wait for RISING and FALLING edges and store them in arrays.
	const char *events_event[80];
	uint64_t time_eventtimestamps[80] = {0};
	uint64_t time_event, time_pulse;
	size_t total_buffer;
	//Wait -> Detect -> Store -> wait -> ...
	for (int index = 0; index < 40;) 
	{
		gpiod_wait = gpiod_line_request_wait_edge_events(dht11data_listen->gpiodline, 80000);
		gpiod_read = gpiod_line_request_read_edge_events(dht11data_listen->gpiodline, buffer, 0);
		total_buffer = gpiod_edge_event_buffer_get_num_events(buffer);
		printf("\nBuffer Waiting: %zu", total_buffer);
		event = gpiod_edge_event_buffer_get_event(buffer, 0);
		event_type = gpiod_edge_event_get_event_type(event);
		time_eventtimestamps[index] = gpiod_edge_event_get_timestamp_ns(event);
		if(event_type == GPIOD_EDGE_EVENT_RISING_EDGE)
		{
			//printf("\nIndex %d", index);
			events_event[index] = line_edge(event_type);
			printf("\nRISING: %llu", time_eventtimestamps[index]);
			index++; printf("\nIndex: %d", index);
		}	
		else if(event_type == GPIOD_EDGE_EVENT_FALLING_EDGE)
		{
			//printf("\nIndex %d", index);
			events_event[index] = line_edge(event_type);
			printf("\nFALLING: %llu", time_eventtimestamps[index]);
		}
	}
	
	int time_diff;
	for (int low = 0, high = 0 ; low < 40; low++)
	{
		time_diff = time_eventtimestamps[low] - time_eventtimestamps[high];
		printf("\n Time Diff: %d", time_diff);
	}
	
clean:
	printf("\nCleaning...");
	gpiod_line_settings_free(dht11data_listen->gpiodline_settings);
	gpiod_line_config_free(dht11data_listen->gpiodline_config);
	gpiod_request_config_free(dht11data_listen->gpiodreq_config);
	gpiod_line_request_release(dht11data_listen->gpiodline);

	gpiod_edge_event_buffer_free(handshakebuffer);
	gpiod_edge_event_buffer_free(buffer);
	gpiod_chip_close(dht11data_listen->gpiodchip);
	free(dht11data_listen);

	gpiod_line_request_set_value(dht11power->gpiodline, powerlines[0], GPIOD_LINE_VALUE_INACTIVE);

	gpiod_line_settings_free(dht11power->gpiodline_settings);
	gpiod_line_config_free(dht11power->gpiodline_config);
	gpiod_request_config_free(dht11power->gpiodreq_config);
	gpiod_line_request_release(dht11power->gpiodline);
	gpiod_chip_close(dht11power->gpiodchip);

	printf("\nClean Finish\n");
	return EXIT_SUCCESS;

}










