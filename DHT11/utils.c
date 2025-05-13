#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>
#include <stdbool.h>

#include "utils.h"





struct output_requests *request_out(int *offsets, size_t size_offsets, const char* gpiodfd, const char *consumer)
{
	struct gpiod_chip *chip = gpiod_chip_open(gpiodfd);
	if (!chip) 
	{
		printf("\nIssue with Chip");
		exit(EXIT_FAILURE);
	}

	struct gpiod_request_config *req_config = gpiod_request_config_new();
	if (!req_config)
	{
		printf("\nIssue with req_config");
		goto clean;
	}
	gpiod_request_config_set_consumer(req_config, consumer);

	struct gpiod_line_config *line_config = gpiod_line_config_new();
	if(!line_config)
	{
		printf("\nIssue with line_config");
		goto clean;
	}
	
	struct gpiod_line_settings *line_settings = gpiod_line_settings_new();
	if(!line_settings)
	{
		printf("\nIssue with line_settings");
		goto clean;
	}
	
	int setValue = gpiod_line_settings_set_output_value(line_settings, GPIOD_LINE_VALUE_ACTIVE);
	if(setValue < 0)
	{
		printf("\nIssue with setValue");
		goto clean;
	}

	int setDirection = gpiod_line_settings_set_direction(line_settings, GPIOD_LINE_DIRECTION_OUTPUT);
	if(setDirection < 0)
	{
		printf("\nIssue with setDirection");
		goto clean;
	}

	int add_config_offsets = gpiod_line_config_add_line_settings(line_config, offsets, size_offsets, line_settings);
	if(add_config_offsets < 0)
	{
		printf("\nIssue with add_config_offsets");
		goto clean;
	}
	
	struct gpiod_line_request* line = gpiod_chip_request_lines(chip, req_config, line_config);
	if(!line)
	{
		printf("\nIssue with line");
		goto clean;
	}

	struct output_requests *results = malloc(sizeof(struct output_requests));
        if (!results)
        {
                printf("\nIssue allocating results");
                goto clean;
        } 
	
	results-> gpiodline = line;
	results-> gpiodline_settings = line_settings;
	results-> gpiodline_config = line_config;
	results-> gpiodreq_config = req_config;
	results-> gpiodchip = chip;
	return results;

clean:
	if (line_settings)
		gpiod_line_settings_free(line_settings);
	if (line_config)
		gpiod_line_config_free(line_config);
	if (req_config)
		gpiod_request_config_free(req_config);
	if (line)
		gpiod_line_request_release(line);
	if (chip)
		gpiod_chip_close(chip);
	exit(EXIT_FAILURE);

}

struct input_requests *request_in(int *offsets, size_t size_offsets, const char* gpiodfd, const char *consumer)
{
	struct gpiod_chip *chip = gpiod_chip_open(gpiodfd);
        if (!chip) 
        {
                printf("\nIssue with chip");
                exit(EXIT_FAILURE);
        }
 
        struct gpiod_request_config *req_config = gpiod_request_config_new();
        if (!req_config)
        {
                printf("\nIssue with req_config");
                goto clean;
        }
        gpiod_request_config_set_consumer(req_config, consumer);

        struct gpiod_line_config *line_config = gpiod_line_config_new();
        if(!line_config)
        {
                printf("\nIssue with line_config");
                goto clean;
        }
        
        struct gpiod_line_settings *line_settings = gpiod_line_settings_new();
        if(!line_settings)
        {
                printf("\nIssue with line_settings");
                goto clean;
        }
	//gpiod_line_settings_set_active_low(line_settings, true);

        int setDirection = gpiod_line_settings_set_direction(line_settings, GPIOD_LINE_DIRECTION_INPUT);
        if(setDirection < 0)
        {
                printf("\nIssue with setDirection");
                goto clean;
        }

	int setEdgedetection = gpiod_line_settings_set_edge_detection(line_settings,GPIOD_LINE_EDGE_BOTH);
	if(setEdgedetection < 0)
	{
		printf("\nIssue with setEdgedetection");
		goto clean;
	}
	
	// lgpiod bias works with the internal pull-up or pull-down resistors of RASPI5. However this is insufficient to work with DHT11 and they recomment an external 5K pullup.
	/*
	int setBias = gpiod_line_settings_set_bias(line_settings, GPIOD_LINE_BIAS_PULL_UP);
	if(setBias <0)
	{
		printf("\nIssue with setBias");
		goto clean;
	}
	*/
	//ignore setBias shorter than debounce period below.
	//gpiod_line_settings_set_debounce_period_us();

