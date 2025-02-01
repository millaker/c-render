#include "util.h"

scene_t *construct_scene(instance_t *inst, int size, transform_t cam) {
  scene_t *s = malloc(sizeof(scene_t));
  s->vl = cvec_vec3_alloc(1);
  s->tl = cvec_vec4_alloc(1);
  s->tr_s = cvec_float_alloc(1);
  s->tr_r = cvec_vec3_alloc(1);
  s->tr_tr = cvec_vec3_alloc(1);

  size_t offset = 0;
  for (int i = 0; i < size; i++) {
    model_t *m = inst[i].m;
    transform_t tr = inst[i].tr;
    /* Append vertices with transformations */
    for (size_t i = 0; i < m->v->size; i++) {
      cvec_vec3_push(s->vl, m->v->arr[i]);

      cvec_float_push(s->tr_s, tr.s);
      cvec_vec3_push(s->tr_r, tr.r);
      cvec_vec3_push(s->tr_tr, tr.tr);
    }
    /* Append triangles with index offset */
    for (size_t i = 0; i < m->t->size; i++) {
      vec4 temp = m->t->arr[i];
      temp.x += offset;
      temp.y += offset;
      temp.z += offset;
      cvec_vec4_push(s->tl, temp);
    }
    offset += m->v->size;
  }
  s->cam_r = cam.r;
  s->cam_tr = cam.tr;
  return s;
}

void swap_vec2(vec2 *p0, vec2 *p1) {
  vec2 temp;
  temp = *p0;
  *p0 = *p1;
  *p1 = temp;
}

void swap_vec3(vec3 *p0, vec3 *p1) {
  vec3 temp;
  temp = *p0;
  *p0 = *p1;
  *p1 = temp;
}

void swap_vec4(vec4 *p0, vec4 *p1) {
  vec4 temp;
  temp = *p0;
  *p0 = *p1;
  *p1 = temp;
}

void swap_int(int *p0, int *p1) {
  int temp = *p0;
  *p0 = *p1;
  *p1 = temp;
}
