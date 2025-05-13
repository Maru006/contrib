#pragma once
#include <gpiod.h>


struct output_requests
{
        struct gpiod_chip *gpiodchip;
        struct gpiod_line_settings *gpiodline_settings;
        struct gpiod_line_request *gpiodline;
        struct gpiod_line_config *gpiodline_config;
        struct gpiod_request_config *gpiodreq_config;
};

struct input_requests
{
        struct gpiod_chip *gpiodchip;
        struct gpiod_line_settings *gpiodline_settings;
        struct gpiod_line_request *gpiodline;
        struct gpiod_line_config *gpiodline_config;
        struct gpiod_request_config *gpiodreq_config;

};

struct output_requests* request_out(int* offset, size_t size_offsets, const char* gpiodfd, const char* consumer);

struct input_requests* request_in(int *offsets, size_t size_offsets, const char *gpiodfd, const char *consumer);

unsigned long time_stamp();

void snapshot(struct gpiod_chip *chip, unsigned int offset, struct gpiod_line_settings *linesettings);

uint64_t elapsed(struct timespec timestamp);

