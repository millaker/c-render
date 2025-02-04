#include "render.h"
#include "cvec_float.h"
#include "cvec_vec2.h"
#include "cvec_vec3.h"
#include "cvec_vec4.h"
#include "display.h"
#include "types.h"
#include "util.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>

cvec_float *interpolate(int i0, float d0, int i1, float d1) {
  cvec_float *res;
  if (i0 == i1) {
    res = cvec_float_alloc(1);
    res->size = 1;
    res->arr[0] = d0;
    return res;
  }
  int size = i1 - i0 + 1;
  res = cvec_float_alloc(size);
  float a = (d1 - d0) / (i1 - i0);
  float d = d0;
  int idx = 0;
  res->size = size;
  for (int i = i0; i <= i1; i++) {
    res->arr[idx++] = d;
    d += a;
  }
  assert(idx == size);
  return res;
}

void draw_line(vec2 p0, vec2 p1, uint32_t color) {
  float dx = p1.x - p0.x;
  float dy = p1.y - p0.y;
  cvec_float *temp;
  if (fabs(dx) > fabs(dy)) {
    if (p0.x > p1.x)
      swap_vec2(&p0, &p1);
    temp = interpolate(p0.x, p0.y, p1.x, p1.y);
    for (int x = p0.x; x < (int)p1.x; x++) {
      int px = CANVAS_WIDTH / 2 + x;
      int py = CANVAS_HEIGHT / 2 - (int)temp->arr[(int)(x - p0.x)];
      display_put_pixel(px, py, color);
    }
  } else {
    if (p0.y > p1.y)
      swap_vec2(&p0, &p1);
    temp = interpolate(p0.y, p0.x, p1.y, p1.x);
    for (int y = p0.y; y < (int)p1.y; y++) {
      int px = CANVAS_WIDTH / 2 + (int)temp->arr[(int)(y - p0.y)];
      int py = CANVAS_HEIGHT / 2 - y;
      display_put_pixel(px, py, color);
    }
  }
  cvec_float_free(temp);
}

void draw_triangle(vec2 p0, vec2 p1, vec2 p2, uint32_t color) {
  draw_line(p0, p1, color);
  draw_line(p1, p2, color);
  draw_line(p2, p0, color);
}

void draw_filled_triangle(vec2 p0, vec2 p1, vec2 p2, uint32_t color) {
  /* Sort points with y, p0 has smallest y */
  if (p1.y < p0.y)
    swap_vec2(&p1, &p0);
  if (p2.y < p0.y)
    swap_vec2(&p2, &p0);
  if (p2.y < p1.y)
    swap_vec2(&p1, &p2);

  /* Compute coordinates of triangle edges */
  cvec_float *x01 = interpolate(p0.y, p0.x, p1.y, p1.x);
  cvec_float *x02 = interpolate(p0.y, p0.x, p2.y, p2.x);
  cvec_float *x12 = interpolate(p1.y, p1.x, p2.y, p2.x);

  /* Combine the two short sides */
  cvec_float *x012 = cvec_float_alloc(x01->size + x12->size - 1);
  x012->size = x012->capacity;
  for (int i = 0; i < x01->size - 1; i++) {
    x012->arr[i] = x01->arr[i];
  }
  for (int i = x01->size - 1; i < x012->size; i++) {
    x012->arr[i] = x12->arr[i - (x01->size - 1)];
  }

  /* Determine left right x */
  int m = x02->size / 2;
  cvec_float *left, *right;
  if (x02->arr[m] < x012->arr[m]) {
    left = x02;
    right = x012;
  } else {
    left = x012;
    right = x02;
  }

  /* Draw horizontal line */
  for (int y = p0.y; y <= (int)p2.y; y++) {
    draw_line((vec2){left->arr[(int)(y - p0.y)], y},
              (vec2){right->arr[(int)(y - p0.y)], y}, color);
  }
  cvec_float_free(x01);
  cvec_float_free(x02);
  cvec_float_free(x12);
  cvec_float_free(x012);
}

static void free_scene(scene_t *s) {
  cvec_vec3_free(s->vl);
  cvec_vec4_free(s->tt);
  cvec_vec2_free(s->txl);
  cvec_vec4_free(s->tl);
  cvec_float_free(s->tr_s);
  cvec_vec3_free(s->tr_r);
  cvec_vec3_free(s->tr_tr);
  cvec_vec4_free(s->pl);
  cvec_float_free(s->lt);
  cvec_vec4_free(s->ll);
  free(s);
}

