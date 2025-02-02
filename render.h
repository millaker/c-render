#ifndef RENDER_H
#define RENDER_H

#include "cvec_float.h"
#include "cvec_vec2.h"
#include "cvec_vec3.h"
#include "cvec_vec4.h"
#include "types.h"
#include <stdint.h>

#define VW 1.0
#define VH 1.0
#define VD 1.0

/*
  Scene:
    Instance list
    Camera position (cam)
  Instance:
    Vertex list (vl)
    Triangle list (tl)
    Transform
  Transform:
    Scale (s)
    Rotate (r)
    Translate (tr)
  Triangle:
    Vertex index 0,1,2
    Color
  Vertex:
    3D coordinate
  Light:
    Type:
      0.0 Ambient
      1.0 Direct (xyz = vector)
      2.0 Point (xyz = point)

  The internal representation concat all the instances into a list.
  To mimic hardware behavior, the input of every stage will be discarded.
*/

/*
  scene_t is the input of the rendering pipeline
*/
typedef struct {
  /* Vertex list */
  cvec_vec3 *vl;
  /* Triangle list */
  cvec_vec4 *tl;
  /* Vertex transform info */
  cvec_float *tr_s;
  cvec_vec3 *tr_r;
  cvec_vec3 *tr_tr;
  /* Camera */
  vec3 cam_r;
  vec3 cam_tr;
  /* Plane */
  cvec_vec4 *pl;
  /* Light */
  cvec_float *lt;
  cvec_vec4 *ll;
} scene_t;

typedef struct {
  /* Vertex list */
  cvec_vec3 *vl;
  /* Triangle list */
  cvec_vec4 *tl;
  /* Plane list */
  cvec_vec4 *pl;
  /* Light */
  cvec_float *lt;
  cvec_vec4 *ll;
} transformed_t;

typedef struct {
  /* Vertex list */
  cvec_vec3 *vl;
  /* Triangle list */
  cvec_vec4 *tl;
  cvec_float *t_valid;
  /* Light */
  cvec_float *lt;
  cvec_vec4 *ll;
} clipped_t;

typedef struct {
  /* Vertex list */
  cvec_vec3 *vl;
  /* Triangle list */
  cvec_vec4 *tl;
  cvec_float *t_valid;
  cvec_vec3 *tn;
  /* Light */
  cvec_float *lt;
  cvec_vec4 *ll;
} back_culled_t;

typedef struct {
  /* Vertex list */
  cvec_vec2 *vl;
  /* Triangle list */
  cvec_vec4 *tl;
  cvec_vec3 *tn;
  /* T Valid list*/
  cvec_float *tv;
  /* Z list */
  cvec_float *zl;
  /* Light */
  cvec_float *lt;
  cvec_vec4 *ll;
} projected_t;

typedef struct {
  /* Pixel list */
  cvec_vec4 *pl; // {x, y, z, c}
} rastered_t;

void render(scene_t *s);

/* Debug */
void draw_triangle(vec2 p0, vec2 p1, vec2 p2, uint32_t color);
void draw_filled_triangle(vec2 p0, vec2 p1, vec2 p2, uint32_t color);

#endif