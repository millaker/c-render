#include "display.h"
#include "render.h"
#include "types.h"
#include "util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static vec3 cube_vertices[] = {
    {1, 1, 1},  {-1, 1, 1},  {-1, -1, 1},  {1, -1, 1},
    {1, 1, -1}, {-1, 1, -1}, {-1, -1, -1}, {1, -1, -1},
};

static vec4 cube_triangles[] = {
    {0, 1, 2, RED},    {0, 2, 3, RED},    {4, 0, 3, GREEN},  {4, 3, 7, GREEN},
    {5, 4, 7, BLUE},   {5, 7, 6, BLUE},   {1, 5, 6, YELLOW}, {1, 6, 2, YELLOW},
    {4, 5, 1, PURPLE}, {4, 1, 0, PURPLE}, {2, 6, 7, CYAN},   {2, 7, 3, CYAN},
};

static vec4 unicube_triangles[] = {
    {0, 1, 2, RED}, {0, 2, 3, RED}, {4, 0, 3, RED}, {4, 3, 7, RED},
    {5, 4, 7, RED}, {5, 7, 6, RED}, {1, 5, 6, RED}, {1, 6, 2, RED},
    {4, 5, 1, RED}, {4, 1, 0, RED}, {2, 6, 7, RED}, {2, 7, 3, RED},
};

int main(int argc, char *argv[]) {
  display_init();
  if (argc != 1) {
    model_t *m = load_model(argv[1]);
    uint32_t *tx = NULL;
    vec2 dim = {0};
    if (argc == 3) {
      tx = load_texture(argv[2], &dim);
    }
    light_t l[] = {{0, {0, 0, 0, 0.2}}, {1, {-1, 0, -1, 0.8}}};
    instance_t inst[] = {
        {m, {2.0, {-120, 0, 0}, {0.0, 0.0, 5}}, tx, dim},
    };
    scene_t *s = construct_scene(inst, sizeof(inst) / sizeof(instance_t),
                                 (transform_t){0, {0, 0, 0}, {0, 0, 0}}, l,
                                 sizeof(l) / sizeof(l[0]));
    /* temp plane list */
    cvec_vec4 *pl = cvec_vec4_alloc(5);
    cvec_vec4_push(pl, (vec4){0, 0, 1, -1});
    cvec_vec4_push(pl, (vec4){1 / sqrtf(2), 0, 1 / sqrtf(2), 0});
    cvec_vec4_push(pl, (vec4){-1 / sqrtf(2), 0, 1 / sqrtf(2), 0});
    cvec_vec4_push(pl, (vec4){0, 1 / sqrtf(2), 1 / sqrtf(2), 0});
    cvec_vec4_push(pl, (vec4){0, -1 / sqrtf(2), 1 / sqrtf(2), 0});
    s->pl = pl;
    render(s);
    display_show();
    display_close();
    free_model(m);
    free(tx);
    printf("Render model successfully\n");
    return 0;
  }
  /* Construct cube model */
  model_t *m = malloc(sizeof(model_t));
  m->t = cvec_vec4_alloc(12);
  m->v = cvec_vec3_alloc(8);
  for (int i = 0; i < sizeof(cube_triangles) / sizeof(vec4); i++) {
    cvec_vec4_push(m->t, cube_triangles[i]);
  }
  for (int i = 0; i < sizeof(cube_vertices) / sizeof(vec3); i++) {
    cvec_vec3_push(m->v, cube_vertices[i]);
  }

  model_t *um = malloc(sizeof(model_t));
  um->v = m->v;
  um->t = cvec_vec4_alloc(12);
  for (int i = 0; i < sizeof(unicube_triangles) / sizeof(vec4); i++) {
    cvec_vec4_push(um->t, unicube_triangles[i]);
  }

  model_t *sm = generate_sphere(30, RED);
  model_t *sm1 = generate_sphere(30, CYAN);

  /* Temp light */
  light_t l[] = {
      {0, {0, 0, 0, 0.1}},
      {1, {1, 0, -1, 0.3}},
      {2, {-2, 0, -2, 0.8}},
  };

  /* Construct instances */
  instance_t cubes[] = {
      {m, {0.5, {-30, -30, -30}, {1.5, -1.5, 5}}},
      {um, {0.5, {0, -30, -30}, {1.5, 1.5, 5}}},
      {sm, {1.5, {0, 0, 0}, {-1.5, 0, 7}}},
      {sm1, {5.0, {-45, 0, 0}, {0, 0, 12}}},
  };
  scene_t *s = construct_scene(cubes, sizeof(cubes) / sizeof(instance_t),
                               (transform_t){0, {0, 0, 0}, {0, 0, 0}}, l,
                               sizeof(l) / sizeof(l[0]));
  /* temp plane list */
  cvec_vec4 *pl = cvec_vec4_alloc(5);
  cvec_vec4_push(pl, (vec4){0, 0, 1, -1});
  cvec_vec4_push(pl, (vec4){1 / sqrtf(2), 0, 1 / sqrtf(2), 0});
  cvec_vec4_push(pl, (vec4){-1 / sqrtf(2), 0, 1 / sqrtf(2), 0});
  cvec_vec4_push(pl, (vec4){0, 1 / sqrtf(2), 1 / sqrtf(2), 0});
  cvec_vec4_push(pl, (vec4){0, -1 / sqrtf(2), 1 / sqrtf(2), 0});
  s->pl = pl;
  render(s);
  cvec_vec4_free(m->t);
  cvec_vec3_free(m->v);
  free(m);
  cvec_vec4_free(um->t);
  free(um);
  cvec_vec4_free(sm->t);
  cvec_vec3_free(sm->v);
  free(sm);
  cvec_vec4_free(sm1->t);
  cvec_vec3_free(sm1->v);
  free(sm1);

  // draw_triangle((vec2){0, 0}, (vec2){60, 180}, (vec2){90, 100}, 0);
  // draw_filled_triangle((vec2){0, 0}, (vec2){60, 180}, (vec2){90, 100}, 0);
  display_show();
  display_close();
  printf("Ended successfully\n");
  return 0;
}