static void free_transformed(transformed_t *t) {
  cvec_vec3_free(t->vl);
  cvec_vec4_free(t->tt);
  cvec_vec2_free(t->txl);
  cvec_vec4_free(t->tl);
  cvec_vec4_free(t->pl);
  cvec_float_free(t->lt);
  cvec_vec4_free(t->ll);
  free(t);
}

static void free_back_culled(back_culled_t *b) {
  cvec_vec3_free(b->vl);
  cvec_vec4_free(b->tt);
  cvec_vec2_free(b->txl);
  cvec_vec4_free(b->tl);
  cvec_float_free(b->t_valid);
  cvec_float_free(b->lt);
  cvec_vec4_free(b->ll);
  cvec_vec3_free(b->tn);
  free(b);
}

static void free_clipped(clipped_t *c) {
  cvec_vec3_free(c->vl);
  cvec_vec4_free(c->tt);
  cvec_vec2_free(c->txl);
  cvec_vec4_free(c->tl);
  cvec_float_free(c->t_valid);
  cvec_float_free(c->lt);
  cvec_vec4_free(c->ll);
  free(c);
}

static void free_projected(projected_t *p) {
  cvec_vec2_free(p->vl);
  cvec_vec4_free(p->tt);
  cvec_vec2_free(p->txl);
  cvec_vec4_free(p->tl);
  cvec_float_free(p->tv);
  cvec_float_free(p->zl);
  cvec_float_free(p->lt);
  cvec_vec4_free(p->ll);
  cvec_vec3_free(p->tn);
  free(p);
}

static void free_rastered(rastered_t *r) {
  cvec_vec4_free(r->pl);
  free(r);
}

static vec3 apply_transform(vec3 p, float s, vec3 r, vec3 tr) {
  /* Scale */
  p.x *= s;
  p.y *= s;
  p.z *= s;
  float cos = cosf(r.x * PI / 180.0);
  float sin = sinf(r.x * PI / 180.0);
  float temp_x = p.x, temp_y = p.y, temp_z = p.z;
  /* Rotate x */
  p.y = temp_y * cos - temp_z * sin;
  p.z = temp_y * sin + temp_z * cos;
  /* Rotate y */
  temp_x = p.x;
  temp_z = p.z;
  cos = cosf(r.y * PI / 180.0);
  sin = sinf(r.y * PI / 180.0);
  p.x = temp_x * cos + temp_z * sin;
  p.z = -temp_x * sin + temp_z * cos;
  /* Rotate z */
  temp_x = p.x;
  temp_y = p.y;
  cos = cosf(r.z * PI / 180.0);
  sin = sinf(r.z * PI / 180.0);
  p.x = temp_x * cos - temp_y * sin;
  p.y = temp_x * sin + temp_y * cos;
  /* Translate */
  p.x += tr.x;
  p.y += tr.y;
  p.z += tr.z;
  return p;
}

vec3 apply_cam_transform(vec3 p, vec3 r, vec3 tr) {
  /* Translate */
  p.x -= tr.x;
  p.y -= tr.y;
  p.z -= tr.z;

  r.x *= -1;
  r.y *= -1;
  r.z *= -1;

  float cos = cosf(r.x * PI / 180.0);
  float sin = sinf(r.x * PI / 180.0);
  float temp_x = p.x, temp_y = p.y, temp_z = p.z;
  /* Rotate x */
  p.y = temp_y * cos - temp_z * sin;
  p.z = temp_y * sin + temp_z * cos;
  /* Rotate y */
  temp_x = p.x;
  temp_z = p.z;
  cos = cosf(r.y * PI / 180.0);
  sin = sinf(r.y * PI / 180.0);
  p.x = temp_x * cos + temp_z * sin;
  p.z = -temp_x * sin + temp_z * cos;
  /* Rotate z */
  temp_x = p.x;
  temp_y = p.y;
  cos = cosf(r.z * PI / 180.0);
  sin = sinf(r.z * PI / 180.0);
  p.x = temp_x * cos - temp_y * sin;
  p.y = temp_x * sin + temp_y * cos;
  return p;
}

