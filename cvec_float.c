#include "cvec_float.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

cvec_float *cvec_float_alloc(size_t cap) {
  assert(cap > 0);
  cvec_float *temp = malloc(sizeof(cvec_float));
  if (!temp) {
    return NULL;
  }
  temp->size = 0;
  temp->capacity = cap;
  temp->arr = malloc(sizeof(float) * cap);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  return temp;
}

int cvec_float_push(cvec_float *l, float val) {
  if (l->capacity > l->size) {
    l->arr[l->size++] = val;
    return 0;
  }
  size_t new_cap = l->capacity * 2;
  float *temp = realloc(l->arr, new_cap * sizeof(float));
  if (!temp)
    return 1;
  l->arr = temp;
  l->arr[l->size++] = val;
  l->capacity = new_cap;
  return 0;
}

void cvec_float_pop(cvec_float *l) { l->size--; }

void cvec_float_free(cvec_float *l) {
  free(l->arr);
  free(l);
}

cvec_float *cvec_float_copy_alloc(cvec_float *l) {
  cvec_float *temp = malloc(sizeof(cvec_float));
  if (!temp)
    return NULL;
  temp->size = l->size;
  temp->capacity = l->capacity;
  temp->arr = malloc(sizeof(float) * temp->capacity);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  memcpy(temp->arr, l->arr, sizeof(float) * l->size);
  return temp;
}
