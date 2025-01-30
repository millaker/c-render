#include "cvec_vec2.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

cvec_vec2 *cvec_vec2_alloc(size_t cap) {
  assert(cap > 0);
  cvec_vec2 *temp = malloc(sizeof(cvec_vec2));
  if (!temp) {
    return NULL;
  }
  temp->size = 0;
  temp->capacity = cap;
  temp->arr = malloc(sizeof(vec2) * cap);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  return temp;
}

int cvec_vec2_push(cvec_vec2 *l, vec2 val) {
  if (l->capacity > l->size) {
    l->arr[l->size++] = val;
    return 0;
  }
  size_t new_cap = l->capacity * 2;
  vec2 *temp = realloc(l->arr, new_cap * sizeof(vec2));
  if (!temp)
    return 1;
  l->arr = temp;
  l->arr[l->size++] = val;
  l->capacity = new_cap;
  return 0;
}

void cvec_vec2_pop(cvec_vec2 *l) { l->size--; }

void cvec_vec2_free(cvec_vec2 *l) {
  free(l->arr);
  free(l);
}

cvec_vec2 *cvec_vec2_copy_alloc(cvec_vec2 *l) {
  cvec_vec2 *temp = malloc(sizeof(cvec_vec2));
  if (!temp)
    return NULL;
  temp->size = l->size;
  temp->capacity = l->capacity;
  temp->arr = malloc(sizeof(vec2) * temp->capacity);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  memcpy(temp->arr, l->arr, sizeof(vec2) * l->size);
  return temp;
}