static transformed_t *transform(scene_t *s) {
  cvec_vec3 *nvl = cvec_vec3_alloc(s->vl->size);
  cvec_vec4 *ntt = cvec_vec4_copy_alloc(s->tt);
  cvec_vec4 *ntl = cvec_vec4_copy_alloc(s->tl);
  cvec_vec2 *ntxl = cvec_vec2_copy_alloc(s->txl);
  cvec_vec4 *npl = cvec_vec4_copy_alloc(s->pl);
  cvec_float *nlt = cvec_float_copy_alloc(s->lt);
  cvec_vec4 *nll = cvec_vec4_copy_alloc(s->ll);
  /* Apply transform to every vertex */
  for (size_t i = 0; i < s->vl->size; i++) {
    vec3 tmp = apply_transform(s->vl->arr[i], s->tr_s->arr[i], s->tr_r->arr[i],
                               s->tr_tr->arr[i]);
    tmp = apply_cam_transform(tmp, s->cam_r, s->cam_tr);
    cvec_vec3_push(nvl, tmp);
  }
  transformed_t *temp = malloc(sizeof(transformed_t));
  if (!temp) {
    cvec_vec3_free(nvl);
    cvec_vec4_free(ntt);
    cvec_vec2_free(ntxl);
    cvec_vec4_free(ntl);
    cvec_float_free(nlt);
    cvec_vec4_free(nll);
    free_scene(s);
    return NULL;
  }
  temp->tl = ntl;
  temp->tt = ntt;
  temp->txl = ntxl;
  temp->vl = nvl;
  temp->pl = npl;
  temp->lt = nlt;
  temp->ll = nll;
  temp->tx = s->tx;
  temp->tx_dim = s->tx_dim;
  free_scene(s);
  return temp;
}

static vec2 viewport_to_canvas(vec2 v) {
  return (vec2){v.x * CANVAS_WIDTH / VW, v.y * CANVAS_HEIGHT / VH};
}

static vec2 project_vertex(vec3 v) {
  return viewport_to_canvas((vec2){v.x * VD / v.z, v.y * VD / v.z});
}

/* Clipped triangles are in camera space, which the camera is located at 0,0,0
 */
static back_culled_t *back_face_culling(clipped_t *c) {
  cvec_vec3 *nvl = cvec_vec3_copy_alloc(c->vl);
  cvec_vec4 *ntt = cvec_vec4_copy_alloc(c->tt);
  cvec_vec2 *ntxl = cvec_vec2_copy_alloc(c->txl);
  cvec_vec4 *ntl = cvec_vec4_copy_alloc(c->tl);
  cvec_float *ntv = cvec_float_copy_alloc(c->t_valid);
  cvec_float *nlt = cvec_float_copy_alloc(c->lt);
  cvec_vec4 *nll = cvec_vec4_copy_alloc(c->ll);
  cvec_vec3 *ntn = cvec_vec3_alloc(ntl->size);
  ntn->size = ntn->capacity;
  /* Check every valid triangle */
  for (size_t i = 0; i < c->tl->size; i++) {
    if (ntv->arr[i] == 0.0)
      continue;
    vec4 curr_t = ntl->arr[i];
    /* Calc triangle normal */
    vec3 AB = compute_vector(nvl->arr[(int)curr_t.x], nvl->arr[(int)curr_t.y]);
    vec3 BC = compute_vector(nvl->arr[(int)curr_t.y], nvl->arr[(int)curr_t.z]);
    vec3 CAM = nvl->arr[(int)curr_t.x];
    vec3 CROSS = vector_cross(AB, BC);
    /* Save triangle normal for later use */
    float I_N = vector_len(CROSS);
    ntn->arr[i] = (vec3){CROSS.x / I_N, CROSS.y / I_N, CROSS.z / I_N};
    /* Comp */
    float res = vector_dot(CAM, CROSS);
    if (res > 0) {
      ntv->arr[i] = 0.0;
    }
  }
  back_culled_t *b = malloc(sizeof(back_culled_t));
  if (!b) {
    cvec_vec3_free(nvl);
    cvec_vec4_free(ntt);
    cvec_vec2_free(ntxl);
    cvec_vec4_free(ntl);
    cvec_float_free(ntv);
    cvec_float_free(nlt);
    cvec_vec4_free(nll);
    cvec_vec3_free(ntn);
    free_clipped(c);
    return NULL;
  }
  b->tl = ntl;
  b->tt = ntt;
  b->txl = ntxl;
  b->vl = nvl;
  b->t_valid = ntv;
  b->tn = ntn;
  b->lt = nlt;
  b->ll = nll;
  b->tx = c->tx;
  b->tx_dim = c->tx_dim;
  free_clipped(c);
  return b;
}

