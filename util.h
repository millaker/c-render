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

scene_t *construct_scene(instance_t *inst, int size, transform_t cam);

void swap_vec2(vec2 *, vec2 *);
void swap_vec3(vec3 *, vec3 *);
void swap_vec4(vec4 *, vec4 *);
void swap_int(int *, int *);

vec3 compute_vector(vec3 A, vec3 B);
vec3 vector_cross(vec3 A, vec3 B);
float vector_dot(vec3 A, vec3 B);

#endif