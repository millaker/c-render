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

#endif