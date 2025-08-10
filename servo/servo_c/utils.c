#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <time.h>
#include "utils.h"


int command2tca(int fd, uint8_t channel)
{
	if (channel < 7)
	{
		if (ioctl(fd, I2C_SLAVE, MUX) < 0)
		{
			perror("\ncommand2tca: Failed to select mux");
			return -1;
		}
		else
		{
			printf("\ncommand2tca: SUCCESS");
			uint8_t mask = 1 << channel; 
			int commitchannel = write(fd, &mask, 1); 
			if (commitchannel != 1) // 1 as n = byte to be written
			{
				perror("\ncommand2tca: Failed commit channel");
				return -1;
			}
		}
	}
	else
	{
		printf("\nChannel must be between 0 and 7");
		return -1;
	}
	printf("\nSuccess: TCA(%d), channels: %hhu\n", fd, channel);
	return 0; // does not affect while(1). Only main()
}

int command2pca(int fd, uint8_t reg, uint8_t data)
{
	if (ioctl(fd, I2C_SLAVE, PCA) < 0)
	{
		perror("\ncommand2pca: Failed to select PCA");
		return -1;
	}
	else 
	{
		uint8_t buffer[2] = {reg, data};
		int commit = write(fd, buffer, 2);
		if (commit != 2)
		{
			perror("\ncommand2pca: Failed to commit command");
			return -1;
		}
	}
	return 0; // does not affect while(1). Only main()
}

int setangle(int fd, int *runtime, uint8_t channel, int data, uint8_t prescale)
{
	struct timespec pause;	

	// runetime != NULL checks if the pointer exists
	if (runtime != NULL && (*runtime) == 0)
	{
		printf("\nFirst runtime commands");
		printf("\nPutting Device to sleep: Mode1 to 0x10");
		int set_mode0 = command2pca(fd, MODE1, 0x10);

		if(set_mode0 < 0)
		{
			perror("\nsetangle: Issue Setting MODE 1 as: ");
			return -1;
			   int auto_increment = command2pca(fd, MODE1, 0xA1);
			   if (auto_increment < 0)
			   {
			   perror("\nsetangle: Issue Setting AUTOINCREMENT");
			   }
			int set_prescale = command2pca(fd, PRESCALE, prescale);
			if(set_prescale < 0)
			{
				perror("\nsetangle: Issue Setting PRESCALE as: ");
				return -1;	
			}
			command2pca(fd, channel, 0x00);
			command2pca(fd, channel +1, 0x00);
			printf("\nOn at channels: %d, %d", channel, channel+1);

			command2pca(fd,channel +2, data & 0xFF); //0xFF is 1111 1111
			command2pca(fd, channel +3, (data >> 8) & 0xFF);
			printf("\nOff at channels: %d, %d", channel+2, channel+3);

			pause.tv_sec = 0;
			pause.tv_nsec = 5000000;
			nanosleep(&pause, NULL);

			printf("\nWaking Up Device: Mode1 as 0x00");
			command2pca(fd, MODE1, 0x00);
		}
		(*runtime)++;
	}

	command2pca(fd, channel, 0x00);
	command2pca(fd, channel +1, 0x00);
	printf("\nOn at channels: %d, %d", channel, channel+1);

	command2pca(fd,channel +2, data & 0xFF); //0xFF is 1111 1111
	command2pca(fd, channel +3, (data >> 8) & 0xFF);
	printf("\nOff at channels: %d, %d", channel+2, channel+3);

	printf("\nWaking Up Device: Mode1 as 0x00");
	command2pca(fd, MODE1, 0x00);
	(*runtime)++;

	return 0;  // does not affect while(1). Only main()
}

int raw_command2pca(int fd, uint8_t reg, uint8_t data)
{
	uint8_t buffer[2] = {reg, data};
	int commit = write(fd, buffer, 2);
	if (commit != 2)
	{
		perror("\nraw_command2pca: Failed to commit command");
		return -1;
	}

	return 0; // does not affect while(1). Only main()
}

int raw_set_angle(int fd, int channel, int data, uint8_t prescale)
{
	struct timespec pause;
	pause.tv_sec = 0;
	pause.tv_nsec = 5000000;
	printf("\nSet ioctl raw set");
	if (ioctl(fd, I2C_SLAVE, PCA) < 0)
	{
		perror("\ncommand2pca: Failed to select PCA");
		return -1;
	}

	printf("\nSleep");
	int sleep = raw_command2pca(fd, MODE1, 0x10);
	if (sleep < 0)
	{
		printf("\nsleep issue");
		return -1;
	}
	printf("\nSetting Params");
	int set_prescale = raw_command2pca(fd, PRESCALE, prescale);
	if(set_prescale < 0)
	{
		perror("\nsetangle: Issue Setting PRESCALE as: ");
		return -1;	
	}
	raw_command2pca(fd, channel, 0x00);
	raw_command2pca(fd,  channel +1, 0x00);
	printf("\nOn at channels: %d, %d", channel, channel+1);

	raw_command2pca(fd, channel +2, data & 0xFF); //0xFF is 1111 1111
	raw_command2pca(fd, channel +3, (data >> 8) & 0xFF);
	printf("\nOff at channels: %d, %d", channel+2, channel+3);

	nanosleep(&pause, NULL);

	printf("\nWaking Up Device: Mode1 as 0x00");
	command2pca(fd, MODE1, 0x00);
	return 0;
}