static projected_t *project(back_culled_t *t) {
  cvec_vec2 *new_vl = cvec_vec2_alloc(t->vl->size);
  cvec_vec4 *ntt = cvec_vec4_copy_alloc(t->tt);
  cvec_vec2 *ntxl = cvec_vec2_copy_alloc(t->txl);
  cvec_vec4 *new_tl = cvec_vec4_copy_alloc(t->tl);
  cvec_float *new_tv = cvec_float_copy_alloc(t->t_valid);
  cvec_float *new_zl = cvec_float_alloc(t->tl->size);
  cvec_float *nlt = cvec_float_copy_alloc(t->lt);
  cvec_vec4 *nll = cvec_vec4_copy_alloc(t->ll);
  cvec_vec3 *ntn = cvec_vec3_copy_alloc(t->tn);
  /* For every triangle vertex, project its vertices to the canvas */
  for (size_t i = 0; i < t->vl->size; i++) {
    cvec_vec2_push(new_vl, project_vertex(t->vl->arr[i]));
    cvec_float_push(new_zl, 1 / t->vl->arr[i].z);
  }
  assert(new_vl->size == new_zl->size);
  projected_t *temp = malloc(sizeof(projected_t));
  if (!temp) {
    cvec_vec2_free(new_vl);
    cvec_vec4_free(ntt);
    cvec_vec2_free(ntxl);
    cvec_vec4_free(new_tl);
    cvec_float_free(new_tv);
    cvec_float_free(new_zl);
    cvec_float_free(nlt);
    cvec_vec4_free(nll);
    cvec_vec3_free(ntn);
    free_back_culled(t);
    return NULL;
  }
  temp->vl = new_vl;
  temp->tt = ntt;
  temp->txl = ntxl;
  temp->tl = new_tl;
  temp->tv = new_tv;
  temp->zl = new_zl;
  temp->lt = nlt;
  temp->ll = nll;
  temp->tn = ntn;
  temp->tx = t->tx;
  temp->tx_dim = t->tx_dim;
  free_back_culled(t);
  return temp;
}

static void draw_canvas(projected_t *p) {
  /* For every triangle, draw the triangle */
  vec2 *vl = p->vl->arr;
  vec4 *tl = p->tl->arr;
  for (size_t i = 0; i < p->tl->size; i++) {
    if (p->tv->arr[i] == 1.0) {
      draw_triangle(vl[(int)tl[i].x], vl[(int)tl[i].y], vl[(int)tl[i].z],
                    tl[i].w);
    }
  }
  free_projected(p);
}

/* Assume normalized plane normal vector */
static float signed_dis2plane(vec4 plane, vec3 p) {
  return plane.x * p.x + plane.y * p.y + plane.z * p.z + plane.w;
}

static vec3 get_intersect_p(vec4 plane, vec3 A, vec3 B) {
  float N_A = plane.x * A.x + plane.y * A.y + plane.z * A.z;
  vec3 B_A = {B.x - A.x, B.y - A.y, B.z - A.z};
  float N_B = plane.x * B_A.x + plane.y * B_A.y + plane.z * B_A.z;
  float t = (-plane.w - N_A) / N_B;
  vec3 res = A;
  res.x += t * B_A.x;
  res.y += t * B_A.y;
  res.z += t * B_A.z;
  return res;
}

