#ifndef CVEC_FLOAT_H
#define CVEC_FLOAT_H

#include <stdlib.h>

typedef struct {
  float *arr;
  size_t size;     // Current size
  size_t capacity; // Allocated size
} cvec_float;

cvec_float *cvec_float_alloc(size_t cap);
int cvec_float_push(cvec_float *l, float val);
void cvec_float_pop(cvec_float *l);
void cvec_float_free(cvec_float *l);
cvec_float *cvec_float_copy_alloc(cvec_float *l);

#endif