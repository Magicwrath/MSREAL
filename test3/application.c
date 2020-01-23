#include "draw.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

int main(void) {
  int *p;
  int fd;
  FILE *fbutton;
  unsigned int display[680*480];

  fd = open("/dev/vga_dma", O_RDWR | O_NDELAY);
  if(fd < 0) {
    printf("Can't open /dev/vga_dma!\n");
    return -1;
  }

  //pointer to the virtual memory mapped to the driver memory space
  p = (int*) mmap(0, MAX_PKT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 0);

  clearScreen(display);

  drawSquare(display, 1, 1, 30, 0xffff);
  drawCross(display, 50, 1, 30, 0xffff);
  drawTriangle(display, 100, 1, 30, 0xffff);
  drawScreen(display, 150, 51, 50, 0xffff);

  memcpy(p, display, MAX_PKT_SIZE);
  munmap(p, MAX_PK_SIZE);
  close(fd);

  return 0;
}