static clipped_t *clipping(transformed_t *t) {
  cvec_vec3 *nvl = cvec_vec3_copy_alloc(t->vl);
  cvec_vec4 *ntt = cvec_vec4_copy_alloc(t->tt);
  cvec_vec2 *ntxl = cvec_vec2_copy_alloc(t->txl);
  cvec_vec4 *ntl = cvec_vec4_copy_alloc(t->tl);
  cvec_float *nlt = cvec_float_copy_alloc(t->lt);
  cvec_vec4 *nll = cvec_vec4_copy_alloc(t->ll);
  /* Construct valid list in this stage */
  cvec_float *t_valid = cvec_float_alloc(t->tl->size);
  t_valid->size = t_valid->capacity;
  for (size_t i = 0; i < t_valid->size; i++) {
    t_valid->arr[i] = 1.0;
  }
  for (size_t p = 0; p < t->pl->size; p++) {
    vec4 curr_plane = t->pl->arr[p];
    size_t old_size = ntl->size;
    /* For every triangle, check against clipping planes */
    for (size_t i = 0; i < old_size; i++) {
      if (t_valid->arr[i] == 0.0)
        continue;
      vec4 curr_t = ntl->arr[i];
      vec3 *curr_vl = nvl->arr;
      vec3 p_a = curr_vl[(int)curr_t.x];
      vec3 p_b = curr_vl[(int)curr_t.y];
      vec3 p_c = curr_vl[(int)curr_t.z];
      float a_dis = signed_dis2plane(curr_plane, p_a);
      float b_dis = signed_dis2plane(curr_plane, p_b);
      float c_dis = signed_dis2plane(curr_plane, p_c);
      /* Point A can only be in out_p0, Point C can only be in out_p1. If
       * PointC overrides PointB out_cnt will be three and all vertices will be
       * discarded. Same with in_p*. Distance equal to zero will be classified
       * as outside point to avoid corner cases where only an edge of the
       * triangle is inside the visible space. */
      vec3 *out_p0 = NULL, *out_p1 = NULL;
      vec3 *in_p0 = NULL, *in_p1 = NULL;
      float in_idx_p0 = -1, in_idx_p1 = -1;
      /* Position check */
      int out_cnt = 0;
      if (a_dis <= 0) {
        out_cnt++;
        out_p0 = &p_a;
      } else {
        in_p0 = &p_a;
        in_idx_p0 = curr_t.x;
      }

      if (b_dis <= 0) {
        out_cnt++;
        if (!out_p0)
          out_p0 = &p_b;
        else
          out_p1 = &p_b;
      } else {
        if (!in_p0) {
          in_p0 = &p_b;
          in_idx_p0 = curr_t.y;
        } else {
          in_p1 = &p_b;
          in_idx_p1 = curr_t.y;
        }
      }

      if (c_dis <= 0) {
        out_cnt++;
        if (!out_p0)
          out_p0 = &p_c;
        else
          out_p1 = &p_c;
      } else {
        if (!in_p0) {
          in_p0 = &p_c;
          in_idx_p0 = curr_t.z;
        } else {
          in_p1 = &p_c;
          in_idx_p1 = curr_t.z;
        }
      }

      if (out_cnt == 3) {
        /* All three outside */
        t_valid->arr[i] = 0.0;
      } else if (out_cnt == 0) {
        /* All three inside, left untouched */
      } else if (out_cnt == 2) {
        /* One inside */
        /* Calc two new vertices, append to vl */
        vec3 np0 = get_intersect_p(curr_plane, *in_p0, *out_p0);
        vec3 np1 = get_intersect_p(curr_plane, *in_p0, *out_p1);
        /* mark current t invalid and append a new valid triangle */
        t_valid->arr[i] = 0.0;
        cvec_vec3_push(nvl, np0);
        cvec_vec3_push(nvl, np1);
        int tmp = nvl->size;
        cvec_vec4_push(ntl, (vec4){in_idx_p0, tmp - 2, tmp - 1, curr_t.w});
        cvec_vec4_push(ntt, (vec4){0, 0, 0, -1});
        cvec_float_push(t_valid, 1.0);
      } else {
        /* Two inside */
        /* Calc two new vertices, append to vl */
        vec3 np0 = get_intersect_p(curr_plane, *in_p0, *out_p0);
        vec3 np1 = get_intersect_p(curr_plane, *in_p1, *out_p0);
        /* Mark current t invalid and append two new valid triangles */
        t_valid->arr[i] = 0.0;
        cvec_vec3_push(nvl, np0);
        cvec_vec3_push(nvl, np1);
        int tmp = nvl->size;
        cvec_vec4_push(ntl, (vec4){in_idx_p0, tmp - 2, tmp - 1, curr_t.w});
        cvec_vec4_push(ntl, (vec4){in_idx_p0, tmp - 1, in_idx_p1, curr_t.w});
        cvec_vec4_push(ntt, (vec4){0, 0, 0, -1});
        cvec_vec4_push(ntt, (vec4){0, 0, 0, -1});
        cvec_float_push(t_valid, 1.0);
        cvec_float_push(t_valid, 1.0);
      }
    }
  }
  assert(t_valid->size == ntl->size);
  clipped_t *cl = malloc(sizeof(clipped_t));
  if (!cl) {
    cvec_vec3_free(nvl);
    cvec_vec4_free(ntt);
    cvec_vec4_free(ntl);
    cvec_vec2_free(ntxl);
    cvec_float_free(t_valid);
    cvec_float_free(nlt);
    cvec_vec4_free(nll);
    free_transformed(t);
    return NULL;
  }
  cl->tl = ntl;
  cl->tt = ntt;
  cl->txl = ntxl;
  cl->vl = nvl;
  cl->t_valid = t_valid;
  cl->lt = nlt;
  cl->ll = nll;
  cl->tx = t->tx;
  cl->tx_dim = t->tx_dim;
  free_transformed(t);
  assert(ntl->size == t_valid->size);
  return cl;
}

