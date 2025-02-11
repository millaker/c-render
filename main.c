#include "display.h"
#include "render.h"
#include "types.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo(char *mfile, char *tfile) {
  display_init();
  model_t *m = load_model(mfile);
  uint32_t *tx = NULL;
  vec2 dim = {0};
  if (tfile) {
    tx = load_texture(tfile, &dim);
  }
  light_t l[] = {{0, {0, 0, 0, 0.2}}, {1, {-1, 0, -1, 0.8}}};
  instance_t inst[] = {
      {m, {1.0, {0, 45, 0}, {1.0, 0.0, 5}}, tx, dim},
      {m, {1.0, {0, -45, 0}, {-0.5, 0.0, 7}}, tx, dim},
      {m, {1.0, {0, 35, 0}, {-1.5, 0.0, 5}}, tx, dim},
      {m, {1.0, {0, 25, 0}, {2.5, 0.0, 8}}, tx, dim},
      {m, {1.0, {0, 65, 0}, {1.5, 0.0, 15}}, tx, dim},
  };
  scene_t *s = construct_scene(inst, sizeof(inst) / sizeof(instance_t),
                               (transform_t){0, {0, 0, 0}, {0, 0, 0}}, l,
                               sizeof(l) / sizeof(l[0]));
  s->pl = generate_fov90_planes();
  render(s);
  display_show();
  display_close();
  free_model(m);
  free(tx);
  printf("Render model successfully\n");
}

void single(char *mfile, char *tfile) {
  display_init();
  model_t *m = load_model(mfile);
  uint32_t *tx = NULL;
  vec2 dim = {0};
  if (tfile) {
    tx = load_texture(tfile, &dim);
  }
  light_t l[] = {{0, {0, 0, 0, 0.2}}, {1, {-1, 0, -1, 0.8}}};
  instance_t inst[] = {
      {m, {1.0, {0, 45, 0}, {1.0, 0.0, 5}}, tx, dim},
  };
  scene_t *s = construct_scene(inst, sizeof(inst) / sizeof(instance_t),
                               (transform_t){0, {0, 0, 0}, {0, 0, 0}}, l,
                               sizeof(l) / sizeof(l[0]));
  s->pl = generate_fov90_planes();
  render(s);
  display_show();
  display_close();
  free_model(m);
  free(tx);
  printf("Render model successfully\n");
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("Specify model file or test\n");
    return -1;
  }
  if (strcmp(argv[1], "demo") == 0) {
    demo("models/spot.obj", "models/spot_texture.png");
  } else if (strcmp(argv[1], "anime") == 0) {
    anime();
  } else {
    single(argv[1], argc == 3 ? argv[2] : NULL);
  }
  return 0;
}
