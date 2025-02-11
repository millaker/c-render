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

static uint16_t font5x3[] = {
    0x0000, 0x2092, 0x002d, 0x5f7d, 0x279e, 0x52a5, 0x7ad6, 0x0012, 0x4494,
    0x1491, 0x017a, 0x05d0, 0x1400, 0x01c0, 0x0400, 0x12a4, 0x2b6a, 0x749a,
    0x752a, 0x38a3, 0x4f4a, 0x38cf, 0x3bce, 0x12a7, 0x3aae, 0x49ae, 0x0410,
    0x1410, 0x4454, 0x0e38, 0x1511, 0x10e3, 0x73ee, 0x5f7a, 0x3beb, 0x624e,
    0x3b6b, 0x73cf, 0x13cf, 0x6b4e, 0x5bed, 0x7497, 0x2b27, 0x5add, 0x7249,
    0x5b7d, 0x5b6b, 0x3b6e, 0x12eb, 0x4f6b, 0x5aeb, 0x388e, 0x2497, 0x6b6d,
    0x256d, 0x5f6d, 0x5aad, 0x24ad, 0x72a7, 0x6496, 0x4889, 0x3493, 0x002a,
    0xf000, 0x0011, 0x6b98, 0x3b79, 0x7270, 0x7b74, 0x6750, 0x95d6, 0xb9ee,
    0x5b59, 0x6410, 0xb482, 0x56e8, 0x6492, 0x5be8, 0x5b58, 0x3b70, 0x976a,
    0xcd6a, 0x1370, 0x38f0, 0x64ba, 0x3b68, 0x2568, 0x5f68, 0x54a8, 0xb9ad,
    0x73b8, 0x64d6, 0x2492, 0x3593, 0x03e0};
// clang-format on
static void fenster_text(struct fenster *f, int x, int y, char *s, int scale,
                         uint32_t c) {
  while (*s) {
    char chr = *s++;
    if (chr > 32) {
      uint16_t bmp = font5x3[chr - 32];
      for (int dy = 0; dy < 5; dy++) {
        for (int dx = 0; dx < 3; dx++) {
          if (bmp >> (dy * 3 + dx) & 1) {
            fenster_rect(f, x + dx * scale, y + dy * scale, scale, scale, c);
          }
        }
      }
    }
    x = x + 4 * scale;
  }
}

void render_key(struct fenster *f) {
  char c[32] = {0};
  char *p = c;
  for (int i = 0; i < 128; i++) {
    if (f->keys[i]) {
      *p++ = i;
    }
  }
  fenster_text(f, 8, 8, c, 16, 0xffffff);
}

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
  transform_t cam = {0};
  while (fenster_loop(&f) == 0) {

    /* Handle input */
    if (f.keys['W'])
      cam.tr.z += 0.1;
    if (f.keys['S'])
      cam.tr.z -= 0.1;
    if (f.keys['A'])
      cam.tr.x -= 0.1;
    if (f.keys['D'])
      cam.tr.x += 0.1;

    /* Esc */
    if (f.keys[27])
      break;

    /* Scene rendering */
    scene_t *s = construct_scene(inst, 1, cam, l, 2);
    s->pl = generate_fov90_planes();
    display_clear();
    render(s);

    int64_t time = fenster_time();
    // 60 fps
    if (time - now < 1000 / 60) {
      fenster_sleep(time - now);
    }
    now = time;

    /* Model Rotate */
    inst[0].tr.r.y += 0.5;

    /* Render key pressed */
    render_key(&f);
  }
  fenster_close(&f);
  free_model(m);
  free(tx);
  display_close();
}