        int add_config_offsets = gpiod_line_config_add_line_settings(line_config, offsets, size_offsets, line_settings);
        if(add_config_offsets < 0)
        {
                printf("\nIssue with add_config_offsets");
                goto clean;
        }

        struct gpiod_line_request* line = gpiod_chip_request_lines(chip, req_config, line_config);
        if(!line)
        {
                printf("\nIssue with Lines");
                goto clean;
        }
        
	struct input_requests *results = malloc(sizeof(struct input_requests));
	if(!results)
	{
		printf("\nIssue with allocating results");
		goto clean;
	}

	results-> gpiodchip = chip;
        results-> gpiodline_settings = line_settings;
        results-> gpiodline = line;
        results-> gpiodline_config = line_config;
        results-> gpiodreq_config = req_config;

	return results;

clean:
        if (line_settings)
                gpiod_line_settings_free(line_settings);
        if (line_config)
                gpiod_line_config_free(line_config);
        if (req_config)
                gpiod_request_config_free(req_config);
        if (line)
                gpiod_line_request_release(line);
        if (chip)
                gpiod_chip_close(chip);
	exit(EXIT_FAILURE);
}


void snapshot(struct gpiod_chip *chip, unsigned int offset, struct gpiod_line_settings *linesettings)
{
        struct gpiod_chip_info *gpiodchip_info = gpiod_chip_get_info(chip);
        if(!gpiodchip_info)
        {       
                printf("\nIssue with gpiodchip_info");
		goto clean;
	}

	struct gpiod_line_info *lineinfo = gpiod_chip_get_line_info(chip, offset);
	if(!lineinfo)
	{
		printf("\nIssue with line_info");
		goto clean;
	}
	
	const char *gpiodconsumer_info = gpiod_line_info_get_consumer(lineinfo);
	if(!gpiodconsumer_info)
	{
		printf("\nIssue with gpiod_consumer");
		goto clean;
	}
	printf("\n***START SNAPSHOT CONSUMER: %s****", gpiodconsumer_info);

	const char *gpiodline_info = gpiod_line_info_get_name(lineinfo);
	if(!gpiodline_info)
	{
		printf("\nIssue with gpiodline_info");
		goto clean;
	}
	printf("\nLine info: %s", gpiodline_info);
	
	bool gpiodis_used = gpiod_line_info_is_used(lineinfo);
	if(gpiodis_used == true)
	{
		printf("\nThis line is being used");
	}
	else
	{
		printf("\nThis line is not being used");
	}

	enum gpiod_line_direction gpiodinfo_direction = gpiod_line_info_get_direction(lineinfo);
	if(gpiodinfo_direction == GPIOD_LINE_DIRECTION_INPUT)
	{
		printf("\nLine information configured as input");
	}
	else if(gpiodinfo_direction == GPIOD_LINE_DIRECTION_OUTPUT)
	{
		printf("\nLine information configured as output");
	}
	else
	{
		printf("\nIssue with gpiodline_direction");
		goto clean;
	}

	enum gpiod_line_direction gpiodsetting_direction = gpiod_line_settings_get_direction(linesettings);
	if(gpiodinfo_direction == GPIOD_LINE_DIRECTION_INPUT)
        {
                printf("\nLine setting configured as input");
        }
        else if(gpiodinfo_direction == GPIOD_LINE_DIRECTION_OUTPUT)
        {
                printf("\nLine setting configured as output");
        }
	else if(gpiodsetting_direction == GPIOD_LINE_DIRECTION_AS_IS)
	{
		printf("\nLine setting configures as AS IS");
	}
        else
        {
                printf("\nIssue with gpiodsetting_direction");
                goto clean;
        }

	
	bool gpiodactive_low = gpiod_line_info_is_active_low(lineinfo);
	if(gpiodactive_low)
	{
		printf("\nThis line has active-low logic as true");
	}
	else
	{
		printf("\nThis line has active-low logic as false");
	}
	printf("\n***END SNAPSHOT CONSUMER: %s***", gpiodconsumer_info);
clean:
	if(gpiodchip_info)
		gpiod_chip_info_free(gpiodchip_info);
}


uint64_t elapsed(struct timespec timestamp)
{
	struct timespec current;
	clock_gettime(CLOCK_MONOTONIC, &current);
	return ((current.tv_sec - timestamp.tv_sec)) + (current.tv_nsec - timestamp.tv_nsec);
}
