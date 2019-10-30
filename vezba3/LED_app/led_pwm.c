#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	FILE* fp;
	float percentage;
	long int period = 20000L;
	if(argc == 2)
	{
		sscanf(argv[1],"%f",&percentage);
	}
	else
	{
		printf("Pogresan broj argumenata\n");
		return -1;
	}

	while(1) 
	{
		// Upali diode
		fp = fopen("/dev/led", "w");
		if(fp == NULL)
		{
			printf("Problem pri otvaranju /dev/led\n");
			return -1;
		}
		fputs("0x0F\n", fp);
		if(fclose(fp))
		{
			printf("Problem pri zatvaranju /dev/led\n");
			return -1;
		}
		usleep(percentage*period);

		// Ugasi diode
		fp = fopen("/dev/led", "w");
		if(fp == NULL)
		{
			printf("Problem pri otvaranju /dev/led\n");
			return -1;
		}
		fputs("0x00\n", fp);
		if(fclose(fp))
		{
			printf("Problem pri zatvaranju /dev/led\n");
			return -1;
		}
		usleep((1-percentage)*period);
	}
	return 0;
}

