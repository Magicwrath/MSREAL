#include "draw.h"

void drawSquare(unsigned int *display, int pos_x, int pos_y, int length, unsigned int rgb) {
  //Parameter checking
  if((pos_x + length) > 639) {
    printf("Invalid x position or length, the square exceeds the x range of the display!\n");
    //return 0;
  }

  if((pos_y + length) > 479) {
    printf("Invalid y position or lenght, the square exceeds the y range of the display!\n");
    //return 0;
  }

  //Draw the horizontal edges of the square
  int i = pos_x;
  for(i; i < (pos_x + length); i++) {
    display[640 * pos_y + i] = rgb; //color the dot in the desired color
    display[640 * (pos_y + length - 1) + i] = rgb;
  }

  //Draw the vertical edges of the square
  i = pos_y;
  for(i; i < (pos_y + length); i++) {
    display[640 * i + pos_x] = rgb;
    display[640 * i + (pos_x + length - 1)] = rgb;
  }

}

void drawCross(unsigned int *display, int pos_x, int pos_y, int distance, unsigned int rgb) {
  //Optional parameter checking, will be added after testing

  //This method of drawing is used for the simplicity
  //The cross can be drawn like the y=-x function from the starting point down

  //Draw the edges
  int i = 0;
  for(i; i < distance; i++) {
    display[640 * (pos_y + i) + pos_x + i] = rgb;
    display[640 * (pos_y + i) + pos_x + distance - 1 - i] = rgb;
  }
}

void drawTriangle(unsigned int *display, int pos_x, int pos_y, int height, unsigned int rgb) {
  //Optional parameter checking, will be added after testing

  //Draw the edges
  int i = 0;
  for(i; i < height; i++) {
    //Left edge
    display[640 * (pos_y + i) + pos_x - i] = rgb;
    //Right edge
    display[640 * (pos_y + i) + pos_x + i] = rgb;
    //Bottom edge, left part
    display[640 * (pos_y + height - 1) + pos_x - i] = rgb;
    //Bottom edge, right part
    display[640 * (pos_y + height - 1) + pos_x + i] = rgb;
  }
}

void drawCircle(unsigned int *display, int pos_x, int pos_y, int r, unsigned int rgb) {
  float y_val;
  int prev_val_pos;
  int prev_val_neg;
  float common_op;
  int quantized_y;
  int x = pos_x - r;
  int i;

  for(x; x <= pos_x + r; x++) {
    common_op = sqrt(r*r - (x - pos_x)*(x - pos_x));
    y_val = common_op  + pos_y;
    //printf("Calculated for x = %d, y_val = %f\n", x, y_val);
    quantized_y = (int) y_val;
    //printf("Quantized y_val is : %d\n\n", quantized_y);
    display[640 * quantized_y + x] = rgb;
    if(x == (pos_x - r))
      prev_val_pos = quantized_y;
    //printf("prev_val_pos : %d\n", prev_val_pos);
    //printf("quantized_y : %d\n", quantized_y);

    //draws a verical line from the previous y point to the new point
    if(prev_val_pos < quantized_y) {
      for(i = prev_val_pos; i < quantized_y; i++)
	display[640 * i + x] = rgb;
    } else {
      for(i = quantized_y; i < prev_val_pos; i++)
	display[640 * i + x] = rgb;
    }
    prev_val_pos = quantized_y;

    //the other solutions for the negative value of the square root
    y_val = pos_y - common_op;
    quantized_y = (int) y_val;
    if(x == pos_x - r)
      prev_val_neg = quantized_y;
    //printf("prev_val_neg : %d\n", prev_val_neg);
    //printf("quantized_y : %d\n", quantized_y);

    //same function as for the positive solutions
    if(prev_val_neg < quantized_y) {
      for(i = prev_val_neg; i < quantized_y;i++)
	display[640 * i + x] = rgb;
    } else {
      for(i = quantized_y; i < prev_val_neg ; i++)
	display[640 * i + x] = rgb;
    }
    prev_val_neg = quantized_y;
    display[640 * quantized_y + x] = rgb;
  }
}

void clearScreen(unsigned int *display) {
  int max_x = 639;
  int max_y = 479;

  int i = 0;
  for(i; i <= 640 * (max_y) + max_x; i++)
    display[i] = 0;
}
