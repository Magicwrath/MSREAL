#ifndef DRAW_H_INCLUDED
#define DRAW_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

//HEADER DESCRIPTION:
//Contains functions for drawing certain shapes on the display
//They do it by changing the color of the desired pixels in the input array

#define MAX_PKT_SIZE 640*480*4 //680x480 display, 4byte locations in RAM


//FUNCTION PROTOTYPES:

//Draws a square in the display array given the x and y coordinates of upper left point
//with the desired length, and the desired color
void drawSquare(unsigned int *display, int pos_x, int pos_y, int length, unsigned int rgb);

//Draws a cross in the display array given the x and y coordinate of the upper left and distance from that point
//with the desired color
void drawCross(unsigned int *display, int pos_x, int pos_y, int distance, unsigned int rgb);

//Draws a triangle in the dipslay array given the x and y coordinates of the upper point and the height
//with the desired color
void drawTriangle(unsigned int *display, int pos_x, int pos_y, int height, unsigned int rgb);

//Draws a circle in the display array given the center coordinates and half radius
//with the desired color
void drawCircle(unsigned int *display, int pos_x, int pos_y, int r,  unsigned int rgb);

//Clears the screen
void clearScreen(unsigned int *display);

#endif
