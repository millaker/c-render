#include "display.h"
#include "fenster.h"

#include <stdio.h>
#include <stdlib.h>

static struct fenster f = {.title = "Canvas",
                           .height = CANVAS_HEIGHT,
                           .width = CANVAS_WIDTH,
                           .buf = NULL};

int display_init() {
  if (!f.buf) {
    f.buf = malloc(sizeof(uint32_t) * f.height * f.width);
  }
  if (!f.buf) {
    printf("Display init failed\n");
    return -1;
  }
  for (int i = 0; i < f.height * f.width; i++) {
    f.buf[i] = 0xffffff;
  }
  return 0;
}

void display_put_pixel(int x, int y, uint32_t color) {
  if (x >= CANVAS_WIDTH || y >= CANVAS_HEIGHT || x < 0 || y < 0)
    return;
  fenster_pixel(&f, x, y) = color;
}

/*
    Show canvas unitl window closed
*/
void display_show() {
  fenster_open(&f);
  int count = 0;
  while (fenster_loop(&f) == 0) {
  }
  fenster_close(&f);
}

void display_close() { free(f.buf); }

void output_image(FILE *file) {
  fprintf(file, "P3\n");
  fprintf(file, "%d %d\n255\n", f.width, f.height);
  for (int i = 0; i < f.width * f.height; i++) {
    fprintf(file, "%d %d %d\n", f.buf[i] >> 16, (f.buf[i] >> 8) & 0xff,
            (f.buf[i] & 0xff));
  }
}