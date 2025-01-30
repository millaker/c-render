#include "display.h"
#include "render.h"
#include "types.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

static vec3 cube_vertices[8] = {
    {1, 1, 1},  {-1, 1, 1},  {-1, -1, 1},  {1, -1, 1},
    {1, 1, -1}, {-1, 1, -1}, {-1, -1, -1}, {1, -1, -1},
};

static vec4 cube_triangles[12] = {
    {0, 1, 2, RED},    {0, 2, 3, RED},    {4, 0, 3, GREEN},  {4, 3, 7, GREEN},
    {5, 4, 7, BLUE},   {5, 7, 6, BLUE},   {1, 5, 6, YELLOW}, {1, 6, 2, YELLOW},
    {4, 5, 1, PURPLE}, {4, 1, 0, PURPLE}, {2, 6, 7, CYAN},   {2, 7, 3, CYAN},
};

int main() {
  display_init();
  /* Construct cube model */
  model_t *m = malloc(sizeof(model_t));
  m->t = cvec_vec4_alloc(12);
  m->v = cvec_vec3_alloc(8);
  for (int i = 0; i < 12; i++) {
    cvec_vec4_push(m->t, cube_triangles[i]);
  }
  for (int i = 0; i < 8; i++) {
    cvec_vec3_push(m->v, cube_vertices[i]);
  }

  /* Construct instances */
  instance_t cubes[1] = {{m, {0.5, {0, 0, 0}, {-1.5, 0, 7}}}};
  scene_t *s =
      construct_scene(cubes, 1, (transform_t){0, {0, 0, 0}, {0, 0, 0}});
  render(s);
  cvec_vec4_free(m->t);
  cvec_vec3_free(m->v);
  free(m);

  draw_triangle((vec2){0, 0}, (vec2){60, 180}, (vec2){90, 100}, 0);
  // draw_filled_triangle((vec2){0, 0}, (vec2){60, 180}, (vec2){90, 100}, 0);
  display_show();
  display_close();
  printf("Ended successfully\n");
  return 0;
}