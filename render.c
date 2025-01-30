#include "render.h"
#include "cvec_float.h"
#include "cvec_vec2.h"
#include "cvec_vec3.h"
#include "cvec_vec4.h"
#include "display.h"
#include "types.h"
#include <math.h>
#include <stdint.h>

static void swap_point(vec2 *p0, vec2 *p1) {
  vec2 temp;
  temp.x = p0->x;
  temp.y = p0->y;
  p0->x = p1->x;
  p0->y = p1->y;
  p1->x = temp.x;
  p1->y = temp.y;
}

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
  return res;
}

void draw_line(vec2 p0, vec2 p1, uint32_t color) {
  float dx = p1.x - p0.x;
  float dy = p1.y - p0.y;
  cvec_float *temp;
  if (fabs(dx) > fabs(dy)) {
    if (p0.x > p1.x)
      swap_point(&p0, &p1);
    temp = interpolate(p0.x, p0.y, p1.x, p1.y);
    for (int x = p0.x; x < p1.x; x++) {
      display_put_pixel(x, temp->arr[(int)(x - p0.x)], color);
    }
  } else {
    if (p0.y > p1.y)
      swap_point(&p0, &p1);
    temp = interpolate(p0.y, p0.x, p1.y, p1.x);
    for (int y = p0.y; y < p1.y; y++) {
      display_put_pixel(temp->arr[(int)(y - p0.y)], y, color);
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
    swap_point(&p1, &p0);
  if (p2.y < p0.y)
    swap_point(&p2, &p0);
  if (p2.y < p1.y)
    swap_point(&p1, &p2);

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
  for (int y = p0.y; y <= p2.y; y++) {
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
  cvec_vec4_free(s->tl);
  cvec_float_free(s->tr_s);
  cvec_vec3_free(s->tr_r);
  cvec_vec3_free(s->tr_tr);
  free(s);
}

static void free_transformed(transformed_t *t) {
  cvec_vec3_free(t->vl);
  cvec_vec4_free(t->tl);
  free(t);
}

static void free_projected(projected_t *p) {
  cvec_vec2_free(p->vl);
  cvec_vec4_free(p->tl);
  free(p);
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
  cvec_vec4 *ntl = cvec_vec4_copy_alloc(s->tl);
  /* Apply transform to every vertex */
  for (size_t i = 0; i < s->vl->size; i++) {
    vec3 tmp = apply_transform(s->vl->arr[i], s->tr_s->arr[i], s->tr_r->arr[i],
                               s->tr_tr->arr[i]);
    tmp = apply_cam_transform(tmp, s->cam_r, s->cam_tr);
    cvec_vec3_push(nvl, tmp);
  }
  free_scene(s);
  transformed_t *temp = malloc(sizeof(transformed_t));
  if (!temp) {
    cvec_vec3_free(nvl);
    cvec_vec4_free(ntl);
    return NULL;
  }
  temp->tl = ntl;
  temp->vl = nvl;
  return temp;
}

static vec2 viewport_to_canvas(vec2 v) {
  return (vec2){v.x * CANVAS_WIDTH / VW, v.y * CANVAS_HEIGHT / VH};
}

static vec2 project_vertex(vec3 v) {
  return viewport_to_canvas((vec2){v.x * VD / v.z, v.y * VD / v.z});
}

static projected_t *project(transformed_t *t) {
  cvec_vec2 *new_vl = cvec_vec2_alloc(t->vl->size * 3);
  cvec_vec4 *new_tl = cvec_vec4_copy_alloc(t->tl);
  /* For every triangle vertex, project its vertices to the canvas */
  for (size_t i = 0; i < t->vl->size; i++) {
    cvec_vec2_push(new_vl, project_vertex(t->vl->arr[i]));
  }
  free_transformed(t);
  projected_t *temp = malloc(sizeof(projected_t));
  if (!temp) {
    cvec_vec2_free(new_vl);
    cvec_vec4_free(new_tl);
    return NULL;
  }
  temp->vl = new_vl;
  temp->tl = new_tl;
  return temp;
}

static void draw_canvas(projected_t *p) {
  /* For every triangle, draw the triangle */
  vec2 *vl = p->vl->arr;
  vec4 *tl = p->tl->arr;
  for (size_t i = 0; i < p->tl->size; i++) {
    draw_triangle(vl[(int)tl[i].x], vl[(int)tl[i].y], vl[(int)tl[i].z],
                  tl[i].w);
  }
  free_projected(p);
}

void render(scene_t *s) {
  transformed_t *t = transform(s);
  projected_t *p = project(t);
  draw_canvas(p);
}
