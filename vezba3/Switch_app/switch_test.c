#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main ()
{
	FILE *fp;
	char *str;
	char sval1,sval2,sval3,sval4;
	size_t num_of_bytes = 6;

	while(1)
	{

		//Citanje vrednosti tastera
		fp = fopen ("/dev/switch", "r");
		if(fp==NULL)
		{
			puts("Problem pri otvaranju /dev/switch");
			return -1;
		}


		str = (char *)malloc(num_of_bytes+1); 
		getline(&str, &num_of_bytes, fp); 

		if(fclose(fp))
		{
			puts("Problem pri zatvaranju /dev/switch");
			return -1;
		}


		sval1 = str[2] - 48;
		sval2 = str[3] - 48;
		sval3 = str[4] - 48;
		sval4 = str[5] - 48;
		free(str);

		printf("Vrednosti prekidaca: %d %d %d %d \n",sval1,sval2,sval3,sval4);
		sleep(1);

	}
}
