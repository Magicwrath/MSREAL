#include "draw.h"

void drawSquare(unsigned int *display, int pos_x, int pos_y, int length, unsigned int rgb) {
  //Parameter checking
  if((pos_x + length) > 639) {
    printf("Invalid x position or length, the square exceeds the x range of the display!\n");
    return 0;
  }

  if((pos_y + length) > 479) {
    printf("Invalid y position or lenght, the square exceeds the y range of the display!\n");
    return 0;
  }

  //Draw the horizontal edges of the square
  for(int i = pos_x; i < (pos_x + length); i++) {
    display[640 * pos_y + i] = rgb; //color the dot in the desired color
    display[640 * (pos_y + length - 1) + i] = rgb;
  }

  //Draw the vertical edges of the square
  for(int i = pos_y; y < (pos_y + length); i++) {
    display[640 * i + pos_x] = rgb;
    display[640 * i + (pos_x + length - 1)] = rgb;
  }

}

void drawCross(unsigned int *display, int pos_x, int pos_y, int distance, unsigned int rgb) {
  //Optional parameter checking, will be added after testing

  //This method of drawing is used for the simplicity
  //The cross can be drawn like the y=-x function from the starting point down

  //Draw the edges
  for(int i = 0; i < distance; i++) {
    display[640 * (pos_y + i) + pos_x + i] = rgb;
    display[640 * (pos_y + i) + pos_x + distance - 1 - i] = rgb;
  }
}

void drawTriangle(unsigned int *display, int pos_x, int pos_y, int height, unsigned int rgb) {
  //Optional parameter checking, will be added after testing

  //Draw the edges
  for(int i = 0; i < height; i++) {
    //Left edge
    display[640 * (y_pos + i) + pos_x - i] = rgb;
    //Right edge
    display[640 * (y_pos + i) + pos_x + i] = rgb;
    //Bottom edge, left part
    display[640 * (y_pos + height - 1) + pos_x - i] = rgb;
    //Bottom edge, right part
    display[640 * (y_pos + height - 1) + pos_x + i] = rgb;
  }
}

void drawCircle(unsigned int *display, int pos_x, int pos_y, int r, unsigned int rgb) {
  float y_val = 0;
  float common_op = 0;

  //Calculate the y values for the given x values with the analytic formula
  //y_val = sqrt(r^2 - (x - pos_x) ^ 2) + pos_y

  for(int x = -r; x <= r; x++) {
    //common operator in the positive and negative values
    common_op = sqrt(r^2 - (x - pos_x)^2);
    //positive values
    y_val = common_op + pos_y;
    display[640 * (int)y_val + pos_x + x] = rgb;

    //negative values
    y_val = pos_y - common_op;
    display[640 * (int)y_val + pos_x + x] = rgb;
  }
}

void clearScreen(unsigned int *display) {
  int max_x = 639;
  int max_y = 479;

  for(int i = 0; i <= 680 * (max_y) + max_x; i++)
    display[i] = 0;
}
