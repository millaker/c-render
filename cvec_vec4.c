#include "cvec_vec4.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

cvec_vec4 *cvec_vec4_alloc(size_t cap) {
  assert(cap > 0);
  cvec_vec4 *temp = malloc(sizeof(cvec_vec4));
  if (!temp) {
    return NULL;
  }
  temp->size = 0;
  temp->capacity = cap;
  temp->arr = malloc(sizeof(vec4) * cap);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  return temp;
}

int cvec_vec4_push(cvec_vec4 *l, vec4 val) {
  if (l->capacity > l->size) {
    l->arr[l->size++] = val;
    return 0;
  }
  size_t new_cap = l->capacity * 2;
  vec4 *temp = realloc(l->arr, new_cap * sizeof(vec4));
  if (!temp)
    return 1;
  l->arr = temp;
  l->arr[l->size++] = val;
  l->capacity = new_cap;
  return 0;
}

void cvec_vec4_pop(cvec_vec4 *l) { l->size--; }

void cvec_vec4_free(cvec_vec4 *l) {
  free(l->arr);
  free(l);
}

cvec_vec4 *cvec_vec4_copy_alloc(cvec_vec4 *l) {
  cvec_vec4 *temp = malloc(sizeof(cvec_vec4));
  if (!temp)
    return NULL;
  temp->size = l->size;
  temp->capacity = l->capacity;
  temp->arr = malloc(sizeof(vec4) * temp->capacity);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  memcpy(temp->arr, l->arr, sizeof(vec4) * l->size);
  return temp;
}