static float calc_light_intensity(cvec_float *lt, cvec_vec4 *ll, vec3 P,
                                  vec3 N) {
  assert(lt->size == ll->size);
  float res = 0.0;
  for (size_t i = 0; i < lt->size; i++) {
    if (lt->arr[i] == 0.0) {
      res += ll->arr[i].w;
    } else if (lt->arr[i] == 1.0) {
      vec3 L = {ll->arr[i].x, ll->arr[i].y, ll->arr[i].z};
      float NL = vector_dot(N, L);
      float ABS_N = vector_len(N);
      float ABS_L = vector_len(L);
      float temp = ll->arr[i].w * NL / (ABS_N * ABS_L);
      if (temp > 0)
        res += temp;
    } else if (lt->arr[i] == 2.0) {
      vec3 L = {ll->arr[i].x, ll->arr[i].y, ll->arr[i].z};
      L = compute_vector(P, L);
      float NL = vector_dot(N, L);
      float ABS_N = vector_len(N);
      float ABS_L = vector_len(L);
      float temp = ll->arr[i].w * NL / (ABS_N * ABS_L);
      if (temp > 0)
        res += temp;
    }
  }
  return res >= 1.0 ? 1.0 : res;
}

static uint32_t apply_light(float I, uint32_t c) {
  uint32_t b = c & 0xff;
  uint32_t g = (c >> 8) & 0xff;
  uint32_t r = (c >> 16) & 0xff;
  return (int)(b * I) | ((int)(g * I) << 8) | ((int)(r * I) << 16);
}

static void output(int x, int y, float z, uint32_t color) {
  static float zbuf[CANVAS_WIDTH][CANVAS_HEIGHT] = {0};
  int px = CANVAS_WIDTH / 2 + x;
  int py = CANVAS_HEIGHT / 2 - y;
  /* Ignore points outside canvas */
  if (px >= CANVAS_WIDTH || py >= CANVAS_HEIGHT || px < 0 || py < 0) {
    return;
  }
  /* Compare 1/z, ignore smaller */
  if (z < zbuf[px][py]) {
    return;
  }
  zbuf[px][py] = z;
  display_put_pixel(px, py, color);
}

