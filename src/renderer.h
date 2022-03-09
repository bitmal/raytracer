#ifndef __RENDERER_H
#define __RENDERER_H

struct raytracer_canvas;

typedef struct raytracer_renderer raytracer_renderer;

extern raytracer_renderer *
renderer_init(raytracer_canvas *canvas);

extern void
renderer_draw(raytracer_renderer *renderer);

#endif
