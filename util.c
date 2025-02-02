#include "util.h"
#include <math.h>
#include <stdio.h>

scene_t *construct_scene(instance_t *inst, int isize, transform_t cam,
                         light_t *l, int lsize) {
  scene_t *s = malloc(sizeof(scene_t));
  s->vl = cvec_vec3_alloc(1);
  s->tl = cvec_vec4_alloc(1);
  s->tr_s = cvec_float_alloc(1);
  s->tr_r = cvec_vec3_alloc(1);
  s->tr_tr = cvec_vec3_alloc(1);
  s->lt = cvec_float_alloc(1);
  s->ll = cvec_vec4_alloc(1);

  size_t offset = 0;
  for (int i = 0; i < isize; i++) {
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

  for (int i = 0; i < lsize; i++) {
    switch (l[i].type) {
    case 0:
      cvec_float_push(s->lt, 0.0);
      cvec_vec4_push(s->ll, l[i].p);
      break;
    case 1:
      cvec_float_push(s->lt, 1.0);
      cvec_vec4_push(s->ll, l[i].p);
      break;
    case 2:
      cvec_float_push(s->lt, 2.0);
      cvec_vec4_push(s->ll, l[i].p);
      break;
    }
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

vec3 compute_vector(vec3 A, vec3 B) {
  return (vec3){B.x - A.x, B.y - A.y, B.z - A.z};
}

float vector_dot(vec3 A, vec3 B) { return A.x * B.x + A.y * B.y + A.z * B.z; }

vec3 vector_cross(vec3 A, vec3 B) {
  return (vec3){A.y * B.z - B.y * A.z, A.z * B.x - B.z * A.x,
                A.x * B.y - B.x * A.y};
}

float vector_len(vec3 A) { return sqrtf(A.x * A.x + A.y * A.y + A.z * A.z); }

model_t *generate_sphere(int divs, int color) {
  cvec_vec3 *vl = cvec_vec3_alloc(1);
  cvec_vec4 *tl = cvec_vec4_alloc(1);
  float delta_angle = 2.0 * M_PI / divs;

  /* Generate vertices */
  for (int d = 0; d < divs + 1; d++) {
    float y = (2.0 / divs) * (d - divs / 2.0);
    float r = sqrtf(1 - y * y);
    for (int i = 0; i < divs; i++) {
      cvec_vec3_push(
          vl, (vec3){r * cosf(i * delta_angle), y, r * sinf(i * delta_angle)});
    }
  }

  /* Generate triangles */
  for (int d = 0; d < divs; d++) {
    for (int i = 0; i < divs; i++) {
      int i0 = d * divs + i;
      int i1 = (d + 1) * divs + (i + 1) % divs;
      int i2 = divs * d + (i + 1) % divs;
      cvec_vec4_push(tl, (vec4){i0, i1, i2, color});
      cvec_vec4_push(tl, (vec4){i0, i0 + divs, i1, color});
    }
  }
  model_t *m = malloc(sizeof(model_t));
  m->t = tl;
  m->v = vl;
  return m;
}

model_t *load_model(char *file) {
  FILE *f = fopen(file, "r");
  if (!f) {
    printf("Read file %s error\n", file);
    return NULL;
  }
  model_t *m = malloc(sizeof(model_t));
  if (!m)
    return NULL;
  cvec_vec3 *vl = cvec_vec3_alloc(1);
  cvec_vec4 *tl = cvec_vec4_alloc(1);

  char buf[1024] = {0};
  float x, y, z;
  int i, j, k;
  while (fgets(buf, sizeof(buf), f)) {
    if (buf[0] == 'v') {
      /* Add vertex */
      sscanf(buf, "v %f %f %f\n", &x, &y, &z);
      cvec_vec3_push(vl, (vec3){x, y, z});
    } else if (buf[0] == 'f') {
      /* Add face */
      sscanf(buf, "f %d %d %d\n", &i, &j, &k);
      cvec_vec4_push(tl, (vec4){i - 1, j - 1, k - 1, 0xffff00});
    }
  }
  printf("Read %ld vertices and %ld faces from %s\n", vl->size, tl->size, file);
  m->t = tl;
  m->v = vl;
  fclose(f);
  return m;
}

void free_model(model_t *m) {
  cvec_vec3_free(m->v);
  cvec_vec4_free(m->t);
  free(m);
}