static rastered_t *rasterize(projected_t *p) {
  float zbuf[CANVAS_WIDTH][CANVAS_HEIGHT] = {0};
  /* For every triangle, calculate which pixel to color with z test */
  printf("%ld triangles to raster\n", p->tl->size);
  for (size_t i = 0; i < p->tl->size; i++) {
    if (p->tv->arr[i] == 0.0)
      continue;
    vec4 curr_t = p->tl->arr[i];
    int idx_x = curr_t.x, idx_y = curr_t.y, idx_z = curr_t.z;
    int tx_idx0 = p->tt->arr[i].x;
    int tx_idx1 = p->tt->arr[i].y;
    int tx_idx2 = p->tt->arr[i].z;
    vec2 p0 = p->vl->arr[idx_x];
    vec2 p1 = p->vl->arr[idx_y];
    vec2 p2 = p->vl->arr[idx_z];
    //printf("%d/%d %d/%d %d/%d\n", idx_x, tx_idx0, idx_y, tx_idx1, idx_z, tx_idx2);
    /* Sort points with y, p0 has smallest y */
    if (p1.y < p0.y) {
      swap_vec2(&p1, &p0);
      swap_int(&idx_y, &idx_x);
      swap_int(&tx_idx1, &tx_idx0);
    }
    if (p2.y < p0.y) {
      swap_vec2(&p2, &p0);
      swap_int(&idx_z, &idx_x);
      swap_int(&tx_idx2, &tx_idx0);
    }
    if (p2.y < p1.y) {
      swap_vec2(&p1, &p2);
      swap_int(&idx_z, &idx_y);
      swap_int(&tx_idx2, &tx_idx1);
    }

    extern int texture_loaded;
    int has_tx = p->tt->arr[i].w >= 0 && texture_loaded;
    int tt_idx = p->tt->arr[i].w;

    /* Interpolate x values for each edge */
    cvec_float *x01 = interpolate(p0.y, p0.x, p1.y, p1.x);
    cvec_float *x02 = interpolate(p0.y, p0.x, p2.y, p2.x);
    cvec_float *x12 = interpolate(p1.y, p1.x, p2.y, p2.x);

    cvec_float *x012 = cvec_float_alloc(x01->size + x12->size - 1);
    x012->size = x012->capacity;
    for (int i = 0; i < x01->size - 1; i++) {
      x012->arr[i] = x01->arr[i];
    }
    for (int i = x01->size - 1; i < x012->size; i++) {
      x012->arr[i] = x12->arr[i - (x01->size - 1)];
    }
    cvec_float_free(x01);
    cvec_float_free(x12);

    /* Interpolate z values for each edge */
    float z0 = p->zl->arr[idx_x];
    float z1 = p->zl->arr[idx_y];
    float z2 = p->zl->arr[idx_z];
    cvec_float *z01 = interpolate(p0.y, z0, p1.y, z1);
    cvec_float *z02 = interpolate(p0.y, z0, p2.y, z2);
    cvec_float *z12 = interpolate(p1.y, z1, p2.y, z2);

    cvec_float *z012 = cvec_float_alloc(z01->size + z12->size - 1);
    z012->size = z012->capacity;
    for (size_t i = 0; i < z01->size - 1; i++) {
      z012->arr[i] = z01->arr[i];
    }
    for (size_t i = z01->size - 1; i < z012->size; i++) {
      z012->arr[i] = z12->arr[i - (z01->size - 1)];
    }

    cvec_float_free(z01);
    cvec_float_free(z12);
    assert(z012->size == x012->size);

    /* Interpolate tx u,v values for each edge */
    float u0, u1, u2, v0, v1, v2;
    cvec_float *u01, *u02, *u12, *u012;
    cvec_float *v01, *v02, *v12, *v012;
    if (has_tx) {
      u0 = p->txl->arr[tx_idx0].x;
      u1 = p->txl->arr[tx_idx1].x;
      u2 = p->txl->arr[tx_idx2].x;
      u0 = (u0 < 0 ? (u0 - floor(u0)) : u0) * z0;
      u1 = (u1 < 0 ? (u1 - floor(u1)) : u1) * z1;
      u2 = (u2 < 0 ? (u2 - floor(u2)) : u2) * z2;
      u01 = interpolate(p0.y, u0, p1.y, u1);
      u02 = interpolate(p0.y, u0, p2.y, u2);
      u12 = interpolate(p1.y, u1, p2.y, u2);
      u012 = cvec_float_alloc(u01->size + u12->size - 1);
      u012->size = u012->capacity;
      for (size_t i = 0; i < u01->size - 1; i++) {
        u012->arr[i] = u01->arr[i];
      }
      for (size_t i = u01->size - 1; i < u012->size; i++) {
        u012->arr[i] = u12->arr[i - (u01->size - 1)];
      }
      cvec_float_free(u01);
      cvec_float_free(u12);
      assert(u012->size == x012->size);

      v0 = p->txl->arr[tx_idx0].y;
      v1 = p->txl->arr[tx_idx1].y;
      v2 = p->txl->arr[tx_idx2].y;
      v0 = (v0 < 0 ? (v0 - floor(v0)) : v0) * z0;
      v1 = (v1 < 0 ? (v1 - floor(v1)) : v1) * z1;
      v2 = (v2 < 0 ? (v2 - floor(v2)) : v2) * z2;
      v01 = interpolate(p0.y, v0, p1.y, v1);
      v02 = interpolate(p0.y, v0, p2.y, v2);
      v12 = interpolate(p1.y, v1, p2.y, v2);
      v012 = cvec_float_alloc(v01->size + v12->size - 1);
      v012->size = v012->capacity;
      for (size_t i = 0; i < v01->size - 1; i++) {
        v012->arr[i] = v01->arr[i];
      }
      for (size_t i = v01->size - 1; i < v012->size; i++) {
        v012->arr[i] = v12->arr[i - (v01->size - 1)];
      }
      cvec_float_free(v01);
      cvec_float_free(v12);
      assert(v012->size == x012->size);
    }

    /* Raster pixels between the two sides */
    /* Determine left right */
    int m = x02->size / 2;
    cvec_float *left, *right, *left_z, *right_z, *left_u, *right_u, *left_v,
        *right_v;
    if (x02->arr[m] < x012->arr[m]) {
      left = x02;
      right = x012;
      left_z = z02;
      right_z = z012;
      left_u = u02;
      right_u = u012;
      left_v = v02;
      right_v = v012;
    } else {
      left = x012;
      right = x02;
      left_z = z012;
      right_z = z02;
      left_u = u012;
      right_u = u02;
      left_v = v012;
      right_v = v02;
    }

    /* Light intensity */
    /* reconstruct point from projected */
    vec3 rp0 = {p0.x / (1 * z0), p0.y / (1 * z0), 1 / z0};
    vec3 rp1 = {p1.x / (1 * z1), p1.y / (1 * z1), 1 / z1};
    vec3 rp2 = {p2.x / (2 * z2), p2.y / (2 * z2), 2 / z2};
    vec3 rp = {(rp0.x + rp1.x + rp2.x) / 3, (rp0.y + rp1.y + rp2.y) / 3,
               (rp0.z + rp1.z + rp2.z) / 3};
    float rpl = vector_len(rp);
    rp = (vec3){rp.x / rpl, rp.y / rpl, rp.z / rpl}; /* Normalized */
    float I = calc_light_intensity(p->lt, p->ll, rp, p->tn->arr[i]);
    uint32_t c_t = apply_light(I, curr_t.w);

    /* Process every horizontal line */
    for (int y = p0.y; y <= (int)p2.y; y++) {
      int idx = y - (int)p0.y;
      int left_x = left->arr[idx], right_x = right->arr[idx];
      if (left_x > right_x)
        continue;
      assert(left_x <= right_x);
      cvec_float *ztemp =
          interpolate(left_x, left_z->arr[idx], right_x, right_z->arr[idx]);
      assert(ztemp->size == right_x - left_x + 1);
      cvec_float *utemp = NULL, *vtemp = NULL;
      if (has_tx) {
        utemp =
            interpolate(left_x, left_u->arr[idx], right_x, right_u->arr[idx]);
        vtemp =
            interpolate(left_x, left_v->arr[idx], right_x, right_v->arr[idx]);
      }

      /* Convert to pixel space */
      for (int i = left_x; i < right_x; i++) {
        int idx = i - left_x;
        uint32_t c = c_t;
        if (has_tx) {
          uint32_t tx = utemp->arr[idx] / ztemp->arr[idx] * p->tx_dim[tt_idx].x;
          uint32_t ty = vtemp->arr[idx] / ztemp->arr[idx] * p->tx_dim[tt_idx].y;
          tx = tx >= p->tx_dim[tt_idx].x ? tx - 1: tx;
          ty = ty >= p->tx_dim[tt_idx].x ? ty - 1: ty;
          tx = tx < 0 ? 0 : tx;
          ty = ty < 0 ? 0 : ty;
          int mapid = ty * p->tx_dim[tt_idx].x + tx;
          c = apply_light(I, p->tx[tt_idx][mapid]);
        }
        output(i, y, ztemp->arr[idx], c);
      }
      cvec_float_free(ztemp);
      if (has_tx) {
        cvec_float_free(vtemp);
        cvec_float_free(utemp);
      }
    }
    cvec_float_free(x02);
    cvec_float_free(z02);
    cvec_float_free(z012);
    cvec_float_free(x012);
    if (has_tx) {
      cvec_float_free(u02);
      cvec_float_free(v02);
      cvec_float_free(u012);
      cvec_float_free(v012);
    }
  }
  free(p->tx);
  free(p->tx_dim);
  free_projected(p);
  return NULL;
}

void render(scene_t *s) {
  printf("Transforming...\n");
  transformed_t *t = transform(s);
  printf("Clipping...\n");
  clipped_t *c = clipping(t);
  printf("Back face culling...\n");
  back_culled_t *b = back_face_culling(c);
  printf("Projecting...\n");
  projected_t *p = project(b);
  printf("Rasterizing...\n");
  rastered_t *r = rasterize(p);
  printf("Raster Done\n");
  fflush(stdout);
}
