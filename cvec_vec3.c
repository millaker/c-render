#include "cvec_vec3.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

cvec_vec3 *cvec_vec3_alloc(size_t cap) {
  assert(cap > 0);
  cvec_vec3 *temp = malloc(sizeof(cvec_vec3));
  if (!temp) {
    return NULL;
  }
  temp->size = 0;
  temp->capacity = cap;
  temp->arr = malloc(sizeof(vec3) * cap);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  return temp;
}

int cvec_vec3_push(cvec_vec3 *l, vec3 val) {
  if (l->capacity > l->size) {
    l->arr[l->size++] = val;
    return 0;
  }
  size_t new_cap = l->capacity * 2;
  vec3 *temp = realloc(l->arr, new_cap * sizeof(vec3));
  if (!temp)
    return 1;
  l->arr = temp;
  l->arr[l->size++] = val;
  l->capacity = new_cap;
  return 0;
}

void cvec_vec3_pop(cvec_vec3 *l) { l->size--; }

void cvec_vec3_free(cvec_vec3 *l) {
  free(l->arr);
  free(l);
}

cvec_vec3 *cvec_vec3_copy_alloc(cvec_vec3 *l) {
  cvec_vec3 *temp = malloc(sizeof(cvec_vec3));
  if (!temp)
    return NULL;
  temp->size = l->size;
  temp->capacity = l->capacity;
  temp->arr = malloc(sizeof(vec3) * temp->capacity);
  if (!temp->arr) {
    free(temp);
    return NULL;
  }
  memcpy(temp->arr, l->arr, sizeof(vec3) * l->size);
  return temp;
}
