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
    f.buf[i] = 0;
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

void display_clear() {
  for (int i = 0; i < f.height; i++) {
    for (int j = 0; j < f.width; j++) {
      fenster_pixel(&f, j, i) = 0x0;
    }
  }
}

static void fenster_rect(struct fenster *f, int x, int y, int w, int h,
                         uint32_t c) {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      fenster_pixel(f, x + col, y + row) = c;
    }
  }
}

#include "render.h"
#include "types.h"
#include "util.h"

void anime() {
  display_init();
  fenster_open(&f);
  model_t *m = load_model("models/spot.obj");
  uint32_t *tx = NULL;
  vec2 dim = {0};
  tx = load_texture("models/spot_texture.png", &dim);
  int64_t now = fenster_time();
  light_t l[2] = {{0, {0, 0, 0, 0.2}}, {1, {-1, 0, -1, 0.8}}};
  instance_t inst[1] = {
      {m, {1.0, {0, 60, 0}, {1.0, 0.0, 5}}, tx, dim},
  };
  int count = 0, not_turned = 1;
  while (fenster_loop(&f) == 0) {
    scene_t *s =
        construct_scene(inst, 1, (transform_t){0, {0, 0, 0}, {0, 0, 0}}, l, 2);
    s->pl = generate_fov90_planes();
    display_clear();
    render(s);
    int64_t time = fenster_time();
    // 60 fps
    if (time - now < 1000 / 60) {
      fenster_sleep(time - now);
    }
    now = time;
    inst[0].tr.r.y += 0.5;
  }
  fenster_close(&f);
  free_model(m);
  free(tx);
  display_close();
}
