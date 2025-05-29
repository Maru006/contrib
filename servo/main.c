#include <stdio.h> 
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "utils.h"

#define I2C_DEV_PATH "/dev/i2c-1"
#define SENSITIVITY 100
#define PWM 30

#define MAXLEFT 400
#define MAXDOWN 400
#define MAXRIGHT 2200
#define MAXUP 2100

int main (void)
{
	int fd = open(I2C_DEV_PATH, O_RDWR);
	if (fd < 0)
	{
		perror("\nIssue at request: ");
		goto clean;
	}

	int slave = ioctl(fd, I2C_SLAVE, ADDR1);
	if (slave < 0)
	{
		perror("\n Issue with at slave");
		return -1;
	}

	int channel1 = 0x06;
	int channel2 = 0x0A;

	// Need to press ENTTER key after instruction key below. 
	// However as it is a getchar() func, typing multiple instructions key, the servo will compare each character and move corresponding to the series of characters that follow
	int up = 'w';
	int down = 's';
	int left = 'a';
	int right = 'd';
	int commit = '\n';
	int reset = 'r';

	int channel1base = 1500;
	int channel2base = 1400;
	setangle(fd, channel1, channel1base, PWM);
	setangle(fd, channel2, channel2base, PWM);
	int c;

	while(((c = getchar()) != 27))
	{
		if (c == reset)
		{
			setangle(fd, channel1, channel1base, PWM);
			setangle(fd, channel2, channel2base, PWM);
		}
		if (c == up)
		{
			channel1base+=SENSITIVITY;
			setangle(fd, channel1, channel1base, PWM);
		}
		else if(c == down)
		{
			channel1base-=SENSITIVITY;
			setangle(fd, channel1, channel1base, PWM);
		}
		else if(c == left)
		{
			channel2base-=SENSITIVITY;
			setangle(fd, channel2, channel2base, PWM);
		}       
		else if(c == right)
		{
			channel2base+=SENSITIVITY;
			setangle(fd, channel2, channel2base, PWM);
		} 
		else if (c == commit)
			printf("\nCommit");

		else if ((channel1base > MAXLEFT)  // Exceeded left limit
				|| (channel1base < MAXRIGHT) // Exceeded right limit
				|| (channel2base > MAXDOWN)  // Exceeded down limit
				|| (channel2base < MAXUP))   // Exceeded up limit
		{
			printf("\nExceeded Stats");
		}
		else
		{
			printf("\nUnknown Key. Use WASD");
		}
		printf("\n Current Stats: Channel1: %d, Channel 2: %d\n", 
				channel1base, 
				channel2base);
	}


clean:
	printf("\nTerminating Program");
	if (fd)
		close(fd);
	printf("\nProgram Terminated\n");

	return 0;
}











