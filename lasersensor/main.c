#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"

#define CHIP "/dev/gpiochip0"


int main(void)
{
	int pwrline[] = {17};
	int doutline[] = {27};
	struct timespec pause;

	struct output_requests *pwr = request_out(
			pwrline,
			1,
			CHIP,
			"PWR");

	struct input_requests *dout = request_in(
			doutline,
			1,
			CHIP,
			"DOUT");

	gpiod_line_request_set_value(
			pwr->gpiodline,
			pwrline[0],
			GPIOD_LINE_VALUE_ACTIVE);
	printf("\nPWR toggled HIGH");
	snapshot(pwr-> gpiodchip, pwrline[0], pwr-> gpiodline_settings);	

	int user = 0;
	pause.tv_sec = 0;
	pause.tv_nsec = 100000000;
	while(user < 100)
	{
		printf("\nDetecting");
		enum gpiod_line_value call = gpiod_line_request_get_value(
				dout-> gpiodline,
				doutline[0]);
		if (call == GPIOD_LINE_VALUE_ACTIVE)
		{
			printf("\nHits: %d", user);
			user++;
		}
		else if (call == GPIOD_LINE_VALUE_INACTIVE)
		{
			printf("\nNO HITS: %d", user);
		}
		fflush(stdout);
		nanosleep(&pause, NULL);
	}

	printf("\nPowering Off");
	gpiod_line_request_set_value(
			pwr->gpiodline,
			pwrline[0],
			GPIOD_LINE_VALUE_INACTIVE);

clean:
	if(pwr)
	{
		gpiod_line_settings_free(pwr-> gpiodline_settings);
		gpiod_line_config_free(pwr-> gpiodline_config);
		gpiod_request_config_free(pwr-> gpiodreq_config);
		gpiod_line_request_release(pwr-> gpiodline);
		gpiod_chip_close(pwr-> gpiodchip);
	}
	if (dout)
	{
		gpiod_line_settings_free(dout-> gpiodline_settings);
		gpiod_line_config_free(dout-> gpiodline_config);
		gpiod_request_config_free(dout-> gpiodreq_config);
		gpiod_line_request_release(dout-> gpiodline);
		gpiod_chip_close(dout-> gpiodchip);
	}
	return 0;
}

