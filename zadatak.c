#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  FILE* fp;
  int led, switches = 0;
  char *str;
  char *leds[3];
  char en, add_or_sub;
  size_t num_of_bytes = 6;

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

    fp = fopen("/dev/led", "w");
    if (fp == NULL) {
      puts("Problem pri otvaranju /dev/led\n");
      return -1;
    }
    
    puts(leds, fp);
    sleep(2);
    
  }  //kraj while(1)
} //kraj main


