#ifndef UTIL_H
#define UTIL_H

#include "render.h"
#include "types.h"

typedef struct {
  cvec_vec3 *v;
  cvec_vec4 *t;
} model_t;

typedef struct {
  float s;
  vec3 r;
  vec3 tr;
} transform_t;

typedef struct {
  model_t *m;
  transform_t tr;
} instance_t;

/*
type 0: ambient
type 1: directional
type 2: point

p stores L for type 1 and coordinate for type2
*/
typedef struct {
  int type;
  vec4 p;
} light_t;

scene_t *construct_scene(instance_t *inst, int isize, transform_t cam,
                         light_t *l, int lsize);
model_t *generate_sphere(int divs, int color);
model_t *load_model(char *file);
void free_model(model_t *m);

void swap_vec2(vec2 *, vec2 *);
void swap_vec3(vec3 *, vec3 *);
void swap_vec4(vec4 *, vec4 *);
void swap_int(int *, int *);

vec3 compute_vector(vec3 A, vec3 B);
vec3 vector_cross(vec3 A, vec3 B);
float vector_dot(vec3 A, vec3 B);
float vector_len(vec3 A);

#endif