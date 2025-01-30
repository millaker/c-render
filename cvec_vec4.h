#ifndef CVEC_VEC4_H
#define CVEC_VEC4_H

#include "types.h"
#include <stdlib.h>

typedef struct {
  vec4 *arr;
  size_t size;     // Current size
  size_t capacity; // Allocated size
} cvec_vec4;

cvec_vec4 *cvec_vec4_alloc(size_t cap);
int cvec_vec4_push(cvec_vec4 *l, vec4 val);
void cvec_vec4_pop(cvec_vec4 *l);
void cvec_vec4_free(cvec_vec4 *l);
cvec_vec4 *cvec_vec4_copy_alloc(cvec_vec4 *l);

#endif
