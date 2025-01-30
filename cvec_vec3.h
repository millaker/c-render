#ifndef CVEC_VEC3_H
#define CVEC_VEC3_H

#include "types.h"
#include <stdlib.h>

typedef struct {
  vec3 *arr;
  size_t size;     // Current size
  size_t capacity; // Allocated size
} cvec_vec3;

cvec_vec3 *cvec_vec3_alloc(size_t cap);
int cvec_vec3_push(cvec_vec3 *l, vec3 val);
void cvec_vec3_pop(cvec_vec3 *l);
void cvec_vec3_free(cvec_vec3 *l);
cvec_vec3 *cvec_vec3_copy_alloc(cvec_vec3 *l);

#endif
