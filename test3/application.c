
#include "draw.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#define CLR_WHITE 0xffff
#define CLR_RED 0xf800
#define CLR_GREEN 0x07e0


void drawSymbol(int sel, int pos_x, int pos_y, unsigned int* display, unsigned int rgb);

int main(void) {
  int *p;
  int fd;
  char button_val[7]; //values read from the /dev/button driver
  int button_pressed = 0; //indicates whether a button is pressed or not
  int pos_x, pos_y; //coordinates used for drawing symbols during generation
  int game_over = 0; //indicates whether the player lost
  int game_begin = 0; //0 for the beggining of the game, after the symbols dissapear turns to 1
  int exit_game = 0; //variable used for indicating if the player wants to exit the game
  int gen_sequence[6]; //a variable to store the generated symbol sequence
  int i = 0; //counter for loops
  FILE *fbutton;
  unsigned int display[640*480];

  fd = open("/dev/vga_dma", O_RDWR | O_NDELAY);
  if(fd < 0) {
    printf("Can't open /dev/vga_dma!\n");
    return -1;
  }

  fbutton = fopen("/dev/button", "r");
  if(fbutton == NULL) {
    printf("Can't open /dev/button!\n");
    return -1;
  }

  //pointer to the virtual memory mapped to the driver memory space
  p = (int*) mmap(0, MAX_PKT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0);
  
  //Clear the screen
  clearScreen(display);
  
  //Main application
  while(!exit_game) {
    if(!game_begin) {
      //--------------------BEGGINING OF THE GAME-----------------------------------
      //set pseudo random number generator seed to current time
      srand(time(0));
      pos_x = 1;
      pos_y = 1;

      //symbol generator loop
      for(i = 0; i < 6; i++) {
        gen_sequence[i] = rand() % 4;
        drawSymbol(gen_sequence[i], pos_x, pos_y, display, CLR_WHITE);

        //offset the next symbol, if the generated symbol is a triangle, use a different offset
        if(gen_sequence[i] == 2) {
          pos_x += 90;
        } else {
          pos_x += 50;
        }
      }

      memcpy(p, display, MAX_PKT_SIZE);

      //sleep for two seconds, then clear the screen
      sleep(2);
      clearScreen(display);
      pos_x = 1; //reset the value
      i = 0; //reset the counter for the game
      memcpy(p, display, MAX_PKT_SIZE);
      game_begin = 1;
    } else {
      //----------------------PLAYING THE GAME--------------------------------------------

      fread(button_val, 1, 7, fbutton);
      button_val[6] = '\0';
      
      //software differentiation
      if(button_pressed == 0 && (strncmp("0b0000", button_val, 6) != 0)) {
        button_pressed = 1;

        //find out which button is pressed
        if(strncmp("0b0001", button_val, 6) == 0) {
          //BTN0 pressed
          if(gen_sequence[i] == 3) {
            drawSymbol(3, pos_x, pos_y, display, CLR_GREEN);
          }
          else {
            drawSymbol(3, pos_x, pos_y, display, CLR_RED);
            game_over = 1;
          }
          memcpy(p, display, MAX_PKT_SIZE);
          //offset next symbol
          pos_x += 50;
          i++;
        }
      

        if(strncmp("0b0010", button_val, 6) == 0) {
          //BTN1 pressed
          if(gen_sequence[i] == 2) {
            drawSymbol(2, pos_x, pos_y, display, CLR_GREEN);
          }
          else {
            drawSymbol(2, pos_x, pos_y, display, CLR_RED);
            game_over = 1;
          }
          memcpy(p, display, MAX_PKT_SIZE);
          //offset next symbol
          pos_x += 90;
          i++;
        }

        if(strncmp("0b0100", button_val, 6) == 0) {
          //BTN2 pressed
          if(gen_sequence[i] == 1) {
            drawSymbol(1, pos_x, pos_y, display, CLR_GREEN);
          }
          else {
            drawSymbol(1, pos_x, pos_y, display, CLR_RED);
            game_over = 1;
          }
          //offset next symbol
          memcpy(p, display, MAX_PKT_SIZE);
          pos_x += 50;
          i++;
        }

        if(strncmp("0b1000", button_val, 6) == 0) {
          //BTN3 pressed
          if(gen_sequence[i] == 0) {
            drawSymbol(0, pos_x, pos_y, display, CLR_GREEN);
          }
          else {
            drawSymbol(0, pos_x, pos_y, display, CLR_RED);
            game_over = 1;
          }
          //offset next symbol
          memcpy(p, display, MAX_PKT_SIZE);
          pos_x += 50;
          i++;
        }

      } //end of the first part of software diff.

      //check if all the buttons have been released
      if(strncmp("0b0000", button_val, 6) == 0)
        button_pressed = 0;

      //button combination for exiting the game
      if(strncmp("0b1100", button_val, 6) == 0)
        exit_game = 1;
	
      
      //end of user input
      if(i == 6) {
	//draw the head of the smiley
        drawCircle(display, 320, 240, 100, 0, CLR_RED | CLR_GREEN);
	
	//draw the eyes as concentric circles
	for(i = 1; i <= 15; i++) {
	  drawCircle(display, 270, 190, i, 0, CLR_RED | CLR_GREEN);
	  drawCircle(display, 370, 190, i, 0, CLR_RED | CLR_GREEN);
	}

	if(game_over) {
	  drawCircle(display, 320, 295, 50, -1, CLR_RED | CLR_GREEN);
	  game_over = 0;
	} else {
	  drawCircle(display, 320, 255, 50, 1, CLR_RED | CLR_GREEN);
	}
	
        memcpy(p, display, MAX_PKT_SIZE);
        sleep(5);
        clearScreen(display);
        memcpy(p, display, MAX_PKT_SIZE);
        game_begin = 0;
      }
      
    } //end of else part for playing the game, game_begin condition
  } //end of while(!exit_game)

  clearScreen(display);
  memcpy(p, display, MAX_PKT_SIZE);
  munmap(p, MAX_PKT_SIZE);
  fclose(fbutton);
  close(fd);

  return 0;
}


void drawSymbol(int sel, int pos_x, int pos_y, unsigned int* display, unsigned int rgb) {
  switch(sel) {
  case 0:  
    drawSquare(display, pos_x, pos_y, 40, rgb);
    break;
  case 1:
    drawCross(display, pos_x, pos_y, 40, rgb);
    break;
  case 2:
    drawTriangle(display, pos_x + 40, pos_y, 40, rgb);
    break;
  case 3:
    drawCircle(display, pos_x + 20, 21, 20, 0, rgb);
    break;
  default:
    printf("Wrong select value!\n");
    break;
  }
}
