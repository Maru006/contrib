#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include "utils.h"

int write2reg(int fd, uint8_t reg, uint8_t data)
{
	printf("\nWritting at fd: %d", fd);
	uint8_t buffer[2] = {reg, data};
	int commit = write(fd, buffer, 2);
	if (commit != 2)
	{
		perror("\nIssue with commit as: ");
		return -1;
	}
	return 1;
}

int setangle(int fd, uint8_t channel, int data, uint8_t prescale)
{
	struct timespec pause;	

	printf("\nPutting Mode1 to 0x10");
	int set_mode0 = write2reg(fd, MODE1, 0x10);

	printf("\nAt set_prescale");
	int set_prescale = write2reg(fd, PRESCALE, prescale);

	printf("\n(STARTING)Setting On at channel: %d", channel);
	write2reg(fd, channel, 0x00);
	write2reg(fd, channel +1, 0x00);

	printf("\n(STOPPING)Setting Off at channel: %d", channel);
	write2reg(fd, channel +2, data & 0xFF);
	write2reg(fd, channel +3, (data >> 8) & 0xFF);

	pause.tv_sec = 0;
	pause.tv_nsec = 5000000;
	nanosleep(&pause, NULL);

	printf("Waking Up Mode1 with 0x00");
	write2reg(fd, MODE1, 0x00);

}


/*
   int readreg(int fd, uint8_t reg)
   {
   printf("\nReading from %u", reg);
   uint8_t	data[1] = {reg};
   int request = read(fd, reg, 1);
   {
   if (request != 1)
   {
   perror("\nIssue with request as:");
   return -1;
   }
   }
   }
   */
