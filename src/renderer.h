#ifndef __RENDERER_H
#define __RENDERER_H

#include "stdinc.h"
#include "rt_math.h"

struct raytracer_canvas;
struct raytracer_scene;

typedef struct raytracer_renderer raytracer_renderer;

extern raytracer_renderer *
renderer_init(raytracer_canvas *canvas);

extern void
renderer_push_sphere(raytracer_renderer *renderer, i32 sphereId);

extern void
renderer_draw(raytracer_renderer *renderer, raytracer_scene *scene);

#endif
