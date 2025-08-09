#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "utils.h"

void flush()
{
	int c;
	while((c = getchar()) != EOF && c != '\n');
}

int main(void)
{
	int runtime = 0;
	int headbase = 1500;
	int tailbase = 1000;
	int move;
	int fd = open(I2C_DEV_PATH, O_RDWR);
	if (fd < 0)
	{
		perror("\nmain: Failed at runtime fd");
		return -1;
	}
	
	int enableTCA;
	printf("\nEnable TCA (Yes = 'y' | No = 'n'): ");
	if ((enableTCA = getchar()) != EOF && enableTCA == TOGGLE)
	{
		goto tca_channel;
	}
	else
	{
		goto pca_channel;
	}
	
tca_channel:
	int tca_channel, mask;
	printf("\nEnter TCA Channel: ");
	while ((tca_channel = getchar())!= EOF)
	{
		if (tca_channel == '\n')
		{
			continue;
		}
		if (tca_channel == TERMINATE)
		{
			goto clean;
		}
		else if (tca_channel > '0' || tca_channel < '7')
		{
			runtime = 0; //reset runtime to 0 for presets on each PCA
			mask = tca_channel - '0'; //convert string to int
			command2tca(fd, mask);
			printf("\nAt Channel: %d", mask);
			goto pca_channel;
		}
		else
		{
			if (tca_channel < '0' || tca_channel > '7')
			{
				printf("\nMut be between 0 and 7");
				continue;
			}
			else
			{
				perror("\ntca_channel: out of bounds");
				return -1;
			}
		}
	}

pca_channel:
	setangle(fd, &runtime, HEAD, headbase, PWM);
	setangle(fd, &runtime, TAIL, tailbase, PWM);	

	printf("\nPress WASD: ");
	while ((move = getchar()) != EOF) 
	{
		printf("\n");
		switch (move)
		{
			case '\n':
				continue;
			case TURNUP:
				headbase -= SENSITIVITY; 
				break;
			case TURNLEFT:
				tailbase += SENSITIVITY;
				break;
			case TURNDOWN:
				headbase += SENSITIVITY;
				break;
			case TURNRIGHT:
				tailbase -= SENSITIVITY;
				break;
			case RESET:
				headbase = 1500;
				tailbase = 1000;
				break;
			case CHANGECHANNEL:
				goto tca_channel;
			case TERMINATE:
				goto clean;
			default:
				printf("");
				continue;
		}
		setangle(fd, &runtime, HEAD, headbase, PWM);
		setangle(fd, &runtime, TAIL, tailbase, PWM);
		printf("\nRuntim: %d, Head: %d, Base: %d", runtime, headbase, tailbase);
	}

clean:
	printf("\nTerminating Program");
	if (fd)
	{
		close(fd);
		printf("\nProgram Terminated\n");

		return 0;
	}
}
