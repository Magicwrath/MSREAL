#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  FILE* fp;
  int led, switches = 0;
  char *str;
  char *leds;
  char en, add_or_sub;
  size_t num_of_bytes = 6;

  leds = (char *)malloc(3);

  while(1) {
    fp = fopen("/dev/switch", "r");
    if (fp == NULL) {
      puts("Problem pri otvaranju /dev/switch\n");
      return -1;
    }

    str = (char *)malloc(num_of_bytes+1);
    getline(&str, &num_of_bytes, fp);

    if(fclose(fp)) {
      puts("Problem sa zatvaranjem /dev/switch\n");
      return -1;
    }

    en = str[5] - 48;
    switches = 4 * (str[2] - 48) + 2 * (str[3] - 48) + (str[4] - 48);

    if (en) {
      led += switches;
      if (led > 15)
	led = 15;
    } else {
      led = 0;
    } //kraj if-else za en

    sprintf(leds, "%d\n", led);
    puts(leds);

    fp = fopen("/dev/led", "w");
    if (fp == NULL) {
      puts("Problem pri otvaranju /dev/led\n");
      return -1;
    }
    
    fputs(leds, fp);
    fclose(fp);
    sleep(2);
    
  }  //kraj while(1)

  return 0;
} //kraj main


