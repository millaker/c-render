#ifndef TYPES_H
#define TYPES_H

#include <math.h>
#include <stdint.h>

/* Colors */
#define RED 0xff0000
#define GREEN 0x00ff00
#define BLUE 0x0000ff
#define YELLOW 0xffff00
#define PURPLE 0xff00ff
#define CYAN 0x00ffff

#define PI M_PI

typedef struct {
  float x;
  float y;
} vec2;

typedef struct {
  float x;
  float y;
  float z;
} vec3;

typedef struct {
  float x;
  float y;
  float z;
  float w;
} vec4;

#endif