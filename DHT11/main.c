#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>
#include <stdlib.h>

#include "../utils/utils.h"

#define CHIP0 "/dev/gpiochip0"
#define BUFFER 100
#define THRESHOLD 60000


const char* line_edge(enum gpiod_edge_event_type edge) {
	switch (edge) {
		case GPIOD_EDGE_EVENT_FALLING_EDGE:
			return "FALLING";
		case GPIOD_EDGE_EVENT_RISING_EDGE:
			return "RISING";
		default:
			return "Unknown";
	}
}

int main(void)
{
	struct timespec pause;
	struct timespec start;
	int powerlines[] = {23};
	int datalines[]  = {24};
	size_t size = 1;
	enum gpiod_edge_event_type event_type;
	uint64_t start_time, current_time = 0, event_time;
	int response, read;
	struct gpiod_edge_event_buffer *handshakebuffer = gpiod_edge_event_buffer_new(3);
	struct gpiod_edge_event_buffer *buffer = gpiod_edge_event_buffer_new(BUFFER);
	struct gpiod_edge_event *event = NULL;





	struct output_requests* dht11power = request_out(
			powerlines, 
			size, 
			CHIP0, 
			"DHTPower");
	struct output_requests* dht11calibrate = request_out(
			datalines,   
			size, 
			CHIP0, 
			"DHTCalibrate"); 
	gpiod_line_request_set_value(
			dht11power->gpiodline, 
			powerlines[0],
			GPIOD_LINE_VALUE_ACTIVE);
	gpiod_line_request_set_value(
			dht11calibrate-> gpiodline,
			datalines[0],
			GPIOD_LINE_VALUE_ACTIVE);
	printf("\nPower Line toggled on HIGH & Data Line toggled on HIGH");
	pause.tv_sec = 2;
	pause.tv_nsec = 0;
	nanosleep(&pause, NULL);






	gpiod_line_request_set_value(
			dht11calibrate-> gpiodline,
			datalines[0],
			GPIOD_LINE_VALUE_INACTIVE);
	printf("\nData Line toggled LOW");
	pause.tv_sec = 0;
	pause.tv_nsec = 18000000;
	nanosleep(&pause, NULL);




	gpiod_line_request_release(dht11calibrate->gpiodline);
	gpiod_line_settings_free(dht11calibrate->gpiodline_settings);
	gpiod_line_config_free(dht11calibrate->gpiodline_config);
	gpiod_request_config_free(dht11calibrate->gpiodreq_config);
	gpiod_chip_close(dht11calibrate->gpiodchip);






	struct input_requests* dht11data = request_in(datalines, size, CHIP0, "DHTdata");
	printf("\nWaiting for handshake LOW and HIGH");
	for (int i = 0; i < 2; i++) {

		response = gpiod_line_request_wait_edge_events(
				dht11data->gpiodline, 
				40000);
		read = gpiod_line_request_read_edge_events(
				dht11data->gpiodline, 
				handshakebuffer, 
				1);
		event = gpiod_edge_event_buffer_get_event(handshakebuffer, 0);
		printf("\nEvent %d: %s",
				i + 1,
				line_edge(gpiod_edge_event_get_event_type(event)));
	}





	printf("\nDecoding Phase");
	uint64_t bits[40] = {0};
	int index = 0;
	uint64_t pulse_time;
	for (int z = 0; z < 80; z++) {
		response = gpiod_line_request_wait_edge_events(dht11data->gpiodline, 80000);
		read = gpiod_line_request_read_edge_events(dht11data->gpiodline, buffer, 1);
		event = gpiod_edge_event_buffer_get_event(buffer, 0);
		event_type = gpiod_edge_event_get_event_type(event);
		start_time = gpiod_edge_event_get_timestamp_ns(event);

		printf("\nDecoded edge is %s", line_edge(event_type));

		if (event_type == GPIOD_EDGE_EVENT_RISING_EDGE) 
		{
			current_time = start_time;
		} 
		else if (event_type == GPIOD_EDGE_EVENT_FALLING_EDGE) 
		{
			if (current_time == 0) {
				printf("\nFALLING with no preceding RISING; skipping.");
				continue;
			}
			pulse_time = start_time - current_time;
			if (index < 40) {
				printf("\nBit %d pulse time: %llu ns",
						index, 
						pulse_time);
				bits[index] = pulse_time;
				index++;
			}
			current_time = 0;
		} 
		else {
			printf("\nIssue with event_type: %s", 
					line_edge(event_type));
			goto clean;
		}
	}





	uint8_t sensordata[5] = {0, 0, 0, 0, 0};
	uint64_t threshold = THRESHOLD;
	for (int t = 0; t < 40; t++) {
		int result = (bits[t] > threshold) ? 1 : 0;
		sensordata[t / 8] = (sensordata[t / 8] << 1) | result;
	}
	printf("\nDecoded Data:");
	printf("\nHumidity: %d.%d%%", sensordata[0], sensordata[1]);
	printf("\nTemperature: %d.%d°C", sensordata[2], sensordata[3]);
	uint8_t checksum = sensordata[0] + sensordata[1] + sensordata[2] + sensordata[3];
	if (checksum == sensordata[4])
		printf("\nChecksum valid: %d", sensordata[4]);
	else
		printf("\nChecksum error! Expected %d, but got %d", checksum, sensordata[4]);

	printf("\nProcess Finished");






clean:
	printf("\nCleaning...");
	gpiod_line_settings_free(dht11data->gpiodline_settings);
	gpiod_line_config_free(dht11data->gpiodline_config);
	gpiod_request_config_free(dht11data->gpiodreq_config);
	gpiod_line_request_release(dht11data->gpiodline);
	gpiod_chip_close(dht11data->gpiodchip);


	gpiod_line_request_set_value(dht11power->gpiodline, powerlines[0], GPIOD_LINE_VALUE_INACTIVE);
	gpiod_line_settings_free(dht11power->gpiodline_settings);
	gpiod_line_config_free(dht11power->gpiodline_config);
	gpiod_request_config_free(dht11power->gpiodreq_config);
	gpiod_line_request_release(dht11power->gpiodline);
	gpiod_chip_close(dht11power->gpiodchip);

	gpiod_edge_event_buffer_free(handshakebuffer);
	gpiod_edge_event_buffer_free(buffer);

	printf("\nClean Finish\n");
	return EXIT_SUCCESS;
}

