#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdio.h>

#define CANVAS_HEIGHT 640
#define CANVAS_WIDTH 640

int display_init();
void display_put_pixel(int x, int y, uint32_t color);
void display_show();
void display_close();
void output_image(FILE *f);

#endif