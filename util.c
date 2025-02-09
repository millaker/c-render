#include "util.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

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
  s->tt = cvec_vec4_alloc(1);
  s->tx = malloc(sizeof(uint32_t *) * isize);
  s->tx_dim = malloc(sizeof(vec2) * isize);
  s->txl = cvec_vec2_alloc(1);

  size_t offset = 0, voffset = 0;
  for (int it = 0; it < isize; it++) {
    model_t *m = inst[it].m;
    transform_t tr = inst[it].tr;
    /* Append vertices with transformations */
    for (size_t i = 0; i < m->v->size; i++) {
      cvec_vec3_push(s->vl, m->v->arr[i]);

      cvec_float_push(s->tr_s, tr.s);
      cvec_vec3_push(s->tr_r, tr.r);
      cvec_vec3_push(s->tr_tr, tr.tr);
    }
    /* Vertex texture uv value */
    for (size_t i = 0; i < m->vt->size; i++) {
      cvec_vec2_push(s->txl, m->vt->arr[i]);
    }
    /* Append triangles with index offset */
    for (size_t i = 0; i < m->t->size; i++) {
      vec4 temp = m->t->arr[i];
      temp.x += offset;
      temp.y += offset;
      temp.z += offset;
      cvec_vec4_push(s->tl, temp);

      /* texture uv index if valid */
      vec4 ttemp = m->tt->arr[i];
      ttemp.x += voffset;
      ttemp.y += voffset;
      ttemp.z += voffset;
      ttemp.w = ttemp.w == 1.0 ? it : -1;
      cvec_vec4_push(s->tt, ttemp);
    }

    voffset += m->vt->size;
    offset += m->v->size;
    /* Assign texture map */
    s->tx[it] = inst[it].tx;
    s->tx_dim[it] = inst[it].tx_dim;
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
  cvec_vec2 *vt = cvec_vec2_alloc(1);
  cvec_vec4 *tt = cvec_vec4_alloc(1);

  char buf[1024] = {0};
  float x, y, z;
  int i[3] = {0};
  int j[3] = {0};
  while (fgets(buf, sizeof(buf), f)) {
    if (buf[0] == 'v') {
      if (buf[1] == 't') {
        /* Add vertex texture */
        sscanf(buf, "vt %f %f\n", &x, &y);
        cvec_vec2_push(vt, (vec2){x, 1.0 - y});
      } else {
        /* Add vertex */
        sscanf(buf, "v %f %f %f\n", &x, &y, &z);
        cvec_vec3_push(vl, (vec3){x, y, z});
      }
    } else if (buf[0] == 'f') {
      /* Add face */
      if (strchr(buf, '/')) {
        /* Have texture */
        sscanf(buf, "f %d/%d %d/%d %d/%d\n", &i[0], &j[0], &i[1], &j[1], &i[2],
               &j[2]);
        cvec_vec4_push(tl, (vec4){i[0] - 1, i[1] - 1, i[2] - 1, 0xffff00});
        cvec_vec4_push(tt, (vec4){j[0] - 1, j[1] - 1, j[2] - 1, 1.0});
      } else {
        /* No texture*/
        sscanf(buf, "f %d %d %d\n", &i[0], &i[1], &i[2]);
        cvec_vec4_push(tl, (vec4){i[0] - 1, i[1] - 1, i[2] - 1, 0xffff00});
        cvec_vec4_push(tt, (vec4){0, 0, 0, 0});
      }
    }
  }
  printf("Read %ld vertices %ld vertex textures and %ld faces from %s\n",
         vl->size, vt->size, tl->size, file);
  m->t = tl;
  m->v = vl;
  m->vt = vt;
  m->tt = tt;
  assert(m->t->size == m->tt->size);
  fclose(f);
  return m;
}

void free_model(model_t *m) {
  cvec_vec3_free(m->v);
  cvec_vec4_free(m->t);
  cvec_vec2_free(m->vt);
  cvec_vec4_free(m->tt);
  free(m);
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int texture_loaded = 0;
uint32_t *load_texture(char *file, vec2 *dim) {
  int x = 0, y = 0, n = 0;
  unsigned char *data = stbi_load(file, &x, &y, &n, 3);
  if (!data)
    return NULL;
  /* Combine to uint32_t for every pixel */
  uint32_t *res = malloc(sizeof(uint32_t) * x * y);
  if (!res) {
    stbi_image_free(data);
    return NULL;
  }
  for (int i = 0; i < x * y; i++) {
    res[i] = data[i * 3] << 16 | data[i * 3 + 1] << 8 | data[i * 3 + 2];
  }
  if (dim) {
    dim->x = x;
    dim->y = y;
  }
  stbi_image_free(data);
  texture_loaded = 1;
  return res;
}

cvec_vec4 *generate_fov90_planes() {
  cvec_vec4 *pl = cvec_vec4_alloc(5);
  cvec_vec4_push(pl, (vec4){0, 0, 1, -1});
  cvec_vec4_push(pl, (vec4){1 / sqrtf(2), 0, 1 / sqrtf(2), 0});
  cvec_vec4_push(pl, (vec4){-1 / sqrtf(2), 0, 1 / sqrtf(2), 0});
  cvec_vec4_push(pl, (vec4){0, 1 / sqrtf(2), 1 / sqrtf(2), 0});
  cvec_vec4_push(pl, (vec4){0, -1 / sqrtf(2), 1 / sqrtf(2), 0});
  return pl;
}
