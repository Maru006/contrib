#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "utils.h"

int main(void)
{
	int move, head_target, headbase, tail_target, tailbase, runtime = 0;
	headbase = head_target = HEADDEFAULT;
	tailbase = tail_target = TAILDEFAULT;
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
	int tca_channel, mask, commit;
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
			commit = command2tca(fd, mask);
			if (commit < 0)
			{
				printf("\nFailed to detect TCA. Resolving to PCA Channel 1 and 2");
				goto pca_channel;
			}
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
				head_target = headbase - SENSITIVITY; 
				break;
			case TURNLEFT:
				tail_target = tailbase - SENSITIVITY;
				break;
			case TURNDOWN:
				head_target = headbase + SENSITIVITY;
				break;
			case TURNRIGHT:
				tail_target = tailbase + SENSITIVITY;
				break;
			case RESET:
				setangle(fd, &runtime, HEAD, headbase, PWM);
				setangle(fd, &runtime, TAIL, tailbase, PWM);
				continue;
			case CHANGECHANNEL:
				goto tca_channel;
			case TERMINATE:
				goto clean;
			default:
				printf("");
				continue;
		}
		setmove(&headbase, &head_target, fd, &runtime, HEAD, PWM);
		setmove(&tailbase, &tail_target, fd, &runtime, TAIL, PWM);
		printf("\nRuntime: %d, Head: %d, Base: %d", runtime, headbase, tailbase);
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
