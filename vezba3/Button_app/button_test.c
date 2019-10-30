#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main ()
{
	FILE *fp;
	char *str;
	char tval1,tval2,tval3,tval4;
	size_t num_of_bytes = 6;

	while(1)
	{

		//Citanje vrednosti tastera
		fp = fopen ("/dev/button", "r");
		if(fp==NULL)
		{
			puts("Problem pri otvaranju /dev/button");
			return -1;
		}


		str = (char *)malloc(num_of_bytes+1); 
		getline(&str, &num_of_bytes, fp); 

		if(fclose(fp))
		{
			puts("Problem pri zatvaranju /dev/button");
			return -1;
		}


		tval1 = str[2] - 48;
		tval2 = str[3] - 48;
		tval3 = str[4] - 48;
		tval4 = str[5] - 48;
		free(str);

		printf("Vrednosti tastera: %d %d %d %d \n",tval1,tval2,tval3,tval4);
		sleep(1);

	}
}
