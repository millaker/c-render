#ifndef CVEC_VEC2_H
#define CVEC_VEC2_H

#include "types.h"
#include <stdlib.h>

typedef struct {
  vec2 *arr;
  size_t size;     // Current size
  size_t capacity; // Allocated size
} cvec_vec2;

cvec_vec2 *cvec_vec2_alloc(size_t cap);
int cvec_vec2_push(cvec_vec2 *l, vec2 val);
void cvec_vec2_pop(cvec_vec2 *l);
void cvec_vec2_free(cvec_vec2 *l);
cvec_vec2 *cvec_vec2_copy_alloc(cvec_vec2 *l);

#endif