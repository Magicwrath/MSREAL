#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  FILE* fp;
  int led, switches = 0;
  char *str;
  char *leds;
  char en;
  char add_or_sub = 0;
  char key_pressed = 0;
  int btn_value = 0;
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
    free(str);

    //Citanje tastera za promenu operacije
    fp = fopen("/dev/button", "r");
    if (fp == NULL) {
      puts("Problem pri otvaranju fajla /dev/button\n");
      return -1;
    }

    str = (char *)malloc(num_of_bytes+1);
    getline(&str, &num_of_bytes, fp);
    btn_value = str[5] - 48; //Vrednost BTN0 tastera
    free(str);

    if (fclose(fp)) {
      puts("Problem sa zatvaranjem /dev/button\n");
      return -1;
    }

    //Softversko diferenciranje (Da li uopste treba?)
    if (!key_pressed && btn_value) {
      key_pressed = 1;
      add_or_sub = (add_or_sub + 1) % 2; //Variranje promenljive add_or_sub svakim pritiskom tastera
    }

    if (!btn_value)
      key_pressed = 0; 

    //Sabiranje / oduzimanje u zavisnosti od vrednosti sub_or add i provera za overflow
    if (en) {
      if (!add_or_sub) {
	led += switches;
	if (led > 15)
	  led = 15;
      } else {
	if (led < switches) {
	  led = 0;
	} else {
	  led -= switches; //dodato da ne bi char odlazio u negativne brojeve
	}
      }
    } 

    sprintf(leds, "%d\n", led); //Formiranje stringa koji ce se ispisati u fajl za diode
    
    //Ispis na diode
    fp = fopen("/dev/led", "w");
    if (fp == NULL) {
      puts("Problem pri otvaranju /dev/led\n");
      return -1;
    }
    
    fputs(leds, fp);
    if(fclose(fp)) {
      puts("Problem sa zatvaranjem /dev/led\n");
      return -1;
    }
    
    sleep(2);
    
  }  //kraj while(1)

  free(leds);

  return 0;
} //kraj main


