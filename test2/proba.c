#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFF_SIZE 20

int main(void) {
  char buff[BUFF_SIZE] = "AB,C0,C3\0";
  char *comma;
  char parse_buff[BUFF_SIZE];
  long value;

  comma = strchr(buff, ','); //pokazivac na ,
  *comma = '\0';

  value = strtol(buff, NULL, 16);
  printf("Prva vrednost je: %ld\n", value);

  //comma = strchr(buff, ',');
  //comma = '\0';
  strncpy(parse_buff, buff+3, 3);
  //parse_buff[3] = '\0';

  value = strtol(parse_buff, NULL, 16);
  printf("Prva vrednost je: %ld\n", value);
  return 0;
}
